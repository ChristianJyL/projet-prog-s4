// SkyBox.hpp
#pragma once

#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "utils/Program.hpp" 
#include "utils/FilePath.hpp" 
#include "utils/Image.hpp"    
#include "utils/Shader.hpp"   

class SkyBox {
public:
    SkyBox();
    SkyBox(const std::vector<std::string>& faces);
    ~SkyBox();
    
    bool initialize();
    
    void Draw(const glm::mat4& view, const glm::mat4& projection);
    
    static std::vector<std::string> findSkyboxTextures();
    
private:
    GLuint loadCubemap(const std::vector<std::string>& faces);
    bool compileShader(); // Maintenant retourne un bool au lieu de GLuint
    
    GLuint VAO, VBO;
    GLuint textureID;
    glBurnout::Program m_skyboxShader; // Remplace GLuint shaderProgram
    bool m_isInitialized;

    std::string loadShaderSource(const std::string& filepath);
};