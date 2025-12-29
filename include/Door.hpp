#pragma once

#include <SFML/Graphics.hpp>

class Door {
public:
    Door(const sf::Vector2f& position, float width = 80.0f, float height = 120.0f);

    void render(sf::RenderWindow& window);
    sf::FloatRect getBounds() const;
    sf::Vector2f getPosition() const { return m_position; }

private:
    void createDoorShape();

private:
    sf::Vector2f m_position;
    float m_width;
    float m_height;

    // Éléments visuels de la porte
    sf::RectangleShape m_doorFrame;      // Cadre de la porte
    sf::RectangleShape m_doorPanel;      // Panneau de la porte
    sf::CircleShape m_doorKnob;          // Poignée de la porte
    sf::RectangleShape m_doorDecoration1; // Décoration 1
    sf::RectangleShape m_doorDecoration2; // Décoration 2
};
