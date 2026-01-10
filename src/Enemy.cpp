#include "Enemy.hpp"
#include "Player.hpp"
#include <cmath>
#include <random>
#include <iostream>
#include <algorithm>

Enemy::Enemy(const sf::Vector2f& position, float scale)
    : m_position(position)
    , m_isActive(true)
    , m_scale(scale)
    , m_pulseTimer(0.0f)
    , m_damageTimer(0.0f)
    , m_isDying(false)
    , m_deathTimer(0.0f)
    , m_shockwaveRadius(0.0f)
    , m_shockwaveAlpha(255.0f)
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

    // Si en train de mourir, gérer l'animation de mort
    if (m_isDying) {
        m_deathTimer -= dt;

        // Mise à jour de l'onde de choc
        m_shockwaveRadius += 400.0f * m_scale * dt;  // Expansion rapide
        m_shockwaveAlpha -= 400.0f * dt;  // Disparition progressive
        if (m_shockwaveAlpha < 0.0f) m_shockwaveAlpha = 0.0f;

        // Mise à jour des particules de mort
        for (auto& p : m_deathParticles) {
            p.position += p.velocity * dt;
            p.velocity *= 0.96f;  // Friction
            p.velocity.y += 200.0f * dt;  // Légère gravité
            p.life -= dt * 1.8f;
            p.size *= 0.98f;
            p.rotation += p.rotationSpeed * dt;
        }

        // Supprimer les particules mortes
        m_deathParticles.erase(
            std::remove_if(m_deathParticles.begin(), m_deathParticles.end(),
                [](const DeathParticle& p) { return p.life <= 0.0f; }),
            m_deathParticles.end());

        // Quand l'animation est finie, désactiver l'ennemi
        if (m_deathTimer <= 0.0f && m_deathParticles.empty()) {
            m_isActive = false;
        }
        return;
    }

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

void Enemy::triggerDeath() {
    if (m_isDying) return;  // Déjà en train de mourir

    m_isDying = true;
    m_deathTimer = DEATH_DURATION;
    m_shockwaveRadius = ENEMY_RADIUS * m_scale;
    m_shockwaveAlpha = 255.0f;

    // Créer l'explosion de particules
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
    std::uniform_real_distribution<float> speedDist(150.0f, 400.0f);
    std::uniform_real_distribution<float> sizeDist(4.0f * m_scale, 12.0f * m_scale);
    std::uniform_real_distribution<float> rotSpeedDist(-10.0f, 10.0f);
    std::uniform_int_distribution<int> colorChoice(0, 3);

    // Nombre de particules proportionnel à la taille
    int particleCount = static_cast<int>(30 * m_scale);

    for (int i = 0; i < particleCount; ++i) {
        DeathParticle p;
        float angle = angleDist(gen);
        float speed = speedDist(gen);

        p.position = m_position;
        p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
        p.life = 1.0f;
        p.size = sizeDist(gen);
        p.rotation = angleDist(gen);
        p.rotationSpeed = rotSpeedDist(gen);

        // Palette de couleurs : rouge, orange, jaune, blanc (feu)
        int color = colorChoice(gen);
        if (color == 0) p.color = sf::Color(200, 50, 30, 255);       // Rouge
        else if (color == 1) p.color = sf::Color(255, 120, 30, 255); // Orange
        else if (color == 2) p.color = sf::Color(255, 220, 80, 255); // Jaune
        else p.color = sf::Color(255, 255, 220, 255);                // Blanc chaud

        m_deathParticles.push_back(p);
    }

    // Convertir les particules infectées en particules de mort (elles explosent aussi)
    for (const auto& infected : m_particles) {
        DeathParticle p;
        float particleX = m_position.x + std::cos(infected.angle) * infected.orbitRadius;
        float particleY = m_position.y + std::sin(infected.angle) * infected.orbitRadius;

        p.position = sf::Vector2f(particleX, particleY);

        // Éjecter dans la direction radiale
        float ejectionAngle = infected.angle + angleDist(gen) * 0.3f;
        float speed = speedDist(gen) * 0.8f;
        p.velocity = sf::Vector2f(std::cos(ejectionAngle) * speed, std::sin(ejectionAngle) * speed);

        p.life = 1.0f;
        p.size = infected.size * 2.0f;
        p.color = infected.color;
        p.rotation = 0.0f;
        p.rotationSpeed = rotSpeedDist(gen);

        m_deathParticles.push_back(p);
    }

    // Vider les particules infectées normales
    m_particles.clear();

    std::cout << "Enemy death triggered with " << m_deathParticles.size() << " particles!" << std::endl;
}

void Enemy::render(sf::RenderWindow& window) {
    if (!m_isActive) return;

    // Si en train de mourir, dessiner l'animation de mort
    if (m_isDying) {
        // Dessiner l'onde de choc (cercle qui s'étend)
        if (m_shockwaveAlpha > 0.0f) {
            // Onde de choc externe (orange)
            sf::CircleShape shockwave(m_shockwaveRadius);
            shockwave.setPosition(m_position);
            shockwave.setOrigin(sf::Vector2f(m_shockwaveRadius, m_shockwaveRadius));
            shockwave.setFillColor(sf::Color::Transparent);
            shockwave.setOutlineThickness(4.0f * m_scale);
            shockwave.setOutlineColor(sf::Color(255, 150, 50, static_cast<unsigned char>(m_shockwaveAlpha)));
            window.draw(shockwave);

            // Onde de choc interne (jaune)
            sf::CircleShape innerShockwave(m_shockwaveRadius * 0.7f);
            innerShockwave.setPosition(m_position);
            innerShockwave.setOrigin(sf::Vector2f(m_shockwaveRadius * 0.7f, m_shockwaveRadius * 0.7f));
            innerShockwave.setFillColor(sf::Color::Transparent);
            innerShockwave.setOutlineThickness(2.0f * m_scale);
            innerShockwave.setOutlineColor(sf::Color(255, 255, 100, static_cast<unsigned char>(m_shockwaveAlpha * 0.7f)));
            window.draw(innerShockwave);
        }

        // Dessiner les particules d'explosion
        for (const auto& p : m_deathParticles) {
            // Halo externe
            sf::CircleShape glow(p.size * 1.8f);
            glow.setPosition(p.position);
            glow.setOrigin(sf::Vector2f(p.size * 1.8f, p.size * 1.8f));
            glow.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, static_cast<unsigned char>(p.life * 60)));
            window.draw(glow);

            // Particule principale
            sf::CircleShape particle(p.size);
            particle.setPosition(p.position);
            particle.setOrigin(sf::Vector2f(p.size, p.size));
            particle.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, static_cast<unsigned char>(p.life * 255)));
            window.draw(particle);

            // Coeur brillant
            sf::CircleShape core(p.size * 0.4f);
            core.setPosition(p.position);
            core.setOrigin(sf::Vector2f(p.size * 0.4f, p.size * 0.4f));
            core.setFillColor(sf::Color(255, 255, 255, static_cast<unsigned char>(p.life * 200)));
            window.draw(core);
        }

        // Flash central au début de l'explosion
        if (m_deathTimer > DEATH_DURATION - 0.15f) {
            float flashProgress = (m_deathTimer - (DEATH_DURATION - 0.15f)) / 0.15f;
            float flashSize = (ENEMY_RADIUS * m_scale * 3.0f) * (1.0f - flashProgress);

            sf::CircleShape flash(flashSize);
            flash.setPosition(m_position);
            flash.setOrigin(sf::Vector2f(flashSize, flashSize));
            flash.setFillColor(sf::Color(255, 255, 200, static_cast<unsigned char>(flashProgress * 200)));
            window.draw(flash);
        }

        return;
    }

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
