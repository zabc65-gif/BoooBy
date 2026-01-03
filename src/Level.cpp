#include "Level.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>

Level::Level()
    : m_tilemap(std::make_unique<Tilemap>(64))  // Chaque tile fait 64x64 pixels à l'écran (256*0.25)
    , m_finishLine(sf::Vector2f(0, 0), sf::Vector2f(0, 0))
    , m_entrancePortalPosition(0.0f, 0.0f)
    , m_hasEntrancePortal(false)
    , m_exitPortalPosition(0.0f, 0.0f)
    , m_hasExitPortal(false)
    , m_doorTextureLoaded(false)
    , m_isPrologueLevel(false)
{
    // Charger la texture de la porte médiévale (taille moyenne - 1.5x hauteur joueur)
    if (m_doorTexture.loadFromFile("assets/tiles/Medieval_door_medium.png")) {
        m_doorTextureLoaded = true;
        m_entranceDoorSprite = std::make_unique<sf::Sprite>(m_doorTexture);
        m_exitDoorSprite = std::make_unique<sf::Sprite>(m_doorTexture);
        std::cout << "Medieval door texture loaded successfully" << std::endl;
    } else {
        std::cerr << "Failed to load medieval door texture" << std::endl;
    }
}

bool Level::load() {
    // Charger le tileset Mossy
    if (!m_tilemap->loadFromFile("assets/tiles/Mossy Tileset/Mossy - TileSet.png")) {
        return false;
    }

    // Charger le niveau prologue
    return loadFromFile("levels/prologue.json");
}

bool Level::loadFromFile(const std::string& filepath) {
    std::cout << "Loading level from: " << filepath << std::endl;

    // Détecter si c'est le niveau prologue
    m_isPrologueLevel = (filepath.find("prologue") != std::string::npos);

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open level file: " << filepath << std::endl;
        return false;
    }

    // Lecture simple du JSON ligne par ligne
    std::string line;
    std::vector<std::vector<int>> levelData;
    int width = 0, height = 0;
    bool inTiles = false;
    bool inRow = false;

    while (std::getline(file, line)) {
        // Enlever les espaces
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

        if (line.find("\"width\":") != std::string::npos) {
            size_t pos = line.find(":");
            width = std::stoi(line.substr(pos + 1, line.find(",") - pos - 1));
            std::cout << "Parsed width: " << width << std::endl;
        }
        else if (line.find("\"height\":") != std::string::npos) {
            size_t pos = line.find(":");
            height = std::stoi(line.substr(pos + 1, line.find(",") - pos - 1));
            std::cout << "Parsed height: " << height << std::endl;
        }
        else if (line.find("\"tiles\":") != std::string::npos) {
            inTiles = true;
            std::cout << "Found tiles array" << std::endl;
        }
        else if (inTiles && line.find("[") == 0) {
            // C'est une ligne de tiles (peut avoir une virgule à la fin)
            std::vector<int> row;

            // Enlever le [ au début
            std::string content = line.substr(1);

            // Enlever le ] et possiblement une virgule à la fin
            size_t endPos = content.find_last_of("]");
            if (endPos != std::string::npos) {
                content = content.substr(0, endPos);
            }

            if (!content.empty()) {
                std::stringstream ss(content);
                std::string token;
                while (std::getline(ss, token, ',')) {
                    row.push_back(std::stoi(token));
                }
                levelData.push_back(row);
                std::cout << "Parsed row " << levelData.size() << " with " << row.size() << " tiles" << std::endl;
            }
        }
        else if (line.find("],") == 0 || line.find("]") == 0) {
            inTiles = false;
            std::cout << "End of tiles array, parsed " << levelData.size() << " rows" << std::endl;
        }
        else if (line.find("\"entrancePortal\":{") != std::string::npos) {
            // Parser le portail d'entrée
            size_t xPos = line.find("\"x\":");
            size_t yPos = line.find("\"y\":");
            if (xPos != std::string::npos && yPos != std::string::npos) {
                int x = std::stoi(line.substr(xPos + 4, line.find(",", xPos) - xPos - 4));
                int y = std::stoi(line.substr(yPos + 4, line.find("}", yPos) - yPos - 4));
                m_entrancePortalPosition = sf::Vector2f(x * 64.0f, y * 64.0f);
                m_hasEntrancePortal = true;

                // Positionner le sprite de la porte d'entrée
                if (m_doorTextureLoaded && m_entranceDoorSprite) {
                    // Positionner la porte pour qu'elle soit sur le sol
                    // La porte fait 156 pixels de haut (taille moyenne), on la place au-dessus du portail
                    float doorX = x * 64.0f + 32.0f - 39.0f;  // Centrer horizontalement (78/2 = 39 pixels)
                    float doorY = y * 64.0f - 156.0f + 64.0f;  // Placer au-dessus du sol (hauteur 156)
                    m_entranceDoorSprite->setPosition(sf::Vector2f(doorX, doorY));
                }

                std::cout << "Entrance portal found at: (" << x << ", " << y << ")" << std::endl;
            }
        }
        else if (line.find("\"exitPortal\":{") != std::string::npos) {
            // Parser le portail de sortie
            size_t xPos = line.find("\"x\":");
            size_t yPos = line.find("\"y\":");
            if (xPos != std::string::npos && yPos != std::string::npos) {
                int x = std::stoi(line.substr(xPos + 4, line.find(",", xPos) - xPos - 4));
                int y = std::stoi(line.substr(yPos + 4, line.find("}", yPos) - yPos - 4));
                m_exitPortalPosition = sf::Vector2f(x * 64.0f, y * 64.0f);
                m_hasExitPortal = true;

                // Positionner le sprite de la porte de sortie
                if (m_doorTextureLoaded && m_exitDoorSprite) {
                    // Positionner la porte pour qu'elle soit sur le sol
                    // La porte fait 156 pixels de haut (taille moyenne), on la place au-dessus du portail
                    float doorX = x * 64.0f + 32.0f - 39.0f;  // Centrer horizontalement (78/2 = 39 pixels)
                    float doorY = y * 64.0f - 156.0f + 64.0f;  // Placer au-dessus du sol (hauteur 156)
                    m_exitDoorSprite->setPosition(sf::Vector2f(doorX, doorY));
                }

                std::cout << "Exit portal found at: (" << x << ", " << y << ")" << std::endl;
            }
        }
    }

    file.close();

    if (levelData.empty()) {
        std::cerr << "No level data found in file" << std::endl;
        return false;
    }

    // Charger les données dans la tilemap
    m_tilemap->loadFromData(levelData, 14);  // 14 tiles par ligne dans le tileset

    // Ligne d'arrivée à la fin du niveau
    m_finishLine = sf::FloatRect(
        sf::Vector2f((width - 1) * 64.0f, 0),
        sf::Vector2f(64.0f, 64.0f * height)
    );

    std::cout << "Level loaded successfully: " << width << "x" << height << std::endl;
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
    levelData[levelHeight - 1][0] = 1;  // Sol classique à gauche
    for (int x = 1; x < levelWidth - 1; ++x) {
        levelData[levelHeight - 1][x] = 1 + (x % 4);  // Tiles 1, 2, 3, 4 en alternance
    }
    levelData[levelHeight - 1][levelWidth - 1] = 5;  // Angle droit

    // Mur droit test devant le personnage (colonne 5, posé sur le sol)
    levelData[levelHeight - 2][5] = 14;  // Tile "mur droit" (tile #14)

    // Tuile de plafond au-dessus du personnage (colonne 2-3, ligne 8)
    levelData[8][2] = 71;  // Tile "dessous de sol" (ceiling)

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
    bool touchingWall = false;

    for (int y = topTile; y < bottomTile; ++y) {
        int centerTile = static_cast<int>(playerCenterX / tileSize);

        // Vérifier si le joueur est adjacent à un mur (indépendamment de la vélocité)
        if (m_tilemap->isSolid(centerTile, y)) {
            // Récupérer la vraie bounding box de collision de cette tuile
            sf::FloatRect wallBounds = m_tilemap->getTileCollisionBounds(centerTile, y);
            float wallLeft = wallBounds.position.x;
            float wallRight = wallBounds.position.x + wallBounds.size.x;

            // Vérifier si le centre du joueur est dans le mur
            if (playerCenterX >= wallLeft && playerCenterX <= wallRight) {
                touchingWall = true;

                // Appliquer la collision seulement si le joueur se déplace vers le mur
                if (velocity.x < 0 && playerCenterX < wallRight) {
                    // Collision à gauche
                    position.x = wallRight - playerWidth / 2.0f;
                    velocity.x = 0.0f;
                    player.setPosition(position);
                    player.setVelocity(velocity);

                    // Recalculer les tiles
                    playerCenterX = position.x + playerWidth / 2.0f;
                    leftTile = static_cast<int>(position.x / tileSize);
                    rightTile = static_cast<int>((position.x + playerWidth) / tileSize);
                } else if (velocity.x > 0 && playerCenterX > wallLeft) {
                    // Collision à droite
                    position.x = wallLeft - playerWidth / 2.0f;
                    velocity.x = 0.0f;
                    player.setPosition(position);
                    player.setVelocity(velocity);

                    // Recalculer les tiles
                    playerCenterX = position.x + playerWidth / 2.0f;
                    leftTile = static_cast<int>(position.x / tileSize);
                    rightTile = static_cast<int>((position.x + playerWidth) / tileSize);
                }
                break;
            }
        }
    }

    // Vérifier collision avec le plafond (uniquement quand le joueur monte)
    if (velocity.y < 0) {
        for (int x = leftTile; x <= rightTile; ++x) {
            if (m_tilemap->isSolid(x, topTile)) {
                CollisionType tileType = m_tilemap->getCollisionType(x, topTile);

                // Vérifier si c'est un plafond
                if (tileType == CollisionType::CEILING) {
                    sf::FloatRect ceilingBounds = m_tilemap->getTileCollisionBounds(x, topTile);
                    float ceilingBottom = ceilingBounds.position.y + ceilingBounds.size.y;

                    // Vérifier si le haut du joueur est proche du plafond
                    if (position.y <= ceilingBottom + 2.0f) {
                        // Arrêter le mouvement vertical uniquement
                        velocity.y = 0.0f;
                        player.setVelocity(velocity);

                        if (frameCount % 60 == 0) {
                            std::cout << "  -> Ceiling collision! topTile=" << topTile
                                      << ", ceilingBottom=" << ceilingBottom << std::endl;
                        }
                        break;
                    }
                }
            }
        }
    }

    // Vérifier collision avec le sol
    bool onGround = false;
    bool hasGroundTile = false;

    // Vérifier d'abord s'il y a une tuile de sol (pas un mur) sous le joueur
    for (int x = leftTile; x <= rightTile; ++x) {
        if (m_tilemap->isSolid(x, bottomTile)) {
            CollisionType tileType = m_tilemap->getCollisionType(x, bottomTile);
            // Si c'est une tuile de sol (pas un mur), noter qu'on a un sol
            if (tileType != CollisionType::WALL_LEFT && tileType != CollisionType::WALL_RIGHT) {
                hasGroundTile = true;
                break;
            }
        }
    }

    // Appliquer la collision avec le sol seulement si:
    // 1. Il n'y a pas de mur OU il y a un sol sous le joueur (priorité au sol)
    if (!touchingWall || hasGroundTile) {
        for (int x = leftTile; x <= rightTile; ++x) {
            // Vérifier si les pieds du joueur touchent une tile solide
            if (m_tilemap->isSolid(x, bottomTile)) {
                // Vérifier le type de collision de la tuile
                CollisionType tileType = m_tilemap->getCollisionType(x, bottomTile);

                // Ignorer la collision si c'est un mur (wall-left ou wall-right)
                if (tileType == CollisionType::WALL_LEFT || tileType == CollisionType::WALL_RIGHT) {
                    continue;  // Passer à la prochaine tuile
                }

                // Récupérer la profondeur d'herbe de la tile (en pourcentage)
                float grassDepthPercent = m_tilemap->getGrassDepth(x, bottomTile);
                float grassDepth = tileSize * (grassDepthPercent / 100.0f);
                float groundY = bottomTile * tileSize + grassDepth - (playerHeight - feetOffset);

                // Appliquer la collision uniquement si le joueur tombe (velocity.y >= 0)
                // et qu'il est au niveau du sol ou en dessous (proche du sol)
                if (velocity.y >= 0 && position.y >= groundY - 2.0f) {
                    onGround = true;
                    // Seulement ajuster la position si on est vraiment en dessous du sol
                    if (position.y > groundY) {
                        position.y = groundY;
                    }
                    velocity.y = 0.0f;
                    player.setPosition(position);
                    player.setVelocity(velocity);

                    if (frameCount % 60 == 0) {
                        std::cout << "  -> Collision! bottomTile=" << bottomTile
                                  << ", groundY=" << groundY << ", grassDepth=" << grassDepth
                                  << " (" << grassDepthPercent << "%)" << std::endl;
                    }
                }
                break;
            }
        }
    }

    // Mettre à jour l'état du joueur
    player.setGrounded(onGround);
}

bool Level::isPlayerAtFinish(const Player& player) const {
    // Vérifier si le joueur est au centre horizontal du portail de sortie (toute hauteur)
    if (!m_hasExitPortal) {
        return false;
    }

    sf::Vector2f playerPos = player.getPosition();
    const float playerWidth = 102.0f;

    // Calculer le centre horizontal du joueur
    float playerCenterX = playerPos.x + playerWidth / 2.0f;

    // Calculer le centre horizontal du portail de sortie
    float portalCenterX = m_exitPortalPosition.x + 32.0f;

    // Calculer la distance horizontale uniquement
    float dx = std::abs(playerCenterX - portalCenterX);

    // Le joueur doit être très proche du centre horizontal (20 pixels de tolérance)
    return dx < 20.0f;
}

bool Level::isLevelValid(int levelNumber) {
    std::string filename = "levels/level_" + std::to_string(levelNumber) + ".json";

    std::cout << "  Checking if level is valid: " << filename << std::endl;

    // Vérifier si le fichier existe
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "  ERROR: Cannot open file " << filename << std::endl;
        return false;
    }

    std::cout << "  File opened successfully" << std::endl;

    // Parser le JSON pour vérifier les portails
    std::string line;
    bool hasEntrancePortal = false;
    bool hasExitPortal = false;

    while (std::getline(file, line)) {
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

        if (line.find("\"entrancePortal\":{") != std::string::npos) {
            hasEntrancePortal = true;
            std::cout << "  Found entrance portal" << std::endl;
        }
        else if (line.find("\"exitPortal\":{") != std::string::npos) {
            hasExitPortal = true;
            std::cout << "  Found exit portal" << std::endl;
        }
    }

    file.close();

    std::cout << "  Entrance portal: " << (hasEntrancePortal ? "YES" : "NO") << std::endl;
    std::cout << "  Exit portal: " << (hasExitPortal ? "YES" : "NO") << std::endl;

    // Le niveau est valide s'il a les deux portails
    bool isValid = hasEntrancePortal && hasExitPortal;
    std::cout << "  Level is valid: " << (isValid ? "YES" : "NO") << std::endl;
    return isValid;
}

void Level::render(sf::RenderWindow& window) {
    // Dessiner la tilemap
    m_tilemap->render(window);

    // Dessiner le portail d'entrée
    if (m_hasEntrancePortal) {
        if (m_isPrologueLevel) {
            // Niveau prologue : afficher la porte médiévale
            if (m_doorTextureLoaded && m_entranceDoorSprite) {
                window.draw(*m_entranceDoorSprite);
            }
        } else {
            // Autres niveaux : afficher un portail d'entrée (vert)
            float centerX = m_entrancePortalPosition.x + 32.0f;
            float centerY = m_entrancePortalPosition.y + 32.0f;
            const float tileSize = 64.0f;

            // Cercle extérieur (lueur verte)
            sf::CircleShape outerGlow(tileSize * 0.6f);
            outerGlow.setOrigin(sf::Vector2f(tileSize * 0.6f, tileSize * 0.6f));
            outerGlow.setPosition(sf::Vector2f(centerX, centerY));
            outerGlow.setFillColor(sf::Color(0, 255, 0, 50));
            outerGlow.setOutlineColor(sf::Color(0, 255, 0, 150));
            outerGlow.setOutlineThickness(3.0f);
            window.draw(outerGlow);

            // Cercle du milieu
            sf::CircleShape middleRing(tileSize * 0.4f);
            middleRing.setOrigin(sf::Vector2f(tileSize * 0.4f, tileSize * 0.4f));
            middleRing.setPosition(sf::Vector2f(centerX, centerY));
            middleRing.setFillColor(sf::Color(0, 200, 0, 100));
            middleRing.setOutlineColor(sf::Color(100, 255, 100, 200));
            middleRing.setOutlineThickness(2.0f);
            window.draw(middleRing);

            // Point central brillant
            sf::CircleShape innerCore(tileSize * 0.2f);
            innerCore.setOrigin(sf::Vector2f(tileSize * 0.2f, tileSize * 0.2f));
            innerCore.setPosition(sf::Vector2f(centerX, centerY));
            innerCore.setFillColor(sf::Color(255, 255, 255, 200));
            window.draw(innerCore);
        }
    }

    // Dessiner le portail de sortie (même style que dans l'éditeur - cyan)
    if (m_hasExitPortal) {
        float centerX = m_exitPortalPosition.x + 32.0f;
        float centerY = m_exitPortalPosition.y + 32.0f;
        const float tileSize = 64.0f;

        // Cercle extérieur (lueur cyan)
        sf::CircleShape outerGlow(tileSize * 0.6f);
        outerGlow.setOrigin(sf::Vector2f(tileSize * 0.6f, tileSize * 0.6f));
        outerGlow.setPosition(sf::Vector2f(centerX, centerY));
        outerGlow.setFillColor(sf::Color(0, 255, 255, 50));
        outerGlow.setOutlineColor(sf::Color(0, 255, 255, 150));
        outerGlow.setOutlineThickness(3.0f);
        window.draw(outerGlow);

        // Cercle du milieu
        sf::CircleShape middleRing(tileSize * 0.4f);
        middleRing.setOrigin(sf::Vector2f(tileSize * 0.4f, tileSize * 0.4f));
        middleRing.setPosition(sf::Vector2f(centerX, centerY));
        middleRing.setFillColor(sf::Color(0, 150, 255, 100));
        middleRing.setOutlineColor(sf::Color(100, 200, 255, 200));
        middleRing.setOutlineThickness(2.0f);
        window.draw(middleRing);

        // Point central brillant
        sf::CircleShape innerCore(tileSize * 0.2f);
        innerCore.setOrigin(sf::Vector2f(tileSize * 0.2f, tileSize * 0.2f));
        innerCore.setPosition(sf::Vector2f(centerX, centerY));
        innerCore.setFillColor(sf::Color(255, 255, 255, 200));
        window.draw(innerCore);
    }
}
