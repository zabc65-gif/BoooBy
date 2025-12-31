#include "Tilemap.hpp"
#include <iostream>

Tilemap::Tilemap(int tileSize)
    : m_tileSize(tileSize)
    , m_width(0)
    , m_height(0)
    , m_tilesetWidthInTiles(0)
    , m_vertices(sf::PrimitiveType::Triangles)  // Utiliser Triangles
{
}

bool Tilemap::loadFromFile(const std::string& tilesetPath) {
    m_tileset = std::make_shared<sf::Texture>();
    if (!m_tileset->loadFromFile(tilesetPath)) {
        std::cerr << "Failed to load tileset: " << tilesetPath << std::endl;
        return false;
    }

    // Désactiver le lissage pour un rendu pixel-perfect
    m_tileset->setSmooth(false);
    // S'assurer que la texture est répétée
    m_tileset->setRepeated(false);

    std::cout << "Tileset loaded: " << tilesetPath << " ("
              << m_tileset->getSize().x << "x" << m_tileset->getSize().y << ")" << std::endl;

    // Load tile properties configuration
    TilePropertiesManager::getInstance().loadFromFile("assets/tiles/mossy_tileset_config.json");

    return true;
}

void Tilemap::loadFromData(const std::vector<std::vector<int>>& data, int tilesetWidth) {
    m_tiles = data;
    m_height = data.size();
    m_width = data.empty() ? 0 : data[0].size();
    m_tilesetWidthInTiles = tilesetWidth;

    std::cout << "Loading tilemap data: " << m_width << "x" << m_height << std::endl;
    std::cout << "Tileset width in tiles: " << m_tilesetWidthInTiles << std::endl;
    updateVertices();
    std::cout << "Generated " << m_vertices.getVertexCount() << " vertices" << std::endl;
}

void Tilemap::updateVertices() {
    m_vertices.clear();

    int tileCount = 0;
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int tileNumber = m_tiles[y][x];

            // -1 = pas de tile
            if (tileNumber < 0) continue;

            // Calculer la position dans le tileset
            int tu = tileNumber % m_tilesetWidthInTiles;
            int tv = tileNumber / m_tilesetWidthInTiles;

            // Debug pour les premières tiles
            if (tileCount < 3) {
                std::cout << "  Tile #" << tileNumber << " at pos(" << x << "," << y << ") -> texture grid(" << tu << "," << tv << ")"
                          << " -> texture pixels(" << (tu * m_tileSize) << "," << (tv * m_tileSize) << ")" << std::endl;
            }
            tileCount++;

            // Positions des 4 coins du quad
            float px = x * m_tileSize;
            float py = y * m_tileSize;

            // Coordonnées de texture
            float tx = tu * m_tileSize;
            float ty = tv * m_tileSize;

            // Deux triangles pour former un quad
            sf::Vertex v0, v1, v2, v3, v4, v5;

            // Triangle 1
            v0.position = sf::Vector2f(px, py);
            v0.texCoords = sf::Vector2f(tx, ty);
            v0.color = sf::Color::White;  // Important: couleur blanche pour afficher la texture correctement

            v1.position = sf::Vector2f(px + m_tileSize, py);
            v1.texCoords = sf::Vector2f(tx + m_tileSize, ty);
            v1.color = sf::Color::White;

            v2.position = sf::Vector2f(px, py + m_tileSize);
            v2.texCoords = sf::Vector2f(tx, ty + m_tileSize);
            v2.color = sf::Color::White;

            // Triangle 2
            v3.position = sf::Vector2f(px, py + m_tileSize);
            v3.texCoords = sf::Vector2f(tx, ty + m_tileSize);
            v3.color = sf::Color::White;

            v4.position = sf::Vector2f(px + m_tileSize, py);
            v4.texCoords = sf::Vector2f(tx + m_tileSize, ty);
            v4.color = sf::Color::White;

            v5.position = sf::Vector2f(px + m_tileSize, py + m_tileSize);
            v5.texCoords = sf::Vector2f(tx + m_tileSize, ty + m_tileSize);
            v5.color = sf::Color::White;

            m_vertices.append(v0);
            m_vertices.append(v1);
            m_vertices.append(v2);
            m_vertices.append(v3);
            m_vertices.append(v4);
            m_vertices.append(v5);
        }
    }
}

void Tilemap::render(sf::RenderWindow& window) {
    if (!m_tileset) {
        return;
    }

    // Afficher le niveau avec des sections de 256x256 pixels du tileset
    // mais affichées à une échelle réduite pour correspondre au personnage
    const int sectionSize = 256;
    const int sectionsPerRow = 14;
    const float scale = 0.25f;  // Afficher les tiles à 64x64 pixels au lieu de 256x256
    const float displaySize = sectionSize * scale;  // 64 pixels

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int tileNumber = m_tiles[y][x];

            // -1 = pas de tile
            if (tileNumber < 0) continue;

            // Calculer quelle section du tileset afficher
            int sectionX = tileNumber % sectionsPerRow;
            int sectionY = tileNumber / sectionsPerRow;

            sf::Sprite tileSprite(*m_tileset);
            tileSprite.setTextureRect(sf::IntRect(
                sf::Vector2i(sectionX * sectionSize, sectionY * sectionSize),
                sf::Vector2i(sectionSize, sectionSize)
            ));

            // Positionner et afficher avec un scale de 0.5 (128x128 à l'écran)
            tileSprite.setPosition(sf::Vector2f(x * displaySize, y * displaySize));
            tileSprite.setScale(sf::Vector2f(scale, scale));
            window.draw(tileSprite);

            // Dessiner un cadre rouge autour de chaque tile pour debug
            sf::RectangleShape tileDebugRect(sf::Vector2f(displaySize, displaySize));
            tileDebugRect.setPosition(sf::Vector2f(x * displaySize, y * displaySize));
            tileDebugRect.setFillColor(sf::Color::Transparent);
            tileDebugRect.setOutlineColor(sf::Color::Red);
            tileDebugRect.setOutlineThickness(1.0f);
            window.draw(tileDebugRect);
        }
    }
}

bool Tilemap::isSolid(int x, int y) const {
    // Hors limites = pas de collision (le joueur tombe dans le vide)
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return false;
    }

    int tileId = m_tiles[y][x];
    if (tileId < 0) return false; // -1 = vide

    // Use tile properties manager to check if solid
    return TilePropertiesManager::getInstance().isSolid(tileId);
}

sf::FloatRect Tilemap::getTileBounds(int x, int y) const {
    return sf::FloatRect(
        sf::Vector2f(x * m_tileSize, y * m_tileSize),
        sf::Vector2f(m_tileSize, m_tileSize)
    );
}

sf::FloatRect Tilemap::getTileCollisionBounds(int x, int y) const {
    int tileId = getTileId(x, y);
    if (tileId < 0) {
        return getTileBounds(x, y);
    }

    // Récupérer la collision box de cette tuile
    CollisionBox box = TilePropertiesManager::getInstance().getCollisionBox(tileId);

    // Appliquer les offsets à la bounding box normale
    float left = x * m_tileSize + box.left;
    float top = y * m_tileSize + box.top;
    float width = m_tileSize - box.left - box.right;
    float height = m_tileSize - box.top - box.bottom;

    return sf::FloatRect(sf::Vector2f(left, top), sf::Vector2f(width, height));
}

int Tilemap::getTileId(int x, int y) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return -1;
    }
    return m_tiles[y][x];
}

const TileProperties* Tilemap::getTileProperties(int x, int y) const {
    int tileId = getTileId(x, y);
    if (tileId < 0) return nullptr;

    return TilePropertiesManager::getInstance().getTileProperties(tileId);
}

CollisionType Tilemap::getCollisionType(int x, int y) const {
    int tileId = getTileId(x, y);
    if (tileId < 0) return CollisionType::NONE;

    return TilePropertiesManager::getInstance().getCollisionType(tileId);
}

float Tilemap::getGrassDepth(int x, int y) const {
    int tileId = getTileId(x, y);
    if (tileId < 0) return 0.0f;

    return TilePropertiesManager::getInstance().getGrassDepth(tileId);
}
