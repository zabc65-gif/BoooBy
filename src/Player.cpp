#include "Player.hpp"
#include <iostream>

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
            if (isPressed && m_isGrounded) {
                m_velocity.y = JUMP_FORCE;
                m_state = State::Jumping;
                m_isGrounded = false;
            }
            break;

        default:
            break;
    }
}

void Player::update(sf::Time deltaTime) {
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
