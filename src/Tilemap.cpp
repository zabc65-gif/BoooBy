#include "Tilemap.hpp"
#include <iostream>

Tilemap::Tilemap(int tileSize)
    : m_tileSize(tileSize)
    , m_width(0)
    , m_height(0)
{
}

bool Tilemap::loadFromFile(const std::string& tilesetPath) {
    m_tileset = std::make_shared<sf::Texture>();
    if (!m_tileset->loadFromFile(tilesetPath)) {
        std::cerr << "Failed to load tileset: " << tilesetPath << std::endl;
        return false;
    }

    std::cout << "Tileset loaded: " << tilesetPath << std::endl;
    return true;
}

void Tilemap::loadFromData(const std::vector<std::vector<int>>& data, int tilesetWidth) {
    m_tiles = data;
    m_height = data.size();
    m_width = data.empty() ? 0 : data[0].size();

    std::cout << "Loading tilemap data: " << m_width << "x" << m_height << std::endl;
    updateVertices();
    std::cout << "Generated " << m_vertices.size() << " vertices" << std::endl;
}

void Tilemap::updateVertices() {
    m_vertices.clear();

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int tileNumber = m_tiles[y][x];

            // -1 = pas de tile
            if (tileNumber < 0) continue;

            // Calculer la position dans le tileset
            int tu = tileNumber % 16; // 16 tiles par ligne (tileset 512x512 avec tiles 32x32)
            int tv = tileNumber / 16;

            // Positions des 4 coins du quad
            float px = x * m_tileSize;
            float py = y * m_tileSize;

            // Coordonnées de texture
            float tx = tu * m_tileSize;
            float ty = tv * m_tileSize;

            // Triangle 1
            sf::Vertex v0, v1, v2, v3, v4, v5;
            v0.position = sf::Vector2f(px, py);
            v0.texCoords = sf::Vector2f(tx, ty);
            v1.position = sf::Vector2f(px + m_tileSize, py);
            v1.texCoords = sf::Vector2f(tx + m_tileSize, ty);
            v2.position = sf::Vector2f(px, py + m_tileSize);
            v2.texCoords = sf::Vector2f(tx, ty + m_tileSize);

            // Triangle 2
            v3.position = sf::Vector2f(px, py + m_tileSize);
            v3.texCoords = sf::Vector2f(tx, ty + m_tileSize);
            v4.position = sf::Vector2f(px + m_tileSize, py);
            v4.texCoords = sf::Vector2f(tx + m_tileSize, ty);
            v5.position = sf::Vector2f(px + m_tileSize, py + m_tileSize);
            v5.texCoords = sf::Vector2f(tx + m_tileSize, ty + m_tileSize);

            m_vertices.push_back(v0);
            m_vertices.push_back(v1);
            m_vertices.push_back(v2);
            m_vertices.push_back(v3);
            m_vertices.push_back(v4);
            m_vertices.push_back(v5);
        }
    }
}

void Tilemap::render(sf::RenderWindow& window) {
    if (m_vertices.empty() || !m_tileset) return;

    sf::RenderStates states;
    states.texture = m_tileset.get();
    window.draw(m_vertices.data(), m_vertices.size(), sf::PrimitiveType::Triangles, states);
}

bool Tilemap::isSolid(int x, int y) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return true; // Hors limites = solide
    }

    int tile = m_tiles[y][x];
    return tile >= 0; // Toute tile >= 0 est solide, -1 = vide
}

sf::FloatRect Tilemap::getTileBounds(int x, int y) const {
    return sf::FloatRect(
        sf::Vector2f(x * m_tileSize, y * m_tileSize),
        sf::Vector2f(m_tileSize, m_tileSize)
    );
}
