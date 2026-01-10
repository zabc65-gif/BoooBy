#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>
#include "ParticleSystem.hpp"
#include "CharacterSelection.hpp"

class Player {
public:
    enum class State {
        Idle,
        Walking,
        Running,
        Jumping,
        Falling,
        UsingPower
    };

    Player(CharacterType characterType = CharacterType::Wizard);

    void handleInput(sf::Keyboard::Key key, bool isPressed);
    void update(sf::Time deltaTime);
    void render(sf::RenderWindow& window);

    sf::Vector2f getPosition() const { return m_position; }
    void setPosition(const sf::Vector2f& position) { m_position = position; }

    sf::Vector2f getVelocity() const { return m_velocity; }
    void setVelocity(const sf::Vector2f& velocity) { m_velocity = velocity; }

    void setGrounded(bool grounded) {
        m_isGrounded = grounded;
        if (grounded) {
            // Réinitialiser les sauts quand on touche le sol
            m_jumpsRemaining = m_hasDoubleJump ? 2 : 1;
        }
    }
    bool isGrounded() const { return m_isGrounded; }

    void setState(State state) { m_state = state; }

    // Système de vie
    int getHealth() const { return m_health; }
    int getMaxHealth() const { return m_maxHealth; }
    void takeDamage(int damage);
    void resetHealth() { m_health = m_maxHealth; m_invincibilityTimer = 0.0f; }
    bool isDead() const { return m_health <= 0; }
    bool isInvincible() const { return m_invincibilityTimer > 0.0f; }
    void updateInvincibility(float deltaTime);

    // Désintégration
    void triggerDisintegration();
    void resetDisintegration() { m_isDisintegrating = false; }
    bool isDisintegrating() const { return m_isDisintegrating; }
    ParticleSystem& getParticleSystem() { return m_particleSystem; }

    // Double saut
    void unlockDoubleJump() { m_hasDoubleJump = true; }
    void lockDoubleJump() { m_hasDoubleJump = false; m_jumpsRemaining = 1; }
    bool hasDoubleJump() const { return m_hasDoubleJump; }
    int getJumpsRemaining() const { return m_jumpsRemaining; }

    // Charge du héros
    void unlockHeroCharge() { m_hasHeroCharge = true; }
    void lockHeroCharge() { m_hasHeroCharge = false; }
    bool hasHeroCharge() const { return m_hasHeroCharge; }
    bool isPreparingCharge() const { return m_isPreparingCharge; }
    bool isCharging() const { return m_isCharging; }
    float getChargeProgress() const { return m_chargeTimer / CHARGE_DURATION; }
    sf::FloatRect getChargeBounds() const;  // Bounds de la zone d'attaque pendant la charge

private:
    void updatePhysics(sf::Time deltaTime);
    void updateAnimation(sf::Time deltaTime);
    void loadAnimationFrames(const std::string& directory, const std::string& prefix, int frameCount, std::vector<std::shared_ptr<sf::Texture>>& textures);
    void loadSpriteSheet(const std::string& filepath, int frameWidth, int frameHeight, int totalFrames, std::vector<std::shared_ptr<sf::Texture>>& textures);

private:
    sf::Vector2f m_position;
    sf::Vector2f m_velocity;

    State m_state;
    State m_previousState;

    bool m_isMovingLeft;
    bool m_isMovingRight;
    bool m_isMovingUp;
    bool m_isMovingDown;
    bool m_isRunning;
    bool m_isGrounded;
    bool m_facingRight;

    // Debug constants
    static constexpr bool SHOW_DEBUG_HITBOX = false; // Mettre à true pour afficher la hitbox rouge

    // Physics constants
    static constexpr float GRAVITY = 980.0f;
    static constexpr float WALK_SPEED = 150.0f;
    static constexpr float RUN_SPEED = 300.0f;
    static constexpr float JUMP_FORCE = -500.0f; // Force de saut augmentée

    // Animation constants
    static constexpr float FRAME_TIME = 0.05f; // 50ms par frame = 20 FPS

    // Character type
    CharacterType m_characterType;
    float m_spriteScale;  // Scale du sprite (varie selon le personnage)
    float m_animationSpeed;  // Multiplicateur de vitesse d'animation (1.0 = normal, 2.0 = deux fois plus lent)

    // Animations
    std::vector<std::shared_ptr<sf::Texture>> m_idleTextures;
    std::vector<std::shared_ptr<sf::Texture>> m_walkTextures;
    std::vector<std::shared_ptr<sf::Texture>> m_jumpTextures;

    std::unique_ptr<sf::Sprite> m_sprite;
    int m_currentFrame;
    float m_frameTimer;

    // Son
    sf::SoundBuffer m_jumpSoundBuffer;
    std::unique_ptr<sf::Sound> m_jumpSound;

    // Vie
    int m_health;
    int m_maxHealth;
    float m_invincibilityTimer;  // Temps d'invincibilité restant après avoir pris des dégâts
    static constexpr float INVINCIBILITY_DURATION = 1.0f;  // 1 seconde d'invincibilité

    // Désintégration
    ParticleSystem m_particleSystem;
    bool m_isDisintegrating;

    // Double saut
    bool m_hasDoubleJump;
    int m_jumpsRemaining;  // 1 = saut simple, 2 = double saut disponible

    // Charge du héros
    bool m_hasHeroCharge;
    bool m_isPreparingCharge;  // Phase de préparation (particules qui tournent)
    bool m_isCharging;         // Phase de charge active (déplacement rapide)
    float m_chargeTimer;
    float m_chargeCooldown;
    float m_prepareTimer;      // Timer pour la phase de préparation
    float m_chargeAlpha;       // Opacité du personnage pendant la charge (0.0 = invisible, 1.0 = opaque)
    sf::Vector2f m_chargeDirection;  // Direction normalisée de la charge (supporte les diagonales)
    sf::Vector2f m_chargeStartPosition;
    std::vector<sf::Vector2f> m_chargeTrailPositions;  // Traînée de la charge

    // Particules d'explosion lors du déclenchement
    struct ExplosionParticle {
        sf::Vector2f position;
        sf::Vector2f velocity;
        float life;       // 1.0 = neuve, 0.0 = morte
        float size;
        sf::Color color;
    };
    std::vector<ExplosionParticle> m_explosionParticles;

    static constexpr float CHARGE_PREPARE_DURATION = 0.5f;  // Durée de préparation
    static constexpr float CHARGE_DURATION = 0.4f;          // Durée de la charge
    static constexpr float CHARGE_SPEED = 800.0f;           // Vitesse de la charge
    static constexpr float CHARGE_COOLDOWN = 1.5f;          // Cooldown entre les charges
    static constexpr int CHARGE_TRAIL_LENGTH = 30;          // Longueur de la traînée

    // Traînée des lucioles
    struct FireflyTrailPoint {
        sf::Vector2f position;
        float age;  // Age du point (0.0 = nouveau, 1.0 = ancien)
    };
    std::vector<FireflyTrailPoint> m_fireflyTrail1;
    std::vector<FireflyTrailPoint> m_fireflyTrail2;
    sf::Vector2f m_firefly1Position;  // Position actuelle de la luciole 1
    sf::Vector2f m_firefly2Position;  // Position actuelle de la luciole 2
    sf::Vector2f m_fireflyLagPosition;  // Position avec retard (pour l'inertie)
    float m_fireflyTimer;  // Timer partagé pour l'animation des lucioles
    float m_fireflyAlpha1;  // Opacité de la luciole 1 (0.0 = invisible, 1.0 = opaque)
    float m_fireflyAlpha2;  // Opacité de la luciole 2 (0.0 = invisible, 1.0 = opaque)
    static constexpr int MAX_TRAIL_POINTS = 10;
    static constexpr float TRAIL_FADE_SPEED = 3.0f;
    static constexpr float FIREFLY_LAG_SMOOTH_X = 0.06f;  // Facteur de lissage horizontal (plus petit = plus de retard)
    static constexpr float FIREFLY_LAG_SMOOTH_Y = 0.12f;  // Facteur de lissage vertical
    static constexpr float FIREFLY_FADE_SPEED = 2.0f;  // Vitesse d'apparition/disparition (unités par seconde)
};
