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
    // Le mouvement n'est possible qu'en mode trackball
    if (m_currentMode == CameraMode::Trackball) {
        m_fDistance -= delta; // Réduire la distance quand delta > 0 (avancer)
        m_fDistance = glm::clamp(m_fDistance, m_minDistance, m_maxDistance);
        updateCameraPosition();
    }
}

void Camera::rotateLeft(float degrees) {
    if (m_currentMode == CameraMode::Trackball) {
        m_fAngleY += degrees;
        // Normaliser l'angle pour rester dans [-π, π]
        while (m_fAngleY > glm::pi<float>()) m_fAngleY -= glm::two_pi<float>();
        while (m_fAngleY < -glm::pi<float>()) m_fAngleY += glm::two_pi<float>();
        updateCameraPosition();
    } 
    else if (m_currentMode == CameraMode::Piece) {
        m_horizontalAngle += degrees; // Inversion supprimée, on utilise le même sens que le mode trackball
        // Normaliser l'angle pour rester dans [-π, π]
        while (m_horizontalAngle > glm::pi<float>()) m_horizontalAngle -= glm::two_pi<float>();
        while (m_horizontalAngle < -glm::pi<float>()) m_horizontalAngle += glm::two_pi<float>();
    }
}

void Camera::rotateUp(float degrees) {
    if (m_currentMode == CameraMode::Trackball) {
        m_fAngleX += degrees;
        // Limiter les angles de rotation verticale pour éviter les problèmes
        m_fAngleX = glm::clamp(m_fAngleX, m_minAngleX, m_maxAngleX);
        updateCameraPosition();
    } 
    else if (m_currentMode == CameraMode::Piece) {
        // Inverser le sens du mouvement vertical pour une expérience plus intuitive
        m_verticalAngle -= degrees;  // Mouvement inversé pour correspondre au mouvement naturel de la souris
        
        // En mode pièce, on peut regarder partout (haut/bas sans limitation)
        // Limiter à -89° et +89° pour éviter le gimbal lock
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
    else { // Mode pièce
        // En mode pièce, on calcule la cible en fonction de la direction du regard
        glm::vec3 direction(
            cos(m_verticalAngle) * sin(m_horizontalAngle),
            sin(m_verticalAngle),
            cos(m_verticalAngle) * cos(m_horizontalAngle)
        );
        
        // Normaliser la direction
        direction = glm::normalize(direction);
        
        // Calculer la cible comme étant la position + direction
        glm::vec3 pieceTarget = m_position + direction;
        
        // Vecteur "up" toujours vertical
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        
        return glm::lookAt(m_position, pieceTarget, up);
    }
}

void Camera::setInitialPosition() {
    // Initialiser avec des valeurs qui donnent une bonne vue initiale
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
    // Calculer la position de la caméra à partir des angles et de la distance
    m_position.x = m_target.x + m_fDistance * cos(m_fAngleX) * sin(m_fAngleY);
    m_position.y = m_target.y + m_fDistance * sin(m_fAngleX);
    m_position.z = m_target.z + m_fDistance * cos(m_fAngleX) * cos(m_fAngleY);
}

void Camera::setPieceView(const glm::vec3& piecePosition, PieceColor color) {
    std::cout << "setPieceView appelé avec position: (" << piecePosition.x << ", " 
              << piecePosition.y << ", " << piecePosition.z << ")" 
              << " et couleur: " << (color == PieceColor::White ? "Blanche" : "Noire") << std::endl;
    
    // Positionner la caméra au sommet de la pièce
    m_piecePosition = piecePosition;
    m_position = m_piecePosition + glm::vec3(0.0f, 1.0f, 0.0f); 
    
    // Orientation de la cam en fonction de la couleur de la pièce
    if (color == PieceColor::White) {
        m_horizontalAngle = glm::radians(0.0f); 
    } else {
        m_horizontalAngle = glm::radians(180.0f); 
    }
    
    //léger tilt vers le bas pour voir le plateau...
    m_verticalAngle = glm::radians(-10.0f);
    
    // Activer le mode pièce
    m_currentMode = CameraMode::Piece;
    /*
    std::cout << "Mode caméra : " << (m_currentMode == CameraMode::Trackball ? "Trackball" : "Pièce") << std::endl;
    std::cout << "New position cam: (" << m_position.x << ", " << m_position.y << ", " << m_position.z << ")" << std::endl;
    std::cout << "Orientation: horizontale=" << glm::degrees(m_horizontalAngle) << "°, verticale=" << glm::degrees(m_verticalAngle) << "°" << std::endl;
    */
    // Mettre à jour la position de vue
}

void Camera::toggleCameraMode() {
    std::cout << "Camera::toggleCameraMode appelé" << std::endl;
    
    if (m_currentMode == CameraMode::Trackball) {
        // Basculer vers le mode pièce nécessite une position de pièce
        // La position est gérée par setPieceView(), donc juste montrer un message ici
        std::cout << "Impossible de basculer directement du mode trackball au mode pièce" << std::endl;
        std::cout << "Utilisez setPieceView() à la place" << std::endl;
    } else {
        // Revenir au mode trackball
        std::cout << "Retour au mode trackball depuis le mode pièce" << std::endl;
        m_currentMode = CameraMode::Trackball;
        updateCameraPosition();
    }
    
    std::cout << "Mode actuel après toggle: " << (m_currentMode == CameraMode::Trackball ? "Trackball" : "Pièce") << std::endl;
}



