#pragma once

#include <SFML/Graphics.hpp>

class Camera {
public:
    Camera(float viewWidth, float viewHeight);

    void update(const sf::Vector2f& targetPosition, sf::Time deltaTime);
    void setPosition(const sf::Vector2f& position);

    sf::View getView() const { return m_view; }
    sf::Vector2f getPosition() const { return m_view.getCenter(); }

    void setBounds(float left, float top, float width, float height);

private:
    sf::View m_view;

    // Limites de la caméra (optionnel)
    bool m_hasBounds;
    sf::FloatRect m_bounds;

    // Paramètres de suivi
    float m_smoothness;  // Plus c'est élevé, plus le suivi est lisse (0 = instantané)
};
