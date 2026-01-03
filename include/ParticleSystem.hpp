#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;
    float lifetime;
    float maxLifetime;
    float size;
};

class ParticleSystem {
public:
    ParticleSystem();

    // Crée un effet de désintégration à une position donnée
    void createDisintegrationEffect(const sf::Vector2f& position, const sf::Color& baseColor);

    void update(sf::Time deltaTime);
    void render(sf::RenderWindow& window);

    bool isActive() const { return !m_particles.empty(); }

private:
    std::vector<Particle> m_particles;

    // Constantes pour l'effet de désintégration
    static constexpr int PARTICLE_COUNT = 50;
    static constexpr float PARTICLE_LIFETIME = 1.5f;
    static constexpr float PARTICLE_MIN_SIZE = 2.0f;
    static constexpr float PARTICLE_MAX_SIZE = 6.0f;
    static constexpr float PARTICLE_MIN_SPEED = 50.0f;
    static constexpr float PARTICLE_MAX_SPEED = 200.0f;
    static constexpr float GRAVITY = 200.0f;
};
