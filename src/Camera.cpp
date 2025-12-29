#include "Camera.hpp"
#include <algorithm>

Camera::Camera(float viewWidth, float viewHeight)
    : m_hasBounds(false)
    , m_smoothness(0.1f)
{
    m_view.setSize(sf::Vector2f(viewWidth, viewHeight));
    m_view.setCenter(sf::Vector2f(viewWidth / 2.0f, viewHeight / 2.0f));
}

void Camera::update(const sf::Vector2f& targetPosition, sf::Time deltaTime) {
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

    m_view.setCenter(newCenter);
}

void Camera::setPosition(const sf::Vector2f& position) {
    m_view.setCenter(position);
}

void Camera::setBounds(float left, float top, float width, float height) {
    m_hasBounds = true;
    m_bounds = sf::FloatRect(sf::Vector2f(left, top), sf::Vector2f(width, height));
}
