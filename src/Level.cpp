#include "Level.hpp"
#include <iostream>

Level::Level()
    : m_tilemap(std::make_unique<Tilemap>(64))  // Chaque tile fait 64x64 pixels à l'écran (256*0.25)
    , m_finishLine(sf::Vector2f(0, 0), sf::Vector2f(0, 0))
{
}

bool Level::load() {
    // Charger le tileset Mossy
    if (!m_tilemap->loadFromFile("assets/tiles/Mossy Tileset/Mossy - TileSet.png")) {
        return false;
    }

    // Créer un niveau simple
    createSimpleLevel();

    std::cout << "Level loaded successfully" << std::endl;
    return true;
}

void Level::createSimpleLevel() {
    // Créer un niveau très simple avec juste un sol continu et un mur à gauche
    // -1 = vide (air)
    // Tile 0 = angle supérieur gauche du sol
    // Tiles 1-4 = sol milieu (variantes)
    // Tile 5 = angle supérieur droit du sol
    // Tile 19 = mur gauche

    const int levelWidth = 20;   // 20 sections = 1280 pixels de large
    const int levelHeight = 12;  // 12 sections = 768 pixels de haut

    std::vector<std::vector<int>> levelData(levelHeight, std::vector<int>(levelWidth, -1));

    // Mur à gauche (colonne 0, lignes 0 à 10)
    for (int y = 0; y < levelHeight - 1; ++y) {
        levelData[y][0] = 19;  // Tile "mur gauche"
    }

    // Sol (dernière ligne uniquement) - sol continu sans trous
    levelData[levelHeight - 1][0] = 0;  // Angle gauche du sol
    for (int x = 1; x < levelWidth - 1; ++x) {
        levelData[levelHeight - 1][x] = 1 + (x % 4);  // Tiles 1, 2, 3, 4 en alternance
    }
    levelData[levelHeight - 1][levelWidth - 1] = 5;  // Angle droit

    // Le tileset fait 3584x3584, avec des sections de 256x256 = 14 sections par ligne
    m_tilemap->loadFromData(levelData, 14);

    // Ligne d'arrivée à la fin du niveau
    m_finishLine = sf::FloatRect(
        sf::Vector2f((levelWidth - 1) * 64.0f, 0),
        sf::Vector2f(64.0f, 64.0f * levelHeight)
    );
}

void Level::update(sf::Time deltaTime, Player& player) {
    handlePlayerCollision(player);
}

void Level::handlePlayerCollision(Player& player) {
    // Récupérer la position et la vélocité du joueur
    sf::Vector2f position = player.getPosition();
    sf::Vector2f velocity = player.getVelocity();

    // Taille approximative du joueur (en se basant sur le sprite à l'échelle 0.2)
    const float playerWidth = 102.0f;  // 512 * 0.2
    const float playerHeight = 102.0f;
    const float feetOffset = 25.0f;  // Les pieds sont à 25 pixels du bas du sprite

    // Calculer les tiles autour du joueur
    int tileSize = m_tilemap->getTileSize();  // 64

    int leftTile = static_cast<int>(position.x / tileSize);
    int rightTile = static_cast<int>((position.x + playerWidth) / tileSize);
    int topTile = static_cast<int>(position.y / tileSize);
    // Position des pieds réels du joueur
    float feetY = position.y + playerHeight - feetOffset;
    int bottomTile = static_cast<int>(feetY / tileSize);

    // Debug
    static int frameCount = 0;
    if (frameCount % 60 == 0) {
        std::cout << "Player Y: " << position.y << ", bottomTile: " << bottomTile
                  << ", tileSize: " << tileSize << std::endl;
    }
    frameCount++;

    // Vérifier collision horizontale avec les murs (en utilisant le centre du joueur)
    float playerCenterX = position.x + playerWidth / 2.0f;
    int centerTile = static_cast<int>(playerCenterX / tileSize);

    for (int y = topTile; y < bottomTile; ++y) {
        // Vérifier collision à gauche
        if (velocity.x < 0) {
            // Calculer la tile à gauche du centre
            int leftWallTile = static_cast<int>((playerCenterX - 1) / tileSize);
            if (m_tilemap->isSolid(leftWallTile, y)) {
                // Récupérer la vraie bounding box de collision de cette tuile
                sf::FloatRect wallBounds = m_tilemap->getTileCollisionBounds(leftWallTile, y);
                // Positionner le centre du joueur juste à droite du mur
                float targetCenterX = wallBounds.position.x + wallBounds.size.x + playerWidth / 2.0f;
                position.x = targetCenterX - playerWidth / 2.0f;
                velocity.x = 0.0f;
                player.setPosition(position);
                player.setVelocity(velocity);

                // Recalculer les tiles
                playerCenterX = position.x + playerWidth / 2.0f;
                centerTile = static_cast<int>(playerCenterX / tileSize);
                leftTile = static_cast<int>(position.x / tileSize);
                rightTile = static_cast<int>((position.x + playerWidth) / tileSize);
                break;
            }
        }
        // Vérifier collision à droite
        else if (velocity.x > 0) {
            // Calculer la tile à droite du centre
            int rightWallTile = static_cast<int>((playerCenterX + 1) / tileSize);
            if (m_tilemap->isSolid(rightWallTile, y)) {
                // Récupérer la vraie bounding box de collision de cette tuile
                sf::FloatRect wallBounds = m_tilemap->getTileCollisionBounds(rightWallTile, y);
                // Positionner le centre du joueur juste à gauche du mur
                float targetCenterX = wallBounds.position.x - playerWidth / 2.0f;
                position.x = targetCenterX - playerWidth / 2.0f;
                velocity.x = 0.0f;
                player.setPosition(position);
                player.setVelocity(velocity);

                // Recalculer les tiles
                playerCenterX = position.x + playerWidth / 2.0f;
                centerTile = static_cast<int>(playerCenterX / tileSize);
                leftTile = static_cast<int>(position.x / tileSize);
                rightTile = static_cast<int>((position.x + playerWidth) / tileSize);
                break;
            }
        }
    }

    // Vérifier collision avec le sol
    bool onGround = false;

    for (int x = leftTile; x <= rightTile; ++x) {
        // Vérifier si les pieds du joueur touchent une tile solide
        if (m_tilemap->isSolid(x, bottomTile)) {
            // Récupérer la profondeur d'herbe de la tile (en pourcentage)
            float grassDepthPercent = m_tilemap->getGrassDepth(x, bottomTile);
            float grassDepth = tileSize * (grassDepthPercent / 100.0f);
            float groundY = bottomTile * tileSize + grassDepth - (playerHeight - feetOffset);

            // Appliquer la collision uniquement si le joueur tombe (velocity.y >= 0)
            // et qu'il est au niveau du sol ou en dessous
            if (velocity.y >= 0) {
                // Le joueur tombe et touche le sol
                if (position.y >= groundY - 5.0f) {  // Tolérance de 5 pixels pour éviter les vibrations
                    onGround = true;
                    position.y = groundY;
                    velocity.y = 0.0f;
                    player.setPosition(position);
                    player.setVelocity(velocity);

                    if (frameCount % 60 == 0) {
                        std::cout << "  -> Collision! bottomTile=" << bottomTile
                                  << ", groundY=" << groundY << ", grassDepth=" << grassDepth
                                  << " (" << grassDepthPercent << "%)" << std::endl;
                    }
                }
            }
            break;
        }
    }

    // Mettre à jour l'état du joueur
    player.setGrounded(onGround);
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
