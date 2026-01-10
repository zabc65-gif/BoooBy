#include "Camera.hpp"
#include <algorithm>
#include <cmath>

Camera::Camera(float viewWidth, float viewHeight)
    : m_hasBounds(false)
    , m_smoothness(0.1f)
    , m_breathingTimer(0.0f)
    , m_breathingIntensity(1.0f)
    , m_lastTargetPosition(0.0f, 0.0f)
{
    m_view.setSize(sf::Vector2f(viewWidth, viewHeight));
    m_view.setCenter(sf::Vector2f(viewWidth / 2.0f, viewHeight / 2.0f));
}

void Camera::update(const sf::Vector2f& targetPosition, sf::Time deltaTime, bool playerIsMoving) {
    sf::Vector2f currentCenter = m_view.getCenter();

    // Interpolation linéaire pour un suivi fluide
    float interpolation = std::min(1.0f, m_smoothness * deltaTime.asSeconds() * 60.0f);
    sf::Vector2f newCenter = currentCenter + (targetPosition - currentCenter) * interpolation;

    // Appliquer les limites si elles existent
    if (m_hasBounds) {
        sf::Vector2f viewSize = m_view.getSize();
        float halfWidth = viewSize.x / 2.0f;
        float halfHeight = viewSize.y / 2.0f;

        // Limiter la caméra pour qu'elle ne sorte pas des bounds
        newCenter.x = std::max(m_bounds.position.x + halfWidth, newCenter.x);
        newCenter.x = std::min(m_bounds.position.x + m_bounds.size.x - halfWidth, newCenter.x);

        newCenter.y = std::max(m_bounds.position.y + halfHeight, newCenter.y);
        newCenter.y = std::min(m_bounds.position.y + m_bounds.size.y - halfHeight, newCenter.y);
    }

    // Ajuster l'intensité du breathing en fonction du mouvement du joueur
    float targetIntensity = playerIsMoving ? 0.0f : 1.0f;
    float intensityChange = BREATHING_FADE_SPEED * deltaTime.asSeconds();

    if (m_breathingIntensity < targetIntensity) {
        m_breathingIntensity = std::min(targetIntensity, m_breathingIntensity + intensityChange);
    } else if (m_breathingIntensity > targetIntensity) {
        m_breathingIntensity = std::max(targetIntensity, m_breathingIntensity - intensityChange);
    }

    // Effet de "breathing" - mouvement subtil oscillant
    m_breathingTimer += deltaTime.asSeconds();
    const float PI = 3.14159265359f;

    // Calculer les offsets sinusoïdaux pour un mouvement organique
    float timeX = m_breathingTimer * BREATHING_SPEED * 2.0f * PI;
    float timeY = m_breathingTimer * BREATHING_SPEED * 2.0f * PI * 0.7f;  // Fréquence légèrement différente

    // Appliquer l'intensité aux offsets
    float offsetX = std::sin(timeX) * BREATHING_AMPLITUDE * m_breathingIntensity;
    float offsetY = std::sin(timeY) * BREATHING_AMPLITUDE * 0.5f * m_breathingIntensity;

    // Ajouter l'offset de breathing au centre
    newCenter.x += offsetX;
    newCenter.y += offsetY;

    m_view.setCenter(newCenter);

    // Ajouter une rotation très légère (aussi affectée par l'intensité)
    float rotationDegrees = std::sin(timeX * 0.5f) * BREATHING_ROTATION * m_breathingIntensity;
    m_view.setRotation(sf::degrees(rotationDegrees));

    // Mémoriser la position pour la prochaine frame
    m_lastTargetPosition = targetPosition;
}

void Camera::setPosition(const sf::Vector2f& position) {
    m_view.setCenter(position);
}

void Camera::setBounds(float left, float top, float width, float height) {
    m_hasBounds = true;
    m_bounds = sf::FloatRect(sf::Vector2f(left, top), sf::Vector2f(width, height));
}
