#include "Player.hpp"
#include <iostream>
#include <cmath>

Player::Player()
    : m_position(0.0f, 0.0f)
    , m_velocity(0.0f, 0.0f)
    , m_state(State::Idle)
    , m_previousState(State::Idle)
    , m_isMovingLeft(false)
    , m_isMovingRight(false)
    , m_isRunning(false)
    , m_isGrounded(false)
    , m_facingRight(true)
    , m_currentFrame(0)
    , m_frameTimer(0.0f)
    , m_health(5)
    , m_maxHealth(5)
    , m_isDisintegrating(false)
    , m_hasDoubleJump(false)
    , m_jumpsRemaining(1)
{
    // Charger toutes les animations
    std::cout << "Chargement des animations du Blue Wizard..." << std::endl;

    loadAnimationFrames("assets/tiles/BlueWizard/2BlueWizardIdle", "Chara - BlueIdle", 20, m_idleTextures);
    loadAnimationFrames("assets/tiles/BlueWizard/2BlueWizardWalk", "Chara_BlueWalk", 20, m_walkTextures);
    loadAnimationFrames("assets/tiles/BlueWizard/2BlueWizardJump", "CharaWizardJump_", 8, m_jumpTextures);

    // Créer le sprite avec la première frame de l'animation idle
    if (!m_idleTextures.empty()) {
        m_sprite = std::make_unique<sf::Sprite>(*m_idleTextures[0]);
        m_sprite->setScale(sf::Vector2f(0.2f, 0.2f));
        std::cout << "✓ Animations chargées avec succès!" << std::endl;
    } else {
        std::cerr << "✗ Échec du chargement des animations!" << std::endl;
    }

    // Charger le son de saut
    if (m_jumpSoundBuffer.loadFromFile("assets/sounds/jump1.wav")) {
        m_jumpSound = std::make_unique<sf::Sound>(m_jumpSoundBuffer);
        m_jumpSound->setVolume(50.0f); // Volume à 50%
        std::cout << "✓ Son de saut chargé avec succès!" << std::endl;
    } else {
        std::cerr << "✗ Échec du chargement du son de saut (assets/sounds/jump1.wav)" << std::endl;
    }
}

void Player::loadAnimationFrames(const std::string& directory, const std::string& prefix, int frameCount, std::vector<std::shared_ptr<sf::Texture>>& textures) {
    for (int i = 0; i < frameCount; ++i) {
        auto texture = std::make_shared<sf::Texture>();

        // Format: "Chara - BlueIdle00000.png", "Chara - BlueIdle00001.png", etc.
        std::string filename = directory + "/" + prefix + std::string(5 - std::to_string(i).length(), '0') + std::to_string(i) + ".png";

        if (texture->loadFromFile(filename)) {
            textures.push_back(texture);
        } else {
            std::cerr << "Erreur lors du chargement de: " << filename << std::endl;
        }
    }

    std::cout << "  - " << textures.size() << " frames chargées depuis " << directory << std::endl;
}

void Player::handleInput(sf::Keyboard::Key key, bool isPressed) {
    switch (key) {
        case sf::Keyboard::Key::Left:
        case sf::Keyboard::Key::Q:
            m_isMovingLeft = isPressed;
            break;

        case sf::Keyboard::Key::Right:
        case sf::Keyboard::Key::D:
            m_isMovingRight = isPressed;
            break;

        case sf::Keyboard::Key::LShift:
            m_isRunning = isPressed;
            break;

        case sf::Keyboard::Key::Space:
            if (isPressed) {
                // Saut simple ou double saut
                int maxJumps = m_hasDoubleJump ? 2 : 1;

                if (m_jumpsRemaining > 0) {
                    m_velocity.y = JUMP_FORCE;
                    m_state = State::Jumping;
                    m_isGrounded = false;
                    m_jumpsRemaining--;

                    // Jouer le son de saut
                    if (m_jumpSound) {
                        m_jumpSound->play();
                    }

                    std::cout << "Jump! Jumps remaining: " << m_jumpsRemaining << "/" << maxJumps << std::endl;
                }
            }
            break;

        default:
            break;
    }
}

void Player::update(sf::Time deltaTime) {
    // Si le joueur est en train de se désintégrer, ne plus mettre à jour la physique
    if (m_isDisintegrating) {
        m_particleSystem.update(deltaTime);
        return;
    }

    updatePhysics(deltaTime);
    updateAnimation(deltaTime);
}

void Player::updatePhysics(sf::Time deltaTime) {
    float dt = deltaTime.asSeconds();

    // Mouvement horizontal
    if (m_isMovingLeft && !m_isMovingRight) {
        float speed = m_isRunning ? RUN_SPEED : WALK_SPEED;
        m_velocity.x = -speed;
        m_state = m_isRunning ? State::Running : State::Walking;
        m_facingRight = false; // Regarder à gauche
    }
    else if (m_isMovingRight && !m_isMovingLeft) {
        float speed = m_isRunning ? RUN_SPEED : WALK_SPEED;
        m_velocity.x = speed;
        m_state = m_isRunning ? State::Running : State::Walking;
        m_facingRight = true; // Regarder à droite
    }
    else {
        m_velocity.x = 0.0f;
        if (m_isGrounded) {
            m_state = State::Idle;
        }
    }

    // Gravité
    if (!m_isGrounded) {
        m_velocity.y += GRAVITY * dt;

        // Limiter la vitesse de chute maximale pour plus de fluidité
        if (m_velocity.y > 600.0f) {  // MAX_FALL_SPEED
            m_velocity.y = 600.0f;
        }
    }

    // Mise à jour de la position
    m_position += m_velocity * dt;

    // La gestion du sol est maintenant entièrement gérée par le système de niveau (Level::handlePlayerCollision)
    // On garde juste la détection de l'état de chute
    if (m_velocity.y > 0 && m_state != State::Falling && !m_isGrounded) {
        m_state = State::Falling;
    }
}

void Player::updateAnimation(sf::Time deltaTime) {
    if (!m_sprite) return;

    // Mettre à jour la position du sprite
    m_sprite->setPosition(m_position);

    // Si l'état a changé, réinitialiser l'animation
    if (m_state != m_previousState) {
        m_currentFrame = 0;
        m_frameTimer = 0.0f;
        m_previousState = m_state;
    }

    // Sélectionner le bon ensemble de textures selon l'état
    std::vector<std::shared_ptr<sf::Texture>>* currentAnimation = nullptr;

    switch (m_state) {
        case State::Idle:
            currentAnimation = &m_idleTextures;
            break;
        case State::Walking:
        case State::Running:
            currentAnimation = &m_walkTextures;
            break;
        case State::Jumping:
        case State::Falling:
            currentAnimation = &m_jumpTextures;
            break;
        default:
            currentAnimation = &m_idleTextures;
            break;
    }

    if (currentAnimation && !currentAnimation->empty()) {
        // Incrémenter le timer
        m_frameTimer += deltaTime.asSeconds();

        // Changer de frame si nécessaire
        if (m_frameTimer >= FRAME_TIME) {
            m_frameTimer -= FRAME_TIME;
            m_currentFrame = (m_currentFrame + 1) % currentAnimation->size();

            // Changer la texture du sprite
            m_sprite->setTexture(*(*currentAnimation)[m_currentFrame], false);
        }
    }
}

void Player::render(sf::RenderWindow& window) {
    // Si le joueur est en train de se désintégrer, afficher les particules au lieu du sprite
    if (m_isDisintegrating) {
        m_particleSystem.render(window);
        return;
    }

    if (!m_sprite) return;

    // Appliquer le flip horizontal selon la direction
    float scaleX = m_facingRight ? 0.2f : -0.2f;
    m_sprite->setScale(sf::Vector2f(scaleX, 0.2f));

    // Ajuster l'origine pour que le flip soit correct
    if (!m_facingRight) {
        m_sprite->setOrigin(sf::Vector2f(m_sprite->getTexture().getSize().x, 0.0f));
    } else {
        m_sprite->setOrigin(sf::Vector2f(0.0f, 0.0f));
    }

    // Dessiner le sprite
    window.draw(*m_sprite);

    // Dessiner les lucioles si le double saut est débloqué
    if (m_hasDoubleJump) {
        const float playerWidth = 102.0f;
        const float playerHeight = 102.0f;
        const float playerCenterX = m_position.x + playerWidth / 2.0f;
        const float playerTopY = m_position.y - 10.0f; // Un peu au-dessus du joueur

        // Animation de vol réaliste (mouvement en forme de 8 allongé)
        static float fireflyTimer = 0.0f;
        fireflyTimer += 0.08f; // Vitesse d'animation

        // Mouvement vertical et horizontal pour un vol naturel
        float offsetY = std::sin(fireflyTimer) * 5.0f;
        float offsetX = std::cos(fireflyTimer * 0.7f) * 3.0f; // Fréquence différente pour effet de lemniscate

        // Luciole gauche (premier saut disponible)
        if (m_jumpsRemaining >= 1) {
            float firefly1X = playerCenterX - 15.0f + offsetX;
            float firefly1Y = playerTopY + offsetY;

            sf::CircleShape firefly1(4.0f);
            firefly1.setPosition(sf::Vector2f(firefly1X, firefly1Y));
            firefly1.setFillColor(sf::Color(255, 255, 200, 200));
            firefly1.setOrigin(sf::Vector2f(4.0f, 4.0f));

            // Lueur autour de la luciole
            sf::CircleShape glow1(8.0f);
            glow1.setPosition(sf::Vector2f(firefly1X, firefly1Y));
            glow1.setFillColor(sf::Color(255, 255, 100, 80));
            glow1.setOrigin(sf::Vector2f(8.0f, 8.0f));

            window.draw(glow1);
            window.draw(firefly1);
        }

        // Luciole droite (deuxième saut disponible)
        if (m_jumpsRemaining >= 2) {
            // Mouvement déphasé pour un effet naturel (les deux lucioles ne volent pas de la même façon)
            float offsetY2 = std::sin(fireflyTimer + 1.5f) * 5.0f; // Déphasage de ~90°
            float offsetX2 = std::cos(fireflyTimer * 0.7f + 1.5f) * 3.0f;

            float firefly2X = playerCenterX + 15.0f + offsetX2;
            float firefly2Y = playerTopY + offsetY2;

            sf::CircleShape firefly2(4.0f);
            firefly2.setPosition(sf::Vector2f(firefly2X, firefly2Y));
            firefly2.setFillColor(sf::Color(255, 255, 200, 200));
            firefly2.setOrigin(sf::Vector2f(4.0f, 4.0f));

            // Lueur autour de la luciole
            sf::CircleShape glow2(8.0f);
            glow2.setPosition(sf::Vector2f(firefly2X, firefly2Y));
            glow2.setFillColor(sf::Color(255, 255, 100, 80));
            glow2.setOrigin(sf::Vector2f(8.0f, 8.0f));

            window.draw(glow2);
            window.draw(firefly2);
        }
    }

    // Dessiner le contour rouge pour debug (hitbox du joueur)
    // Pour activer: mettre SHOW_DEBUG_HITBOX à true dans Player.hpp
    if constexpr (SHOW_DEBUG_HITBOX) {
        const float playerWidth = 102.0f;
        const float playerHeight = 102.0f;
        sf::RectangleShape debugRect(sf::Vector2f(playerWidth, playerHeight));
        debugRect.setPosition(m_position);
        debugRect.setFillColor(sf::Color::Transparent);
        debugRect.setOutlineColor(sf::Color::Red);
        debugRect.setOutlineThickness(2.0f);
        window.draw(debugRect);
    }
}

void Player::takeDamage(int damage) {
    m_health -= damage;
    if (m_health < 0) {
        m_health = 0;
    }
    std::cout << "Player took " << damage << " damage. Health: " << m_health << "/" << m_maxHealth << std::endl;
}

void Player::triggerDisintegration() {
    if (m_isDisintegrating) return; // Éviter de déclencher plusieurs fois

    m_isDisintegrating = true;

    // Calculer la position centrale du joueur pour l'effet
    const float playerWidth = 102.0f;
    const float playerHeight = 102.0f;
    sf::Vector2f centerPosition = m_position + sf::Vector2f(playerWidth / 2.0f, playerHeight / 2.0f);

    // Créer l'effet de désintégration avec une couleur bleue (couleur du wizard)
    m_particleSystem.createDisintegrationEffect(centerPosition, sf::Color(100, 150, 255));

    std::cout << "Player disintegration triggered!" << std::endl;
}
