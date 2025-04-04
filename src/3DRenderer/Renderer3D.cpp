#include "Renderer3D.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include "utils/FilePath.hpp"
#include "utils/Image.hpp"
#include "utils/Shader.hpp"
#include "utils/Geometry.hpp"

Renderer3D::Renderer3D() 
    : m_skybox(nullptr), m_isInitialized(false), 
      cameraPosition(0.0f, 0.0f, 3.0f), cameraTarget(0.0f, 0.0f, 0.0f), cameraSpeed(5.0f)
{
    std::cout << "Renderer3D constructor called" << std::endl;
}

Renderer3D::~Renderer3D() {
    cleanup();
}

bool Renderer3D::initialize() {
    if (m_isInitialized) {
        return true; // Éviter une réinitialisation si déjà initialisé
    }

    std::cout << "Initializing 3D Renderer..." << std::endl;

    // Essayer différents chemins d'accès aux textures
    std::vector<std::string> basePaths = {
        "assets/textures/skybox/",
        "../assets/textures/skybox/",
        "../../assets/textures/skybox/",
        "c:/Users/anton/Documents/GitHub/projet-prog-s4/assets/textures/skybox/"
    };
    
    std::vector<std::string> skyboxFaces;
    bool foundPath = false;
    
    for (const auto& basePath : basePaths) {
        // Tester si le répertoire existe ou si le premier fichier est accessible
        std::string testPath = basePath + "trouble_rt.jpg";  // Utiliser le nom de fichier correct
        std::ifstream testFile(testPath);
        if (testFile.good()) {
            foundPath = true;
            skyboxFaces = {
                basePath + "trouble_rt.jpg",  // Droite
                basePath + "trouble_lf.jpg",  // Gauche
                basePath + "trouble_up.jpg",  // Haut
                basePath + "trouble_dn.jpg",  // Bas
                basePath + "trouble_ft.jpg",  // Avant
                basePath + "trouble_bk.jpg"   // Arrière
            };
            std::cout << "Textures trouvées au chemin: " << basePath << std::endl;
            break;
        }
    }
    
    if (!foundPath) {
        // Si aucun chemin n'a été trouvé, utiliser le chemin par défaut
        std::cout << "Aucun chemin de texture valide trouvé, utilisation du chemin par défaut." << std::endl;
        skyboxFaces = {
            "assets/textures/skybox/trouble_rt.jpg",
            "assets/textures/skybox/trouble_lf.jpg",
            "assets/textures/skybox/trouble_up.jpg",
            "assets/textures/skybox/trouble_dn.jpg",
            "assets/textures/skybox/trouble_ft.jpg",
            "assets/textures/skybox/trouble_bk.jpg"
        };
    }

    try {
        m_skybox = new SkyBox(skyboxFaces); // Allocation dynamique
        std::cout << "Skybox created successfully." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "ERROR::RENDERER3D::SKYBOX_EXCEPTION: " << e.what() << std::endl;
        return false;
    }

    m_isInitialized = true;
    return true;
}

bool Renderer3D::isInitialized() const {
    return m_isInitialized;
}

void Renderer3D::update(float deltaTime) {
}

void Renderer3D::cleanup() {
    if (m_skybox) {
        delete m_skybox; // Libération explicite de la mémoire
        m_skybox = nullptr;
    }
    m_isInitialized = false;
}
