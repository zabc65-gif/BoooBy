#include "ParticleSystem.hpp"
#include <cmath>
#include <random>

ParticleSystem::ParticleSystem() {
}

void ParticleSystem::createDisintegrationEffect(const sf::Vector2f& position, const sf::Color& baseColor) {
    // Générateur de nombres aléatoires
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 360.0f);
    std::uniform_real_distribution<float> speedDist(PARTICLE_MIN_SPEED, PARTICLE_MAX_SPEED);
    std::uniform_real_distribution<float> sizeDist(PARTICLE_MIN_SIZE, PARTICLE_MAX_SIZE);
    std::uniform_real_distribution<float> lifetimeDist(PARTICLE_LIFETIME * 0.7f, PARTICLE_LIFETIME * 1.3f);

    // Créer les particules
    for (int i = 0; i < PARTICLE_COUNT; ++i) {
        Particle particle;

        // Position initiale (centre du joueur)
        particle.position = position;

        // Vitesse dans une direction aléatoire
        float angle = angleDist(gen) * 3.14159f / 180.0f;
        float speed = speedDist(gen);
        particle.velocity.x = std::cos(angle) * speed;
        particle.velocity.y = std::sin(angle) * speed - 100.0f; // Légère direction vers le haut

        // Couleur basée sur la couleur du joueur avec variations
        particle.color = baseColor;
        particle.color.a = 255;

        // Taille et durée de vie
        particle.size = sizeDist(gen);
        particle.maxLifetime = lifetimeDist(gen);
        particle.lifetime = particle.maxLifetime;

        m_particles.push_back(particle);
    }
}

void ParticleSystem::update(sf::Time deltaTime) {
    float dt = deltaTime.asSeconds();

    // Mettre à jour chaque particule
    for (auto it = m_particles.begin(); it != m_particles.end();) {
        // Diminuer la durée de vie
        it->lifetime -= dt;

        // Supprimer si morte
        if (it->lifetime <= 0.0f) {
            it = m_particles.erase(it);
            continue;
        }

        // Appliquer la gravité
        it->velocity.y += GRAVITY * dt;

        // Mettre à jour la position
        it->position += it->velocity * dt;

        // Fade out basé sur la durée de vie restante
        float lifeRatio = it->lifetime / it->maxLifetime;
        it->color.a = static_cast<unsigned char>(255 * lifeRatio);

        ++it;
    }
}

void ParticleSystem::render(sf::RenderWindow& window) {
    for (const auto& particle : m_particles) {
        sf::CircleShape shape(particle.size);
        shape.setPosition(particle.position);
        shape.setFillColor(particle.color);
        shape.setOrigin(sf::Vector2f(particle.size, particle.size));
        window.draw(shape);
    }
}
