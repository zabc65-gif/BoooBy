#pragma once

#include <SFML/Graphics.hpp>
#include "Tilemap.hpp"
#include "Player.hpp"
#include <memory>

class Level {
public:
    Level();

    bool load();
    bool loadFromFile(const std::string& filepath);
    void update(sf::Time deltaTime, Player& player);
    void render(sf::RenderWindow& window);

    // Collision avec le joueur
    void handlePlayerCollision(Player& player);

    // Ligne d'arrivée
    bool isPlayerAtFinish(const Player& player) const;
    sf::FloatRect getFinishLineBounds() const { return m_finishLine; }

    // Portails
    sf::Vector2f getEntrancePortalPosition() const { return m_entrancePortalPosition; }
    bool hasEntrancePortal() const { return m_hasEntrancePortal; }
    bool hasExitPortal() const { return m_hasExitPortal; }
    sf::Vector2f getExitPortalPosition() const { return m_exitPortalPosition; }

    // Vérifier si un niveau est valide (fichier existe + a les 2 portails)
    static bool isLevelValid(int levelNumber);

private:
    void createSimpleLevel();

private:
    std::unique_ptr<Tilemap> m_tilemap;
    sf::FloatRect m_finishLine;

    // Portail d'entrée pour positionner le joueur
    sf::Vector2f m_entrancePortalPosition;
    bool m_hasEntrancePortal;

    // Portail de sortie
    sf::Vector2f m_exitPortalPosition;
    bool m_hasExitPortal;

    // Porte médiévale
    sf::Texture m_doorTexture;
    std::unique_ptr<sf::Sprite> m_entranceDoorSprite;
    std::unique_ptr<sf::Sprite> m_exitDoorSprite;
    bool m_doorTextureLoaded;

    // Info du niveau
    bool m_isPrologueLevel;  // true si c'est le niveau prologue
};
