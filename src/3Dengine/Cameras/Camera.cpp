#include "Camera.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

Camera::Camera()
    : m_aspectRatio(16.0f/9.0f), 
      m_fov(45.0f), 
      m_nearPlane(0.1f), 
      m_farPlane(100.0f),
      m_position(0.0f, 0.0f, 0.0f),
      m_target(0.0f, 0.0f, 0.0f),
      m_currentMode(CameraMode::Trackball),
      m_fDistance(5.0f),
      m_fAngleX(0.0f),
      m_fAngleY(0.0f),
      m_minDistance(5.0f),
      m_maxDistance(20.0f),
      m_minAngleX(glm::radians(10.0f)),
      m_maxAngleX(glm::radians(85.0f)),
      m_piecePosition(0.0f, 0.0f, 0.0f),
      m_horizontalAngle(0.0f),
      m_verticalAngle(0.0f)
{
    setInitialPosition();
}

void Camera::update(float deltaTime) {
    // Cette méthode peut être utilisée pour des mises à jour automatiques
}

void Camera::moveFront(float delta) {
    if (m_currentMode == CameraMode::Trackball) {
        m_fDistance -= delta;
        m_fDistance = glm::clamp(m_fDistance, m_minDistance, m_maxDistance);
        updateCameraPosition();
    }
}

void Camera::rotateLeft(float degrees) {
    if (m_currentMode == CameraMode::Trackball) {
        m_fAngleY += degrees;
        while (m_fAngleY > glm::pi<float>()) m_fAngleY -= glm::two_pi<float>();
        while (m_fAngleY < -glm::pi<float>()) m_fAngleY += glm::two_pi<float>();
        updateCameraPosition();
    } 
    else if (m_currentMode == CameraMode::Piece) {
        m_horizontalAngle += degrees;
        while (m_horizontalAngle > glm::pi<float>()) m_horizontalAngle -= glm::two_pi<float>();
        while (m_horizontalAngle < -glm::pi<float>()) m_horizontalAngle += glm::two_pi<float>();
    }
}

void Camera::rotateUp(float degrees) {
    if (m_currentMode == CameraMode::Trackball) {
        m_fAngleX += degrees;
        m_fAngleX = glm::clamp(m_fAngleX, m_minAngleX, m_maxAngleX);
        updateCameraPosition();
    } 
    else if (m_currentMode == CameraMode::Piece) {
        m_verticalAngle -= degrees;
        m_verticalAngle = glm::clamp(m_verticalAngle, glm::radians(-89.0f), glm::radians(89.0f));
    }
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
}

void Camera::setAspectRatio(float ratio) {
    m_aspectRatio = ratio;
}

void Camera::setTarget(const glm::vec3& target) {
    m_target = target;
    if (m_currentMode == CameraMode::Trackball) {
        updateCameraPosition();
    }
}

glm::mat4 Camera::getViewMatrix() const {
    if (m_currentMode == CameraMode::Trackball) {
        return glm::lookAt(m_position, m_target, glm::vec3(0.0f, 1.0f, 0.0f));
    } 
    else {
        glm::vec3 direction(
            cos(m_verticalAngle) * sin(m_horizontalAngle),
            sin(m_verticalAngle),
            cos(m_verticalAngle) * cos(m_horizontalAngle)
        );
        
        direction = glm::normalize(direction);
        
        glm::vec3 pieceTarget = m_position + direction;
        
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        
        return glm::lookAt(m_position, pieceTarget, up);
    }
}

void Camera::setInitialPosition() {
    m_target = glm::vec3(0.0f, 0.0f, 0.0f);
    m_fDistance = 15.0f;
    m_fAngleX = glm::radians(50.0f);
    m_fAngleY = glm::radians(0.0f);
    m_currentMode = CameraMode::Trackball;
    m_horizontalAngle = glm::radians(0.0f);
    m_verticalAngle = glm::radians(0.0f);
    updateCameraPosition();
}

void Camera::updateCameraPosition() {
    m_position.x = m_target.x + m_fDistance * cos(m_fAngleX) * sin(m_fAngleY);
    m_position.y = m_target.y + m_fDistance * sin(m_fAngleX);
    m_position.z = m_target.z + m_fDistance * cos(m_fAngleX) * cos(m_fAngleY);
}

void Camera::setPieceView(const glm::vec3& piecePosition, PieceColor color) {
    // Positionner la caméra juste au-dessus de la pièce
    m_piecePosition = piecePosition;
    m_position = m_piecePosition + glm::vec3(0.0f, 1.0f, 0.0f); 
    
    // Orienter la caméra selon la couleur de la pièce
    m_horizontalAngle = (color == PieceColor::White) ? glm::radians(0.0f) : glm::radians(180.0f);
    
    // Incliner légèrement la caméra vers le bas pour voir le plateau
    m_verticalAngle = glm::radians(-10.0f);
    
    m_currentMode = CameraMode::Piece;
}

void Camera::toggleCameraMode() {
    if (m_currentMode == CameraMode::Trackball) {
        // Le mode pièce nécessite d'abord une sélection de pièce
        std::cout << "Utilisez setPieceView() pour passer en mode pièce" << std::endl;
    } else {
        // Retour au mode trackball standard
        m_currentMode = CameraMode::Trackball;
        updateCameraPosition();
    }
}



