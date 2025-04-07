#include "PieceRenderer.hpp"
#include <iostream>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

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

void PieceRenderer::update(float deltaTime) {
    // Mettre à jour toutes les pièces animées
    for (auto& piece : m_pieces) {
        if (piece.state != AnimationState::Idle) {
            // Avancer le temps d'animation
            piece.animationTime += deltaTime;
            
            // Vérifier si l'animation est terminée
            if (piece.animationTime >= piece.animationDuration) {
                // Animation terminée
                piece.animationTime = piece.animationDuration;
                piece.state = AnimationState::Idle;
                
                // Si la pièce était en train d'être capturée, la supprimer
                if (piece.isBeingCaptured) {
                    // Marquer pour suppression (sera traitée après la boucle)
                    piece.type = PieceType::None;
                }
            }
        }
    }
    
    // Supprimer les pièces capturées (marquées avec type None)
    m_pieces.erase(
        std::remove_if(
            m_pieces.begin(), 
            m_pieces.end(),
            [](const ChessPiece& p) { return p.type == PieceType::None; }
        ),
        m_pieces.end()
    );
    
    // Si toutes les animations sont terminées et qu'on a un état suivant,
    // on peut passer à cet état
    if (!m_nextState.empty() && !isAnimating()) {
        m_pieces = m_nextState;
        m_nextState.clear();
    }
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
        
        // Si la pièce est en train d'être capturée, modifier sa couleur 
        if (piece.isBeingCaptured) {
            float captureProgress = piece.animationTime / piece.animationDuration;
            color = glm::mix(color, glm::vec3(0.8f, 0.0f, 0.0f), captureProgress);
        }
        
        // Passer la couleur au shader uniquement si l'uniform existe
        if (pieceColorLoc != -1) {
            glUniform3fv(pieceColorLoc, 1, glm::value_ptr(color));
        }
        
        // Calculer la matrice de modèle en tenant compte de l'animation
        glm::mat4 pieceModel = calculateAnimatedModelMatrix(piece, squareSize);
        
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

glm::vec3 PieceRenderer::calculateChessPosition(int x, int y, float squareSize) const {
    const float pieceOffset = squareSize * 0.5f;
    const float pieceHeight = 0.1f;
    
    return glm::vec3(
        -4 * squareSize + x * squareSize + pieceOffset,
        pieceHeight,
        -4 * squareSize + y * squareSize + pieceOffset
    );
}

glm::mat4 PieceRenderer::calculateAnimatedModelMatrix(const ChessPiece& piece, float squareSize) const {
    glm::mat4 model = glm::mat4(1.0f);
    
    // Calculer l'offset pour centrer les pièces sur les cases
    const float pieceOffset = squareSize * 0.5f;
    // Taille des pièces
    const float pieceScale = 7.0f;
    // Hauteur des pièces sur l'échiquier
    const float pieceHeight = 0.1f;
    
    if (piece.state == AnimationState::Idle) {
        // Positionnement normal de la pièce
        model = glm::translate(model, 
            glm::vec3(-4 * squareSize + piece.x * squareSize + pieceOffset, 
                    pieceHeight, 
                    -4 * squareSize + piece.y * squareSize + pieceOffset));
    } else {
        // Animation en cours
        float t = piece.animationTime / piece.animationDuration;
        
        // Ajout d'une courbe d'accélération/décélération (ease-in-out)
        float smoothT = t * t * (3.0f - 2.0f * t);
        
        if (piece.state == AnimationState::Moving || piece.state == AnimationState::Capturing) {
            // Interpolation linéaire entre les positions de départ et d'arrivée
            glm::vec3 currentPos;
            
            if (piece.isBeingCaptured) {
                // Pour les pièces capturées, on les fait tomber de l'échiquier
                currentPos = glm::mix(
                    piece.startPosition,
                    glm::vec3(piece.startPosition.x, 0.5f, piece.startPosition.z),
                    smoothT
                );
                
                // Ajouter une rotation lors de la capture
                //TODO :  VOIR POUR AJOUTER UNE LOIS DE PROBA POUR PLUS DE RANDOM...
                float captureRotation = smoothT * 90.0f;
                model = glm::translate(model, currentPos);
                model = glm::rotate(model, glm::radians(captureRotation), glm::vec3(1.0f, 0.0f, 0.0f));
            } else {
                // Pour les mouvements normaux, trajectoire parabolique (saut)
                float jumpFactor = 4.0f * smoothT * (1.0f - smoothT); // Fonction parabolique maximale à t=0.5
                
                currentPos = glm::mix(piece.startPosition, piece.targetPosition, smoothT);
                currentPos.y += piece.jumpHeight * jumpFactor;
                
                model = glm::translate(model, currentPos);
            }
        }
    }
    //TODO : AJOUTER UNE ROTATION ALEATOIRE POUR CHAQUE PIECE ? 
    
    // Mise à l'échelle pour toutes les pièces
    model = glm::scale(model, glm::vec3(pieceScale));
    
    return model;
}


void PieceRenderer::startPieceMovement(ChessPiece& piece, int targetX, int targetY, float duration) {
    piece.state = AnimationState::Moving;
    piece.animationTime = 0.0f;
    piece.animationDuration = duration;
    
    //position actuelle comme position de départ pour l'animation
    const float squareSize = 1.0f;
    piece.startPosition = calculateChessPosition(piece.x, piece.y, squareSize);
    piece.targetPosition = calculateChessPosition(targetX, targetY, squareSize);
    
    // Calculer la hauteur du saut en fonction de la distance
    float distance = std::sqrt(pow(targetX - piece.x, 2) + pow(targetY - piece.y, 2));
    piece.jumpHeight =  0.3f * distance;
    
    // Mettre à jour les coordonnées de la pièce
    piece.x = targetX;
    piece.y = targetY;
}

void PieceRenderer::animateTransition(const std::vector<ChessPiece>& newState, float duration) {
    if (m_pieces.empty()) {
        // Si aucune pièce actuelle, juste adopter le nouvel état
        m_pieces = newState;
        return;
    }
    
    // Vérifier s'il y a des changements entre l'état actuel et le nouvel état
    bool hasChanges = false;
    
    // Créer une copie du nouvel état pour le traitement
    std::vector<ChessPiece> mutableNewState = newState;
    
    // Marquer les pièces qui sont présentes dans l'ancien état et le nouveau (non déplacées)
    std::vector<bool> oldPieceMatched(m_pieces.size(), false);
    std::vector<bool> newPieceMatched(mutableNewState.size(), false);
    
    //identification des pièces qui n'ont pas bougé (même position dans les deux états + couleur et type au cas ou)
    for (size_t i = 0; i < m_pieces.size(); ++i) {
        for (size_t j = 0; j < mutableNewState.size(); ++j) {
            if (!newPieceMatched[j] && 
                m_pieces[i].type == mutableNewState[j].type && 
                m_pieces[i].color == mutableNewState[j].color && 
                m_pieces[i].x == mutableNewState[j].x && 
                m_pieces[i].y == mutableNewState[j].y) {
                
                // Cette pièce n'a pas bougé
                oldPieceMatched[i] = true;
                newPieceMatched[j] = true;
                break;
            }
        }
    }
    
    // Appliquer directement les mouvements pour chaque pièce non matchée
    for (size_t i = 0; i < m_pieces.size(); ++i) {
        if (oldPieceMatched[i]) continue;  // Déjà fait
        
        for (size_t j = 0; j < mutableNewState.size(); ++j) {
            if (newPieceMatched[j]) continue;  // Déjà fait
            
            if (m_pieces[i].type == mutableNewState[j].type && 
                m_pieces[i].color == mutableNewState[j].color) {
                // Cette pièce s'est déplacée
                ChessPiece& piece = m_pieces[i];
                const ChessPiece& target = mutableNewState[j];
                
                startPieceMovement(piece, target.x, target.y, duration);
                hasChanges = true;
                
                oldPieceMatched[i] = true;
                newPieceMatched[j] = true;
                break;  // Une seule destination possible par pièce
            }
        }
    }

    // 3. Les pièces de l'ancien état qui n'ont pas été matchées ont été capturées
    for (size_t i = 0; i < m_pieces.size(); ++i) {
        if (!oldPieceMatched[i]) {
            // Cette pièce a été capturée
            m_pieces[i].state = AnimationState::Capturing;
            m_pieces[i].isBeingCaptured = true;
            m_pieces[i].animationTime = 0.0f;
            m_pieces[i].animationDuration = duration * 0.8f;
            m_pieces[i].startPosition = calculateChessPosition(m_pieces[i].x, m_pieces[i].y, 1.0f);
            hasChanges = true;
        }
    }

    // 4. Les pièces du nouvel état qui n'ont pas été matchées sont nouvelles (ex: promotion de pion)
    for (size_t j = 0; j < mutableNewState.size(); ++j) {
        if (!newPieceMatched[j]) {
            // C'est une nouvelle pièce, l'ajouter directement
            addPiece(mutableNewState[j].type, mutableNewState[j].color, 
                    mutableNewState[j].x, mutableNewState[j].y);
            hasChanges = true;
        }
    }

    // Sauvegarder le nouvel état pour l'appliquer une fois l'animation terminée
    if (hasChanges) {
        m_nextState = mutableNewState;
    } else {
        // S'il n'y a aucun changement, appliquer directement le nouvel état
        m_pieces = mutableNewState;
    }
}

bool PieceRenderer::isAnimating() const {
    for (const auto& piece : m_pieces) {
        if (piece.state != AnimationState::Idle) {
            return true;
        }
    }
    return false;
}
