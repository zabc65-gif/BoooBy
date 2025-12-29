#pragma once

#include <SFML/Graphics.hpp>
#include "Tilemap.hpp"
#include "Player.hpp"
#include <memory>

class Level {
public:
    Level();

    bool load();
    void update(sf::Time deltaTime, Player& player);
    void render(sf::RenderWindow& window);

    // Collision avec le joueur
    void handlePlayerCollision(Player& player);

    // Ligne d'arrivée
    bool isPlayerAtFinish(const Player& player) const;
    sf::FloatRect getFinishLineBounds() const { return m_finishLine; }

private:
    void createSimpleLevel();

private:
    std::unique_ptr<Tilemap> m_tilemap;
    sf::FloatRect m_finishLine;
};
