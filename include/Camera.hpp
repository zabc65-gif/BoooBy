#pragma once

#include <SFML/Graphics.hpp>

class Camera {
public:
    Camera(float viewWidth, float viewHeight);

    void update(const sf::Vector2f& targetPosition, sf::Time deltaTime, bool playerIsMoving = false);
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

    // Effet de "breathing" (mouvement subtil de la caméra)
    float m_breathingTimer;
    float m_breathingIntensity;  // Intensité actuelle du breathing (0.0 à 1.0)
    sf::Vector2f m_lastTargetPosition;  // Pour détecter le mouvement
    static constexpr float BREATHING_AMPLITUDE = 1.0f;  // Déplacement de 1 pixel max
    static constexpr float BREATHING_SPEED = 0.15f;     // Vitesse du mouvement (cycles par seconde)
    static constexpr float BREATHING_ROTATION = 0.15f;  // Rotation maximale en degrés
    static constexpr float BREATHING_FADE_SPEED = 2.0f; // Vitesse de transition (unités par seconde)
};
