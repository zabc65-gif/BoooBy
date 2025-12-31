#pragma once

#include <string>
#include <map>
#include <memory>

enum class CollisionType {
    NONE,
    SOLID,
    EMPTY,
    PLATFORM,           // Collision par dessus uniquement
    HALF_BLOCK,         // Collision partielle
    SLOPE_LEFT,         // Pente gauche
    SLOPE_RIGHT,        // Pente droite
    LADDER,             // Grimpable
    SPIKES,             // Dégâts
    WATER,              // Ralentissement
    ICE,                // Glissement
    WALL_LEFT,          // Bloque à gauche
    WALL_RIGHT,         // Bloque à droite
    CEILING,            // Bloque par le haut
    PORTAL,             // Téléportation
    GROUND_CORNER_LEFT,       // Angle de sol supérieur gauche
    GROUND_CORNER_RIGHT,      // Angle de sol supérieur droit
    GROUND_UNDER_CORNER_LEFT, // Angle de dessous de sol gauche
    GROUND_UNDER_CORNER_RIGHT,// Angle de dessous de sol droit
    WALL_CORNER_TOP_LEFT,     // Angle de mur supérieur gauche
    WALL_CORNER_TOP_RIGHT,    // Angle de mur supérieur droit
    WALL_CORNER_BOTTOM_LEFT,  // Angle de mur inférieur gauche
    WALL_CORNER_BOTTOM_RIGHT  // Angle de mur inférieur droit
};

struct CollisionBox {
    float left;   // Offset from left edge in pixels
    float top;    // Offset from top edge in pixels
    float right;  // Offset from right edge in pixels
    float bottom; // Offset from bottom edge in pixels

    CollisionBox() : left(0), top(0), right(0), bottom(0) {}
};

struct TileProperties {
    std::string description;
    CollisionType collisionType;
    float grassDepth;  // Pourcentage (0-100)
    CollisionBox collisionBox; // Offsets pour réduire la zone de collision
    int damage;        // Pour les spikes
    std::string portalDestination; // Pour les portails (format "x,y")
    std::string notes;

    TileProperties()
        : collisionType(CollisionType::NONE)
        , grassDepth(0.0f)
        , damage(0)
    {}
};

class TilePropertiesManager {
public:
    static TilePropertiesManager& getInstance();

    bool loadFromFile(const std::string& filepath);
    const TileProperties* getTileProperties(int tileId) const;

    // Helpers pour les collisions
    bool isSolid(int tileId) const;
    bool isPlatform(int tileId) const;
    bool isSlope(int tileId) const;
    CollisionType getCollisionType(int tileId) const;
    float getGrassDepth(int tileId) const;
    CollisionBox getCollisionBox(int tileId) const;

private:
    TilePropertiesManager() = default;
    ~TilePropertiesManager() = default;
    TilePropertiesManager(const TilePropertiesManager&) = delete;
    TilePropertiesManager& operator=(const TilePropertiesManager&) = delete;

    CollisionType stringToCollisionType(const std::string& str) const;

    std::map<int, TileProperties> m_properties;
};
