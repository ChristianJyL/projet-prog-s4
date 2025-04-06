#include "PieceRenderer.hpp"
#include <iostream>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

PieceRenderer::PieceRenderer()
    : m_piecesLoaded(false)
{
    std::cout << "PieceRenderer constructor called" << std::endl;
}

PieceRenderer::~PieceRenderer() {
    cleanup();
}

bool PieceRenderer::initialize() {
    if (!createShader()) {
        std::cerr << "ERROR: Failed to create piece shader" << std::endl;
        return false;
    }
    
    return true;
}

void PieceRenderer::render(const glm::mat4& view, const glm::mat4& projection, float squareSize) {
    if (!m_piecesLoaded || m_pieces.empty()) {
        return;
    }
    m_pieceShader.use();
    
    // Passer les matrices de vue et projection
    GLuint viewPieceLoc = glGetUniformLocation(m_pieceShader.getGLId(), "view");
    GLuint projPieceLoc = glGetUniformLocation(m_pieceShader.getGLId(), "projection");
    GLuint modelPieceLoc = glGetUniformLocation(m_pieceShader.getGLId(), "model");
    GLuint pieceColorLoc = glGetUniformLocation(m_pieceShader.getGLId(), "pieceColor");
    
    
    glUniformMatrix4fv(viewPieceLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projPieceLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    // Calculer l'offset pour centrer les pièces sur les cases
    const float pieceOffset = squareSize * 0.5f;
    // Taille des pièces
    const float pieceScale = 7.0f;
    // Hauteur des pièces sur l'échiquier
    const float pieceHeight = 0.1f;
    
    // Dessiner chaque pièce
    for (const auto& piece : m_pieces) {
        auto it = m_pieceData.find(piece.type);
        if (it == m_pieceData.end()) {
            continue; // Sauter si le modèle n'est pas chargé
        }
        
        // Définir la couleur de la pièce en fonction de PieceColor
        glm::vec3 color;
        if (piece.color == PieceColor::White) {
            color = glm::vec3(0.95f, 0.95f, 0.85f); // Blanc ivoire pour les pièces blanches
        } else {
            color = glm::vec3(0.15f, 0.15f, 0.15f); // Noir profond pour les pièces noires
        }
        
        // Passer la couleur au shader uniquement si l'uniform existe
        if (pieceColorLoc != -1) {
            glUniform3fv(pieceColorLoc, 1, glm::value_ptr(color));
        }
        
        // Calculer la position de la pièce
        glm::mat4 pieceModel = glm::mat4(1.0f);
        
        // D'abord placer au centre de la case
        pieceModel = glm::translate(pieceModel, 
            glm::vec3(-4 * squareSize + piece.x * squareSize + pieceOffset, 
                      pieceHeight, 
                      -4 * squareSize + piece.y * squareSize + pieceOffset));
        
        // Puis faire la rotation avant la mise à l'échelle
        if (piece.color == PieceColor::Black) {
            pieceModel = glm::rotate(pieceModel, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }
        
        // Enfin appliquer la mise à l'échelle
        pieceModel = glm::scale(pieceModel, glm::vec3(pieceScale));
        
        glUniformMatrix4fv(modelPieceLoc, 1, GL_FALSE, glm::value_ptr(pieceModel));
        
        // Référence aux données de rendu de la pièce
        const auto& renderData = it->second;
        
        // Dessiner la pièce avec son VAO spécifique
        glBindVertexArray(renderData.vao);
        for (size_t i = 0; i < renderData.geometry.getMeshCount(); i++) {
            const auto& mesh = renderData.geometry.getMeshBuffer()[i];
            glDrawElements(GL_TRIANGLES, mesh.m_nIndexCount, GL_UNSIGNED_INT, 
                          (void*)(mesh.m_nIndexOffset * sizeof(unsigned int)));
        }
    }
    
    glBindVertexArray(0);
    glUseProgram(0);
}

bool PieceRenderer::loadPieceModel(PieceType type, const std::string& modelPath) {
    std::cout << "Loading chess piece model: " << modelPath << std::endl;
    
    try {
        // Déterminer le chemin de base pour les textures
        std::string modelDir = modelPath.substr(0, modelPath.find_last_of("/\\") + 1);
        
        // Créer une nouvelle géométrie pour ce type de pièce
        glBurnout::Geometry pieceGeometry;
        
        // Charger le modèle OBJ
        if (!pieceGeometry.loadOBJ(modelPath, modelDir)) {
            std::cerr << "ERROR: Failed to load piece model: " << modelPath << std::endl;
            return false;
        }
        
        std::cout << "Successfully loaded model with " 
                  << pieceGeometry.getVertexCount() << " vertices, " 
                  << pieceGeometry.getIndexCount() << " indices, and "
                  << pieceGeometry.getMeshCount() << " meshes." << std::endl;
        
        // Configurer les buffers pour ce type de pièce
        setupBuffers(type, pieceGeometry);
        
        m_piecesLoaded = true;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Exception during model loading: " << e.what() << std::endl;
        return false;
    }
}

void PieceRenderer::addPiece(PieceType type, PieceColor color, int x, int y) {
    // Vérifier que le type de pièce a un modèle chargé
    if (m_pieceData.find(type) == m_pieceData.end()) {
        std::cerr << "WARNING: Cannot add piece of type " << static_cast<int>(type) 
                  << " - model not loaded" << std::endl;
        return;
    }
    
    ChessPiece piece;
    piece.type = type;
    piece.color = color;
    piece.x = x;
    piece.y = y;
    
    m_pieces.push_back(piece);
}

void PieceRenderer::clearPieces() {
    m_pieces.clear();
}

void PieceRenderer::cleanup() {
    // Nettoyer tous les VAOs, VBOs et EBOs pour chaque type de pièce
    for (auto& data : m_pieceData) {
        if (data.second.vao) {
            glDeleteVertexArrays(1, &data.second.vao);
        }
        if (data.second.vbo) {
            glDeleteBuffers(1, &data.second.vbo);
        }
        if (data.second.ebo) {
            glDeleteBuffers(1, &data.second.ebo);
        }
    }
    
    m_pieceData.clear();
    m_pieces.clear();
    m_piecesLoaded = false;
}

bool PieceRenderer::createShader() {
    try {
        // Chemin vers les fichiers de shader
        std::string basePath = "../../shaders/";
        std::string vertexShaderPath = basePath + "piece.vs.glsl";
        std::string fragmentShaderPath = basePath + "piece.fs.glsl";
        
        // Vérifier si les fichiers existent
        std::ifstream testVertex(vertexShaderPath);
        std::ifstream testFragment(fragmentShaderPath);
        
        if (testVertex.good() && testFragment.good()) {
            std::cout << "Shaders des pièces trouvés au chemin: " << basePath << std::endl;
            
            // Utiliser les utilitaires existants pour charger les shaders depuis les fichiers
            glBurnout::FilePath vsPath(vertexShaderPath);
            glBurnout::FilePath fsPath(fragmentShaderPath);
            
            // Charger le programme shader à partir des fichiers
            m_pieceShader = glBurnout::loadProgram(vsPath, fsPath);
            return true;
        } else {
            // Si les fichiers n'existent pas, utiliser les shaders intégrés
            std::cout << "INFO: Fichiers de shader pour les pièces non trouvés, utilisation des shaders intégrés" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "ERROR: Failed to create piece shader: " << e.what() << std::endl;
        return false;
    }
}


void PieceRenderer::setupBuffers(PieceType type, const glBurnout::Geometry& geometry) {
    // Créer une nouvelle entrée pour ce type de pièce
    PieceRenderData& renderData = m_pieceData[type];
    
    // Copier la géométrie
    renderData.geometry = geometry;
    
    // Générer les buffers OpenGL
    glGenVertexArrays(1, &renderData.vao);
    glGenBuffers(1, &renderData.vbo);
    glGenBuffers(1, &renderData.ebo);
    
    glBindVertexArray(renderData.vao);
    
    // Charger les données de vertex
    glBindBuffer(GL_ARRAY_BUFFER, renderData.vbo);
    glBufferData(GL_ARRAY_BUFFER, 
                 geometry.getVertexCount() * sizeof(glBurnout::Geometry::Vertex), 
                 geometry.getVertexBuffer(), 
                 GL_STATIC_DRAW);
    
    // Charger les indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderData.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 
                 geometry.getIndexCount() * sizeof(unsigned int), 
                 geometry.getIndexBuffer(), 
                 GL_STATIC_DRAW);
    
    // Configurer les attributs de vertex
    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glBurnout::Geometry::Vertex), 
                         (void*)offsetof(glBurnout::Geometry::Vertex, m_Position));
    
    // Normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glBurnout::Geometry::Vertex), 
                         (void*)offsetof(glBurnout::Geometry::Vertex, m_Normal));
    
    // TexCoord
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glBurnout::Geometry::Vertex), 
                         (void*)offsetof(glBurnout::Geometry::Vertex, m_TexCoords));
    
    glBindVertexArray(0);
}
