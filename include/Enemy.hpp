#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class Player;

class Enemy {
public:
    Enemy(const sf::Vector2f& position, float scale = 1.0f);

    void update(sf::Time deltaTime, const Player& player);
    void render(sf::RenderWindow& window);

    float getScale() const { return m_scale; }

    // Collision avec le joueur
    bool checkCollision(const Player& player) const;
    sf::FloatRect getBounds() const;
    bool canDealDamage() const { return m_damageTimer <= 0.0f; }
    void resetDamageCooldown() { m_damageTimer = DAMAGE_COOLDOWN; }

    // Getters
    sf::Vector2f getPosition() const { return m_position; }
    bool isActive() const { return m_isActive; }
    void setActive(bool active) { m_isActive = active; }

private:
    struct InfectedParticle {
        float angle;         // Angle actuel autour de l'ennemi (en radians)
        float orbitRadius;   // Rayon de l'orbite
        float speed;         // Vitesse de rotation (radians par seconde)
        float size;          // Taille de la particule
        sf::Color color;     // Couleur (rouge sombre ou noir)
    };

    sf::Vector2f m_position;
    bool m_isActive;
    float m_scale;  // Facteur de taille (1.0 = normal, 5.0 = géant)

    // Visuel de l'ennemi
    static constexpr float ENEMY_RADIUS = 12.0f;
    static constexpr float CORE_RADIUS = 8.0f;

    // Comportement
    static constexpr float DETECTION_RANGE = 300.0f;  // Distance de détection du joueur
    static constexpr float MOVE_SPEED = 80.0f;         // Vitesse de déplacement
    static constexpr float DAMAGE_COOLDOWN = 1.0f;    // Cooldown entre les dégâts

    // Particules infectées
    std::vector<InfectedParticle> m_particles;
    static constexpr int PARTICLE_COUNT = 8;

    // Animation
    float m_pulseTimer;

    // Collision timer
    float m_damageTimer;
};
