#include "skybox.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstring> 


using namespace glBurnout;

// Constructeur sans paramètre
SkyBox::SkyBox() 
    : textureID(0), VAO(0), VBO(0), m_isInitialized(false)
{
}

SkyBox::SkyBox(const std::vector<std::string>& faces) 
    : textureID(0), VAO(0), VBO(0), m_isInitialized(false)
{
    // Compile shaders
    if (!compileShader()) {
        std::cerr << "ERROR::SKYBOX::SHADER_COMPILATION_FAILED" << std::endl;
        return;
    }
    
    // Load cubemap texture
    textureID = loadCubemap(faces);
    
    // Si tout est OK, préparer les vertex data
    if (m_skyboxShader.getGLId() != 0 && textureID != 0) {
        float skyboxVertices[] = {
            // Face arrière (-Z)
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            // Face gauche (-X)
            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            // Face droite (+X)
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            // Face avant (+Z)
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            // Face haute (+Y)
            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,
            
            // Face basse (-Y)
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f
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
        
        m_isInitialized = true;
    }
}

bool SkyBox::initialize() {
    if (m_isInitialized) {
        return true; // Éviter une réinitialisation si déjà initialisé
    }
    
    std::cout << "Initializing SkyBox..." << std::endl;
    
    // Trouver les textures
    std::vector<std::string> skyboxFaces = findSkyboxTextures();
    if (skyboxFaces.empty()) {
        std::cerr << "ERROR::SKYBOX::NO_TEXTURES_FOUND" << std::endl;
        return false;
    }
    
    // Compiler les shaders
    if (!compileShader()) {
        std::cerr << "ERROR::SKYBOX::SHADER_COMPILATION_FAILED" << std::endl;
        return false;
    }
    
    // Charger les textures
    textureID = loadCubemap(skyboxFaces);
    if (textureID == 0) {
        std::cerr << "ERROR::SKYBOX::TEXTURE_LOADING_FAILED" << std::endl;
        return false;
    }
    
    float skyboxVertices[] = {
        // Face arrière (-Z)
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        // Face gauche (-X)
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        // Face droite (+X)
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        // Face avant (+Z)
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        // Face haute (+Y)
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        
        // Face basse (-Y)
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f
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
    
    m_isInitialized = true;
    std::cout << "SkyBox initialization successful" << std::endl;
    return true;
}

std::vector<std::string> SkyBox::findSkyboxTextures() {
    // Utiliser un chemin fixe pour les textures
    std::string basePath = "../../assets/";
    
    std::vector<std::string> skyboxFaces = {
        basePath + "textures/skybox/harmony_rt.jpg",  // Right
        basePath + "textures/skybox/harmony_lf.jpg",  // Left
        basePath + "textures/skybox/harmony_up.jpg",  // Top
        basePath + "textures/skybox/harmony_dn.jpg",  // Bottom
        basePath + "textures/skybox/harmony_bk.jpg",  // Back
        basePath + "textures/skybox/harmony_ft.jpg"   // Front
    };
    
    std::cout << "SkyBox textures path: " << basePath << "textures/skybox/" << std::endl;
    return skyboxFaces;
}

SkyBox::~SkyBox() {
    if (VAO != 0) {
        glDeleteVertexArrays(1, &VAO);
    }
    
    if (VBO != 0) {
        glDeleteBuffers(1, &VBO);
    }
    
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
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

bool SkyBox::compileShader() {
    try {
        // Chemin vers les fichiers de shader
        std::string basePath = "../../shaders/";
        std::string vertexShaderPath = basePath + "skybox.vs.glsl";
        std::string fragmentShaderPath = basePath + "skybox.fs.glsl";
        
        // Vérifier si les fichiers existent
        std::ifstream testVertex(vertexShaderPath);
        std::ifstream testFragment(fragmentShaderPath);
        
        if (!testVertex.good() || !testFragment.good()) {
            std::cerr << "ERROR::SHADER::FAILED_TO_FIND_SHADERS at path: " << basePath << std::endl;
            return false;
        }
        
        std::cout << "Shaders de skybox trouvés au chemin: " << basePath << std::endl;
        
        // Utiliser les utilitaires existants pour charger les shaders depuis les fichiers
        FilePath vsPath(vertexShaderPath);
        FilePath fsPath(fragmentShaderPath);
        
        // Charger le programme shader à partir des fichiers
        m_skyboxShader = loadProgram(vsPath, fsPath);
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "ERROR::SHADER::EXCEPTION: " << e.what() << std::endl;
        return false;
    }
}

void SkyBox::Draw(const glm::mat4& view, const glm::mat4& projection) {
    if (!m_isInitialized) {
        std::cerr << "ERROR::SKYBOX::DRAW::NOT_INITIALIZED" << std::endl;
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

    // Supprimer la translation de la matrice de vue
    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
    
    // Utiliser le shader du skybox
    m_skyboxShader.use();
    
    // Vérifier les locations des uniformes
    GLint viewLoc = glGetUniformLocation(m_skyboxShader.getGLId(), "view");
    GLint projLoc = glGetUniformLocation(m_skyboxShader.getGLId(), "projection");
    GLint skyboxLoc = glGetUniformLocation(m_skyboxShader.getGLId(), "skybox");
    
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