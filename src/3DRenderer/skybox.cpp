#include "skybox.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstring> 
#include "utils/FilePath.hpp" 
#include "utils/Image.hpp"    
#include "utils/Shader.hpp"   

using namespace glBurnout;

SkyBox::SkyBox(const std::vector<std::string>& faces) 
    : textureID(0), VAO(0), VBO(0), shaderProgram(0)
{
    // Compile shaders
    shaderProgram = compileShader();
    
    // Load cubemap texture
    textureID = loadCubemap(faces);
    
    float skyboxVertices[] = {
        -10.0f,  10.0f, -10.0f,
        -10.0f, -10.0f, -10.0f,
         10.0f, -10.0f, -10.0f,
         10.0f, -10.0f, -10.0f,
         10.0f,  10.0f, -10.0f,
        -10.0f,  10.0f, -10.0f,

        -10.0f, -10.0f,  10.0f,
        -10.0f, -10.0f, -10.0f,
        -10.0f,  10.0f, -10.0f,
        -10.0f,  10.0f, -10.0f,
        -10.0f,  10.0f,  10.0f,
        -10.0f, -10.0f,  10.0f,
        -10.0f,  10.0f,  10.0f,
         10.0f,  10.0f,  10.0f,
         10.0f,  10.0f,  10.0f,
         10.0f, -10.0f,  10.0f,
        -10.0f, -10.0f,  10.0f,

        -10.0f,  10.0f, -10.0f,
         10.0f,  10.0f, -10.0f,
         10.0f,  10.0f,  10.0f,
         10.0f,  10.0f,  10.0f,
        -10.0f,  10.0f,  10.0f,
        -10.0f,  10.0f, -10.0f,

        -10.0f, -10.0f, -10.0f,
        -10.0f, -10.0f,  10.0f,
         10.0f, -10.0f, -10.0f,
         10.0f, -10.0f, -10.0f,
        -10.0f, -10.0f,  10.0f,
         10.0f, -10.0f,  10.0f
    };
    
    // Create and set up VAO/VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
}

SkyBox::~SkyBox() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
    }
    
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
    }
}

GLuint SkyBox::loadCubemap(const std::vector<std::string>& faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    
    for (unsigned int i = 0; i < faces.size(); i++) {
        FilePath filePath(faces[i]);
        const Image* image = ImageManager::loadImage(filePath);
        
        if (image) {
            int width = image->getWidth();
            int height = image->getHeight();
            
            // Convertir les données de l'image au format attendu par OpenGL
            std::vector<unsigned char> rgbData(width * height * 3);
            const glm::vec4* pixels = image->getPixels();
            
            for (int j = 0; j < width * height; j++) {
                rgbData[j * 3 + 0] = static_cast<unsigned char>(pixels[j].r * 255.0f);
                rgbData[j * 3 + 1] = static_cast<unsigned char>(pixels[j].g * 255.0f);
                rgbData[j * 3 + 2] = static_cast<unsigned char>(pixels[j].b * 255.0f);
            }
            
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rgbData.data());
        } else {
            // Si l'image ne peut pas être chargée, générer une texture de base pour cette face
            unsigned char defaultColor[3] = {0, 0, 0};
            switch (i) {
                case 0: defaultColor[0] = 220; defaultColor[1] = 100; defaultColor[2] = 100; break; // Rouge
                case 1: defaultColor[0] = 100; defaultColor[1] = 220; defaultColor[2] = 100; break; // Vert
                case 2: defaultColor[0] = 100; defaultColor[1] = 100; defaultColor[2] = 220; break; // Bleu
                case 3: defaultColor[0] = 220; defaultColor[1] = 220; defaultColor[2] = 100; break; // Jaune
                case 4: defaultColor[0] = 220; defaultColor[1] = 100; defaultColor[2] = 220; break; // Magenta
                case 5: defaultColor[0] = 100; defaultColor[1] = 220; defaultColor[2] = 220; break; // Cyan
            }
            
            // Créer une texture 1x1 de la couleur appropriée
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, defaultColor);
            std::cerr << "ERROR::SKYBOX::TEXTURE_FAILED_TO_LOAD at path: " << faces[i] << std::endl;
        }
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    return textureID;
}

GLuint SkyBox::compileShader() {
    // Création des chemins possibles pour les shaders
    std::vector<FilePath> possibleBasePaths = {
        "./shaders/",
        "../shaders/",
        "../../shaders/",
        "c:/Users/anton/Documents/GitHub/projet-prog-s4/shaders/"
    };
    
    FilePath vertexPath;
    FilePath fragmentPath;
    bool foundShaders = false;
    
    for (const auto& basePath : possibleBasePaths) {
        // Essayer de trouver les fichiers de shader
        FilePath testVertexPath = basePath + "skybox.vs.glsl";
        std::ifstream testVertex(testVertexPath.c_str());
        if (testVertex.good()) {
            vertexPath = testVertexPath;
            fragmentPath = basePath + "skybox.fs.glsl";
            std::ifstream testFragment(fragmentPath.c_str());
            if (testFragment.good()) {
                foundShaders = true;
                std::cout << "Shaders trouvés au chemin: " << basePath << std::endl;
                break;
            }
        }
    }
    
    if (!foundShaders) {
        std::cerr << "ERROR::SHADER::FAILED_TO_FIND_SHADERS" << std::endl;
        return 0;
    }

    GLuint shaderProgram = glCreateProgram();
    
    try {
        // Utilisation de la fonction loadShader du module Shader
        Shader vertexShader = loadShader(GL_VERTEX_SHADER, vertexPath);
        if (!vertexShader.compile()) {
            std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << vertexShader.getInfoLog() << std::endl;
            return 0;
        }
        
        Shader fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentPath);
        if (!fragmentShader.compile()) {
            std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << fragmentShader.getInfoLog() << std::endl;
            return 0;
        }
        
        // Attacher les shaders au programme
        glAttachShader(shaderProgram, vertexShader.getGLId());
        glAttachShader(shaderProgram, fragmentShader.getGLId());
        
        // Lier le programme
        glLinkProgram(shaderProgram);
        
        // Vérifier le statut de liaison
        GLint success;
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            char infoLog[512];
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            return 0;
        }
    } catch (const std::exception& e) {
        std::cerr << "ERROR::SHADER::EXCEPTION: " << e.what() << std::endl;
        return 0;
    }

    return shaderProgram;
}

void SkyBox::Draw(const glm::mat4& view, const glm::mat4& projection) {
    if (shaderProgram == 0 || textureID == 0 || VAO == 0) {
        std::cerr << "ERROR::SKYBOX::DRAW::INVALID_STATE: Shader Program: " << shaderProgram 
                  << ", Texture ID: " << textureID 
                  << ", VAO: " << VAO << std::endl;
        return;
    }

    // Sauvegarder l'état OpenGL actuel
    GLint previousVAO;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &previousVAO);
    
    GLint previousShader;
    glGetIntegerv(GL_CURRENT_PROGRAM, &previousShader);
    
    // Activation du depth test
    glEnable(GL_DEPTH_TEST);
    
    // Sauvegarder la fonction de profondeur et la changer
    GLint previousDepthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunc);
    glDepthFunc(GL_LEQUAL);
    
    // Clear the depth buffer to make sure the skybox is drawn
    // Note: On ne veut généralement pas faire ça si d'autres objets seront dessinés
    // glClear(GL_DEPTH_BUFFER_BIT);
    
    // Supprimer la translation de la matrice de vue
    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
    
    // Utiliser le shader du skybox
    glUseProgram(shaderProgram);
    
    // Vérifier les locations des uniformes
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLint skyboxLoc = glGetUniformLocation(shaderProgram, "skybox");
    
    if (viewLoc == -1 || projLoc == -1 || skyboxLoc == -1) {
        std::cerr << "ERROR::SKYBOX::UNIFORM_LOCATIONS: view=" << viewLoc 
                  << ", projection=" << projLoc 
                  << ", skybox=" << skyboxLoc << std::endl;
    }
    
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewNoTranslation));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    // Activer la texture du skybox
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    glUniform1i(skyboxLoc, 0);
    
    // Dessiner le skybox
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    // Restaurer l'état OpenGL
    glBindVertexArray(previousVAO);
    glUseProgram(previousShader);
    glDepthFunc(previousDepthFunc);
}