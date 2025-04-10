#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glad/glad.h>
#include "utils/Program.hpp"

class Chessboard {
public:
    Chessboard(int size = 8);
    ~Chessboard();
    
    bool initialize();
    void render(const glm::mat4& view, const glm::mat4& projection);
    void cleanup();
    
    float getSquareSize() const { return m_squareSize; }
    float getSquareHeight() const { return 0.1f; } // Hauteur fixe des cases
    
private:
    bool createShaders();
    void createChessboardMesh();
    void addCube(const glm::vec3& position, const glm::vec3& size, const glm::vec3& color);
    void addBorder(float squareHeight);
    
    int m_size;
    float m_squareSize;
    
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ebo;
    
    glBurnout::Program m_shaderProgram;
    
    std::vector<float> m_vertices;
    std::vector<unsigned int> m_indices;
};
