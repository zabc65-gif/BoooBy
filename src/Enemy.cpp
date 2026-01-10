#include "Enemy.hpp"
#include "Player.hpp"
#include <cmath>
#include <random>

Enemy::Enemy(const sf::Vector2f& position, float scale)
    : m_position(position)
    , m_isActive(true)
    , m_scale(scale)
    , m_pulseTimer(0.0f)
    , m_damageTimer(0.0f)
{
    // Nombre de particules proportionnel à la taille
    int particleCount = static_cast<int>(PARTICLE_COUNT * scale);

    // Initialiser les particules infectées qui tournent autour de l'ennemi
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
    std::uniform_real_distribution<float> radiusDist(15.0f * scale, 25.0f * scale);
    std::uniform_real_distribution<float> speedDist(1.5f / scale, 3.0f / scale);  // Plus lent si plus gros
    std::uniform_real_distribution<float> sizeDist(2.0f * scale, 4.0f * scale);
    std::uniform_int_distribution<int> colorChoice(0, 1);

    m_particles.reserve(particleCount);
    for (int i = 0; i < particleCount; ++i) {
        InfectedParticle particle;
        particle.angle = angleDist(gen);
        particle.orbitRadius = radiusDist(gen);
        particle.speed = speedDist(gen);
        particle.size = sizeDist(gen);

        // Alterner entre rouge sombre et noir
        if (colorChoice(gen) == 0) {
            particle.color = sf::Color(120, 20, 20);  // Rouge sombre
        } else {
            particle.color = sf::Color(30, 30, 30);   // Noir grisâtre
        }

        m_particles.push_back(particle);
    }
}

void Enemy::update(sf::Time deltaTime, const Player& player) {
    if (!m_isActive) return;

    float dt = deltaTime.asSeconds();

    // Mettre à jour le timer d'animation de pulsation
    m_pulseTimer += dt;

    // Mettre à jour le timer de dégâts
    if (m_damageTimer > 0.0f) {
        m_damageTimer -= dt;
    }

    // Mettre à jour les particules (rotation autour de l'ennemi)
    for (auto& particle : m_particles) {
        particle.angle += particle.speed * dt;
        // Garder l'angle entre 0 et 2π
        if (particle.angle > 2.0f * 3.14159f) {
            particle.angle -= 2.0f * 3.14159f;
        }
    }

    // Détection et poursuite du joueur
    sf::Vector2f playerPos = player.getPosition();
    // Ajuster pour viser le centre du joueur (approximativement)
    playerPos.x += 51.0f;  // Largeur joueur / 2
    playerPos.y += 51.0f;  // Hauteur joueur / 2

    // Calculer la distance au joueur
    sf::Vector2f toPlayer = playerPos - m_position;
    float distance = std::sqrt(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

    // Si le joueur est dans la zone de détection, se rapprocher
    // La portée de détection augmente avec la taille, la vitesse diminue
    float detectionRange = DETECTION_RANGE * m_scale;
    float moveSpeed = MOVE_SPEED / m_scale;  // Plus gros = plus lent

    if (distance < detectionRange && distance > 0.0f) {
        // Normaliser le vecteur direction
        sf::Vector2f direction = toPlayer / distance;

        // Déplacer l'ennemi vers le joueur (très lent pour les géants)
        m_position += direction * moveSpeed * dt;
    }
}

void Enemy::render(sf::RenderWindow& window) {
    if (!m_isActive) return;

    // Rayon du corps avec l'échelle
    float scaledRadius = ENEMY_RADIUS * m_scale;
    float scaledCoreRadius = CORE_RADIUS * m_scale;

    // Effet de pulsation pour le corps de l'ennemi
    float pulseFactor = 0.5f + 0.5f * std::sin(m_pulseTimer * 3.0f);

    // Corps principal de l'ennemi (cercle rouge sombre/noir)
    sf::CircleShape body(scaledRadius);
    body.setPosition(m_position);
    body.setOrigin(sf::Vector2f(scaledRadius, scaledRadius));

    // Gradient du rouge sombre au noir avec la pulsation
    unsigned char red = static_cast<unsigned char>(80 + 40 * pulseFactor);
    body.setFillColor(sf::Color(red, 10, 10));

    // Contour plus sombre
    body.setOutlineThickness(2.0f);
    body.setOutlineColor(sf::Color(20, 5, 5));

    window.draw(body);

    // Noyau central plus sombre (infecté)
    sf::CircleShape core(scaledCoreRadius);
    core.setPosition(m_position);
    core.setOrigin(sf::Vector2f(scaledCoreRadius, scaledCoreRadius));
    core.setFillColor(sf::Color(30, 5, 5));
    window.draw(core);

    // Dessiner les particules infectées qui tournent autour
    for (const auto& particle : m_particles) {
        // Calculer la position de la particule en orbite
        float particleX = m_position.x + std::cos(particle.angle) * particle.orbitRadius;
        float particleY = m_position.y + std::sin(particle.angle) * particle.orbitRadius;

        // Particule principale
        sf::CircleShape particleShape(particle.size);
        particleShape.setPosition(sf::Vector2f(particleX, particleY));
        particleShape.setOrigin(sf::Vector2f(particle.size, particle.size));
        particleShape.setFillColor(particle.color);

        // Halo autour de la particule
        sf::CircleShape halo(particle.size * 1.5f);
        halo.setPosition(sf::Vector2f(particleX, particleY));
        halo.setOrigin(sf::Vector2f(particle.size * 1.5f, particle.size * 1.5f));
        sf::Color haloColor = particle.color;
        haloColor.a = 80;  // Transparent
        halo.setFillColor(haloColor);

        window.draw(halo);
        window.draw(particleShape);
    }
}

bool Enemy::checkCollision(const Player& player) const {
    if (!m_isActive) return false;

    // Utiliser les bounds du joueur
    sf::Vector2f playerPos = player.getPosition();
    sf::FloatRect playerBounds(sf::Vector2f(playerPos.x, playerPos.y), sf::Vector2f(102.0f, 102.0f));

    // Bounds de l'ennemi
    sf::FloatRect enemyBounds = getBounds();

    // SFML 3.0: utiliser findIntersection
    return enemyBounds.findIntersection(playerBounds).has_value();
}

sf::FloatRect Enemy::getBounds() const {
    float scaledRadius = ENEMY_RADIUS * m_scale;
    return sf::FloatRect(
        sf::Vector2f(m_position.x - scaledRadius, m_position.y - scaledRadius),
        sf::Vector2f(scaledRadius * 2.0f, scaledRadius * 2.0f)
    );
}
