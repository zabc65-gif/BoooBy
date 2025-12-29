#include "Door.hpp"

Door::Door(const sf::Vector2f& position, float width, float height)
    : m_position(position)
    , m_width(width)
    , m_height(height)
{
    createDoorShape();
}

void Door::createDoorShape() {
    // Cadre de la porte (bois foncé)
    m_doorFrame.setSize(sf::Vector2f(m_width, m_height));
    m_doorFrame.setPosition(m_position);
    m_doorFrame.setFillColor(sf::Color(60, 40, 20)); // Brun foncé
    m_doorFrame.setOutlineThickness(3.0f);
    m_doorFrame.setOutlineColor(sf::Color(40, 25, 10)); // Brun très foncé

    // Panneau de la porte (bois plus clair)
    m_doorPanel.setSize(sf::Vector2f(m_width - 12.0f, m_height - 12.0f));
    m_doorPanel.setPosition(sf::Vector2f(m_position.x + 6.0f, m_position.y + 6.0f));
    m_doorPanel.setFillColor(sf::Color(100, 70, 40)); // Brun moyen

    // Poignée de la porte (dorée)
    m_doorKnob.setRadius(4.0f);
    m_doorKnob.setPosition(sf::Vector2f(
        m_position.x + m_width - 20.0f,
        m_position.y + m_height / 2.0f
    ));
    m_doorKnob.setFillColor(sf::Color(218, 165, 32)); // Or

    // Décoration 1 (panneau supérieur)
    m_doorDecoration1.setSize(sf::Vector2f(m_width - 24.0f, m_height / 2.5f - 12.0f));
    m_doorDecoration1.setPosition(sf::Vector2f(m_position.x + 12.0f, m_position.y + 12.0f));
    m_doorDecoration1.setFillColor(sf::Color(80, 55, 30));
    m_doorDecoration1.setOutlineThickness(2.0f);
    m_doorDecoration1.setOutlineColor(sf::Color(60, 40, 20));

    // Décoration 2 (panneau inférieur)
    m_doorDecoration2.setSize(sf::Vector2f(m_width - 24.0f, m_height / 2.0f - 12.0f));
    m_doorDecoration2.setPosition(sf::Vector2f(
        m_position.x + 12.0f,
        m_position.y + m_height / 2.0f + 6.0f
    ));
    m_doorDecoration2.setFillColor(sf::Color(80, 55, 30));
    m_doorDecoration2.setOutlineThickness(2.0f);
    m_doorDecoration2.setOutlineColor(sf::Color(60, 40, 20));
}

void Door::render(sf::RenderWindow& window) {
    window.draw(m_doorFrame);
    window.draw(m_doorPanel);
    window.draw(m_doorDecoration1);
    window.draw(m_doorDecoration2);
    window.draw(m_doorKnob);
}

sf::FloatRect Door::getBounds() const {
    return m_doorFrame.getGlobalBounds();
}
