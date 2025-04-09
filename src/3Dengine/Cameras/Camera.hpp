#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "../../Chess/Piece.hpp" // Pour PieceColor

// Définition des modes de caméra
enum class CameraMode {
    Trackball,  // Mode par défaut (orbite autour de la cible)
    Piece       // Vue depuis une pièce
};

class Camera {
public:
    Camera();
    virtual ~Camera() = default;
    
    // Méthodes de mouvement de caméra
    void moveFront(float delta);
    void rotateLeft(float degrees);
    void rotateUp(float degrees);
    
    // Méthodes de gestion de la vue
    void update(float deltaTime);
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    
    // Méthodes de configuration
    void setAspectRatio(float ratio);
    void setTarget(const glm::vec3& target);
    const glm::vec3& getTarget() const { return m_target; }
    void setInitialPosition();
    
    // Méthodes pour le mode pièce
    void setPieceView(const glm::vec3& piecePosition, PieceColor color = PieceColor::White);
    void toggleCameraMode();
    CameraMode getCameraMode() const { return m_currentMode; }
    
private:
    void updateCameraPosition();
    
    // Paramètres de projection
    float m_aspectRatio;
    float m_fov;
    float m_nearPlane;
    float m_farPlane;
    
    // Position et orientation
    glm::vec3 m_position;
    glm::vec3 m_target;
    
    // Mode de caméra actuel
    CameraMode m_currentMode;
    
    // Paramètres spécifiques au mode trackball
    float m_fDistance;    // Distance par rapport au centre
    float m_fAngleX;      // Angle autour de l'axe X (haut/bas)
    float m_fAngleY;      // Angle autour de l'axe Y (gauche/droite)
    
    // Limites du mode trackball
    float m_minDistance;
    float m_maxDistance;
    float m_minAngleX;
    float m_maxAngleX;
    
    // Paramètres spécifiques au mode pièce
    glm::vec3 m_piecePosition;  // Position de la pièce
    float m_horizontalAngle;    // Angle horizontal (gauche/droite)
    float m_verticalAngle;      // Angle vertical (haut/bas)
};
