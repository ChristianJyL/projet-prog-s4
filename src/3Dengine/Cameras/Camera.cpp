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

      m_fDistance(5.0f),
      m_fAngleX(0.0f),
      m_fAngleY(0.0f),
      m_minDistance(5.0f),
      m_maxDistance(20.0f),
      m_minAngleX(glm::radians(10.0f)),
      m_maxAngleX(glm::radians(85.0f))
{
    setInitialPosition();
}

void Camera::update(float deltaTime) {
    // Cette méthode peut être utilisée pour des mises à jour automatiques
}

void Camera::moveFront(float delta) {
    m_fDistance -= delta; // Réduire la distance quand delta > 0 (avancer)
    m_fDistance = glm::clamp(m_fDistance, m_minDistance, m_maxDistance);
    updateCameraPosition();
}

void Camera::rotateLeft(float degrees) {
    m_fAngleY += degrees;
    // Normaliser l'angle pour rester dans [-π, π]
    while (m_fAngleY > glm::pi<float>()) m_fAngleY -= glm::two_pi<float>();
    while (m_fAngleY < -glm::pi<float>()) m_fAngleY += glm::two_pi<float>();
    updateCameraPosition();
}

void Camera::rotateUp(float degrees) {
    m_fAngleX += degrees;
    // Limiter les angles de rotation verticale pour éviter les problèmes
    m_fAngleX = glm::clamp(m_fAngleX, m_minAngleX, m_maxAngleX);
    updateCameraPosition();
}

glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(m_fov), m_aspectRatio, m_nearPlane, m_farPlane);
}

void Camera::setAspectRatio(float ratio) {
    m_aspectRatio = ratio;
}

void Camera::setTarget(const glm::vec3& target) {
    m_target = target;
    updateCameraPosition();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(m_position, m_target, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Camera::setInitialPosition() {
    // Initialiser avec des valeurs qui donnent une bonne vue initiale
    m_target = glm::vec3(0.0f, 0.0f, 0.0f);
    m_fDistance = 15.0f;
    m_fAngleX = glm::radians(50.0f);
    m_fAngleY = glm::radians(0.0f);
    updateCameraPosition();
}

void Camera::updateCameraPosition() {
    // Calculer la position de la caméra à partir des angles et de la distance
    m_position.x = m_target.x + m_fDistance * cos(m_fAngleX) * sin(m_fAngleY);
    m_position.y = m_target.y + m_fDistance * sin(m_fAngleX);
    m_position.z = m_target.z + m_fDistance * cos(m_fAngleX) * cos(m_fAngleY);
}
