#pragma once

#include <SFML/Graphics.hpp>
#include "Tilemap.hpp"
#include "Player.hpp"
#include "Enemy.hpp"
#include <memory>
#include <vector>

class Level {
public:
    // Particule ambiante flottante (poussière, spore, pollen)
    struct AmbientParticle {
        sf::Vector2f position;
        sf::Vector2f basePosition;  // Position de base pour l'oscillation
        float size;
        float alpha;
        float oscillationPhase;     // Phase pour le mouvement sinusoïdal
        float oscillationSpeed;     // Vitesse d'oscillation
        float oscillationAmplitude; // Amplitude du mouvement
        float driftSpeed;           // Vitesse de dérive lente
        sf::Color color;
    };

    // Rayon de lumière subtil
    struct LightRay {
        sf::Vector2f position;
        float width;
        float height;
        float alpha;
        float flickerPhase;
        float flickerSpeed;
        float angle;  // Angle du rayon (en degrés)
    };

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

    // Accès aux dimensions du niveau
    int getWidth() const { return m_tilemap->getWidth(); }
    int getHeight() const { return m_tilemap->getHeight(); }
    int getTileSize() const { return m_tilemap->getTileSize(); }

    // Gestion des ennemis
    void addEnemy(const sf::Vector2f& position, float scale = 1.0f);
    const std::vector<std::unique_ptr<Enemy>>& getEnemies() const { return m_enemies; }
    void generateEnemies(int levelNumber);

    // Décor ambiant
    void generateAmbientParticles();
    void updateAmbientEffects(sf::Time deltaTime);
    void renderAmbientBackground(sf::RenderWindow& window);  // Derrière les tiles
    void renderAmbientForeground(sf::RenderWindow& window);  // Devant les tiles

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

    // Ennemis
    std::vector<std::unique_ptr<Enemy>> m_enemies;

    // Décor ambiant
    std::vector<AmbientParticle> m_ambientParticles;
    std::vector<LightRay> m_lightRays;
    float m_ambientTimer;
};
