#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

class Tilemap {
public:
    Tilemap(int tileSize = 32);

    bool loadFromFile(const std::string& tilesetPath);
    void loadFromData(const std::vector<std::vector<int>>& data, int tilesetWidth);

    void render(sf::RenderWindow& window);

    // Collision detection
    bool isSolid(int x, int y) const;
    sf::FloatRect getTileBounds(int x, int y) const;

    int getTileSize() const { return m_tileSize; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    int m_tileSize;
    int m_width;
    int m_height;

    std::shared_ptr<sf::Texture> m_tileset;
    std::vector<std::vector<int>> m_tiles;
    std::vector<sf::Vertex> m_vertices;

    void updateVertices();
};
