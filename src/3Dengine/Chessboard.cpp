#include "Chessboard.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <sstream>
#include "utils/Shader.hpp"
#include "utils/FilePath.hpp"
#include "utils/Program.hpp"
#include "utils/common.hpp"

Chessboard::Chessboard(int size)
    : m_size(size), m_squareSize(1.0f), m_vao(0), m_vbo(0), m_ebo(0)
{
    std::cout << "Chessboard constructor called" << std::endl;
}

Chessboard::~Chessboard() {
    cleanup();
}

bool Chessboard::initialize() {
    // Créer les shaders
    if (!createShaders()) {
        std::cerr << "ERROR::CHESSBOARD::SHADER_CREATION_FAILED" << std::endl;
        return false;
    }
    
    // Créer les données du mesh de l'échiquier
    createChessboardMesh();
    
    // Configurer les VAO/VBO/EBO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    
    glBindVertexArray(m_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(float), m_vertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), m_indices.data(), GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    std::cout << "Chessboard initialized successfully" << std::endl;
    return true;
}

void Chessboard::render(const glm::mat4& view, const glm::mat4& projection) {
    if (m_shaderProgram.getGLId() == 0 || m_vao == 0) {
        std::cerr << "ERROR: Cannot render chessboard - shader program or VAO not initialized" << std::endl;
        return;
    }
    
    // Bind shader et VAO
    m_shaderProgram.use();
    
    // Obtenir les emplacements des uniformes à chaque frame pour s'assurer qu'ils sont valides
    GLint viewLoc = glGetUniformLocation(m_shaderProgram.getGLId(), "view");
    GLint projLoc = glGetUniformLocation(m_shaderProgram.getGLId(), "projection");
    GLint modelLoc = glGetUniformLocation(m_shaderProgram.getGLId(), "model");
    
    // Créer un modèle plus simple pour déboguer
    glm::mat4 model = glm::mat4(1.0f);
    
    // Centrer l'échiquier à l'origine
    float halfSize = (m_size * m_squareSize) / 2.0f;
    model = glm::translate(model, glm::vec3(-halfSize, 0.0f, -halfSize));
    
    // Configurer les uniformes
    if (viewLoc != -1) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    if (projLoc != -1) glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    if (modelLoc != -1) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    
    // Activer le depth test et désactiver le face culling pour le débogage
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    // Dessiner l'échiquier
    glBindVertexArray(m_vao);
    
    // Vérifier que nous avons bien des indices à dessiner
    if (m_indices.size() > 0) {
        std::cout << "Drawing chessboard with " << m_indices.size() << " indices" << std::endl;
        glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, 0);
    } else {
        std::cerr << "WARNING: No indices to draw in chessboard" << std::endl;
    }
    
    // Unbind VAO et shader
    glBindVertexArray(0);
    glUseProgram(0);
}

void Chessboard::cleanup() {
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_ebo) {
        glDeleteBuffers(1, &m_ebo);
        m_ebo = 0;
    }
}

bool Chessboard::createShaders() {
    try {
        // Chemins vers les fichiers de shader
        std::string basePath = "../../shaders/";
        std::string vertexShaderPath = basePath + "chessboard.vs.glsl";
        std::string fragmentShaderPath = basePath + "chessboard.fs.glsl";
        
        // Vérifier si les fichiers existent
        std::ifstream testVertex(vertexShaderPath);
        std::ifstream testFragment(fragmentShaderPath);
        
        if (testVertex.good() && testFragment.good()) {
            std::cout << "Shaders d'échiquier trouvés au chemin: " << basePath << std::endl;
            
            // Utiliser les utilitaires existants pour charger les shaders depuis les fichiers
            m_shaderProgram = glBurnout::loadProgram(
                glBurnout::FilePath(vertexShaderPath),
                glBurnout::FilePath(fragmentShaderPath)
            );
        } else {
            // Si les fichiers n'existent pas, utiliser les shaders intégrés
            std::cout << "INFO: Fichiers de shader pour l'échiquier non trouvés, utilisation des shaders intégrés" << std::endl;
        }
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR::SHADER::LOADING_FAILED: " << e.what() << std::endl;
        return false;
    }
}

void Chessboard::createChessboardMesh() {
    // Vider les vecteurs
    m_vertices.clear();
    m_indices.clear();
    
    const float squareHeight = 0.1f;  // Hauteur des cases
    
    // Créer toutes les cases de l'échiquier 
    for (int z = 0; z < m_size; z++) {
        for (int x = 0; x < m_size; x++) {
            // Déterminer la couleur de la case 
            glm::vec3 color = (x + z) % 2 == 0 ? 
                              glm::vec3(0.9f, 0.85f, 0.8f) :   // Case blanche 
                              glm::vec3(0.2f, 0.15f, 0.1f);    // Case noire 
            
            // Position de la case
            glm::vec3 position(x * m_squareSize, 0.0f, z * m_squareSize);
            
            // Ajouter un cube pour cette case
            addCube(position, 
                    glm::vec3(m_squareSize, squareHeight, m_squareSize), 
                    color);
        }
    }
    
    // Ajouter une bordure plus élégante
    addBorder(squareHeight);
    
    std::cout << "Chessboard mesh created with " << m_vertices.size()/6 << " vertices and " 
              << m_indices.size() << " indices" << std::endl;
}

void Chessboard::addCube(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color) {
    unsigned int baseIndex = m_vertices.size() / 6;
    
    const float bottomShade = 0.7f;
    
    // Les 8 sommets du cube
    std::vector<glm::vec3> positions = {
        // Face supérieure
        {position.x,          position.y + size.y, position.z},           // 0
        {position.x + size.x, position.y + size.y, position.z},           // 1
        {position.x + size.x, position.y + size.y, position.z + size.z},  // 2
        {position.x,          position.y + size.y, position.z + size.z},  // 3
        
        // Face inférieure
        {position.x,          position.y,          position.z},           // 4
        {position.x + size.x, position.y,          position.z},           // 5
        {position.x + size.x, position.y,          position.z + size.z},  // 6
        {position.x,          position.y,          position.z + size.z}   // 7
    };
    
    std::vector<glm::vec3> colors = {
        color,                       // 0 (haut)
        color,                       // 1 (haut)
        color,                       // 2 (haut)
        color,                       // 3 (haut)
        color * bottomShade,         // 4 (bas)
        color * bottomShade,         // 5 (bas)
        color * bottomShade,         // 6 (bas)
        color * bottomShade          // 7 (bas)
    };
    
    // Ajouter les sommets
    for (int i = 0; i < 8; i++) {
        m_vertices.push_back(positions[i].x);
        m_vertices.push_back(positions[i].y);
        m_vertices.push_back(positions[i].z);
        m_vertices.push_back(colors[i].r);
        m_vertices.push_back(colors[i].g);
        m_vertices.push_back(colors[i].b);
    }
    
    // Indices pour les faces (2 triangles par face, 6 faces)
    std::vector<std::vector<unsigned int>> faceIndices = {
        // Face supérieure (y+)
        {0, 1, 2, 0, 2, 3},
        // Face inférieure (y-)
        {4, 6, 5, 4, 7, 6},
        // Face avant (z-)
        {0, 4, 5, 0, 5, 1},
        // Face arrière (z+)
        {2, 6, 7, 2, 7, 3},
        // Face gauche (x-)
        {3, 7, 4, 3, 4, 0},
        // Face droite (x+)
        {1, 5, 6, 1, 6, 2}
    };
    
    // Ajouter les indices
    for (const auto& face : faceIndices) {
        for (unsigned int idx : face) {
            m_indices.push_back(baseIndex + idx);
        }
    }
}

void Chessboard::addBorder(float squareHeight) {
    float borderWidth = 0.5f * m_squareSize;
    float borderHeight = squareHeight * 2.5f;
    float boardWidth = m_size * m_squareSize;
    
    // Couleur de la bordure
    glm::vec3 borderColor(0.35f, 0.2f, 0.1f);
    
    // Dimensions et positions des 4 bordures
    struct BorderSection {
        glm::vec3 position;
        glm::vec3 size;
    };
    
    std::vector<BorderSection> borders = {
        // Bordure gauche
        {{-borderWidth, 0.0f, -borderWidth}, 
         {borderWidth, borderHeight, boardWidth + 2 * borderWidth}},
         
        // Bordure droite
        {{boardWidth, 0.0f, -borderWidth}, 
         {borderWidth, borderHeight, boardWidth + 2 * borderWidth}},
         
        // Bordure avant
        {{0.0f, 0.0f, -borderWidth}, 
         {boardWidth, borderHeight, borderWidth}},
         
        // Bordure arrière
        {{0.0f, 0.0f, boardWidth}, 
         {boardWidth, borderHeight, borderWidth}}
    };
    
    // Ajouter chaque section de bordure
    for (const auto& border : borders) {
        addCube(border.position, border.size, borderColor);
    }
}
