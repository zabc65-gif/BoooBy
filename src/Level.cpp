#include "Level.hpp"
#include <iostream>

Level::Level()
    : m_tilemap(std::make_unique<Tilemap>(32))
    , m_finishLine(sf::Vector2f(0, 0), sf::Vector2f(0, 0))
{
}

bool Level::load() {
    // Charger le tileset
    if (!m_tilemap->loadFromFile("assets/tiles/Mossy Tileset/Mossy - TileSet.png")) {
        return false;
    }

    // Créer un niveau simple
    createSimpleLevel();

    std::cout << "Level loaded successfully" << std::endl;
    return true;
}

void Level::createSimpleLevel() {
    // Créer un niveau simple: un couloir horizontal avec sol et plafond
    // -1 = vide (air)
    // 0 = tile de sol/mur (on utilisera la première tile du tileset)

    const int levelWidth = 50;  // 50 tiles de large
    const int levelHeight = 23; // 23 tiles de haut (720 / 32 ≈ 22.5)

    std::vector<std::vector<int>> levelData(levelHeight, std::vector<int>(levelWidth, -1));

    // Sol (2 lignes en bas)
    for (int x = 0; x < levelWidth; ++x) {
        levelData[levelHeight - 1][x] = 48; // Tile de sol
        levelData[levelHeight - 2][x] = 32; // Tile sous le sol
    }

    // Plafond (1 ligne en haut)
    for (int x = 0; x < levelWidth; ++x) {
        levelData[0][x] = 16; // Tile de plafond
    }

    // Murs gauche et droit
    for (int y = 1; y < levelHeight - 2; ++y) {
        levelData[y][0] = 17; // Mur gauche
        levelData[y][levelWidth - 1] = 19; // Mur droit
    }

    // Quelques plateformes pour rendre le niveau plus intéressant
    // Plateforme 1 (milieu-gauche)
    for (int x = 8; x < 12; ++x) {
        levelData[levelHeight - 6][x] = 48;
    }

    // Plateforme 2 (milieu)
    for (int x = 18; x < 24; ++x) {
        levelData[levelHeight - 9][x] = 48;
    }

    // Plateforme 3 (milieu-droit)
    for (int x = 30; x < 35; ++x) {
        levelData[levelHeight - 6][x] = 48;
    }

    m_tilemap->loadFromData(levelData, 16);

    // Ligne d'arrivée à la fin du niveau (déplacée plus loin pour que le joueur la traverse visuellement)
    m_finishLine = sf::FloatRect(
        sf::Vector2f((levelWidth - 2) * 32.0f, (levelHeight - 10) * 32.0f),  // Position (levelWidth - 2 au lieu de - 3)
        sf::Vector2f(32.0f * 1, 32.0f * 8)                                     // Taille (1 tile de large au lieu de 2)
    );
}

void Level::update(sf::Time deltaTime, Player& player) {
    handlePlayerCollision(player);
}

void Level::handlePlayerCollision(Player& player) {
    // Récupérer la position et la vélocité du joueur
    sf::Vector2f position = player.getPosition();

    // Taille approximative du joueur (en se basant sur le sprite à l'échelle 0.2)
    const float playerWidth = 102.0f;  // 512 * 0.2
    const float playerHeight = 102.0f;

    // Calculer les tiles autour du joueur
    int tileSize = m_tilemap->getTileSize();

    int leftTile = static_cast<int>(position.x / tileSize);
    int rightTile = static_cast<int>((position.x + playerWidth) / tileSize);
    int topTile = static_cast<int>(position.y / tileSize);
    int bottomTile = static_cast<int>((position.y + playerHeight) / tileSize);

    // Vérifier collision avec le sol (en dessous du joueur)
    bool onGround = false;
    for (int x = leftTile; x <= rightTile; ++x) {
        if (m_tilemap->isSolid(x, bottomTile + 1)) {
            onGround = true;
            // Ajuster la position Y pour être sur le sol
            float groundY = bottomTile * tileSize - playerHeight;
            if (position.y > groundY) {
                player.setPosition(sf::Vector2f(position.x, groundY));
            }
            break;
        }
    }

    // Note: Pour l'instant, on laisse Player.cpp gérer la gravité et le sol temporaire
    // Dans une version plus avancée, on pourrait complètement gérer les collisions ici
}

bool Level::isPlayerAtFinish(const Player& player) const {
    sf::Vector2f playerPos = player.getPosition();

    // Vérifier si le centre du joueur a franchi la ligne d'arrivée (pas juste une intersection)
    float playerCenterX = playerPos.x + 102.0f / 2.0f;  // Centre horizontal du joueur
    float finishLineX = m_finishLine.position.x;

    // Le joueur doit avoir franchi la ligne d'arrivée
    return playerCenterX >= finishLineX;
}

void Level::render(sf::RenderWindow& window) {
    // Dessiner la tilemap
    m_tilemap->render(window);

    // Dessiner la ligne d'arrivée (drapeau visuel)
    sf::RectangleShape finishFlag(sf::Vector2f(m_finishLine.size.x, m_finishLine.size.y));
    finishFlag.setPosition(sf::Vector2f(m_finishLine.position.x, m_finishLine.position.y));
    finishFlag.setFillColor(sf::Color(255, 215, 0, 100)); // Or transparent
    finishFlag.setOutlineColor(sf::Color(255, 215, 0));
    finishFlag.setOutlineThickness(2.0f);
    window.draw(finishFlag);
}
