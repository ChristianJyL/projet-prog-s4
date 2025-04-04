#pragma once

#include <vector>
#include <string>
#include <imgui.h>
#include <glm/glm.hpp>
#include <stdexcept> 
#include "skybox.hpp"

class Renderer3D {
private:
    // Skybox
    SkyBox* m_skybox; 
    bool m_isInitialized;
    

    // Camera
    glm::vec3 cameraPosition;
    glm::vec3 cameraTarget;
    float cameraSpeed;

public:
    Renderer3D();
    ~Renderer3D();

    bool initialize();
    bool isInitialized() const; // Déclaration de la méthode pour vérifier l'état
    void cleanup();

    // Camera control
    void setCameraPosition(const glm::vec3& position) { cameraPosition = position; }
    void setCameraTarget(const glm::vec3& target) { cameraTarget = target; }
    glm::vec3 getCameraPosition() const { return cameraPosition; }
    glm::vec3 getCameraTarget() const { return cameraTarget; }

    // Access to SkyBox
    SkyBox& getSkybox() { 
        if (!m_skybox) {
            throw std::runtime_error("Skybox n'est pas initialisée"); // Lève une exception si le skybox n'est pas initialisé
        }
        return *m_skybox; 
    } // Retourne une référence à l'objet SkyBox

    // Animation and updates
    void update(float deltaTime);
};