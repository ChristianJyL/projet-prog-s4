// SkyBox.hpp
#pragma once

#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class SkyBox {
public:
    SkyBox(const std::vector<std::string>& faces);
    ~SkyBox();
    
    void Draw(const glm::mat4& view, const glm::mat4& projection);
    
private:
    GLuint loadCubemap(const std::vector<std::string>& faces);
    GLuint compileShader();
    
    GLuint VAO, VBO;
    GLuint textureID;
    GLuint shaderProgram;

    std::string loadShaderSource(const std::string& filepath);
};