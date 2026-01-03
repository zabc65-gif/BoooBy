#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "TileProperties.hpp"

class Tilemap {
public:
    Tilemap(int tileSize = 32);

    bool loadFromFile(const std::string& tilesetPath);
    void loadFromData(const std::vector<std::vector<int>>& data, int tilesetWidth);

    void render(sf::RenderWindow& window);

    // Collision detection
    bool isSolid(int x, int y) const;
    sf::FloatRect getTileBounds(int x, int y) const;
    sf::FloatRect getTileCollisionBounds(int x, int y) const;  // Bounds ajustés avec collisionBox

    // Tile properties access
    int getTileId(int x, int y) const;
    const TileProperties* getTileProperties(int x, int y) const;
    CollisionType getCollisionType(int x, int y) const;
    float getGrassDepth(int x, int y) const;

    int getTileSize() const { return m_tileSize; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    // Debug constants
    static constexpr bool SHOW_DEBUG_TILE_OUTLINE = false; // Mettre à true pour afficher les contours rouges des tuiles

    int m_tileSize;
    int m_width;
    int m_height;
    int m_tilesetWidthInTiles;  // Nombre de tiles par ligne dans le tileset

    std::shared_ptr<sf::Texture> m_tileset;
    std::vector<std::vector<int>> m_tiles;
    sf::VertexArray m_vertices;  // Utiliser VertexArray au lieu de vector<Vertex>

    void updateVertices();
};
