#include "Player.hpp"
#include <iostream>

Player::Player()
    : m_position(0.0f, 0.0f)
    , m_velocity(0.0f, 0.0f)
    , m_state(State::Idle)
    , m_isMovingLeft(false)
    , m_isMovingRight(false)
    , m_isRunning(false)
    , m_isGrounded(false)
    , m_animationTime(sf::Time::Zero)
{
    // Sprite temporaire (rectangle blanc) en attendant les vraies textures
    m_sprite.setSize(sf::Vector2f(40.0f, 60.0f));
    m_sprite.setFillColor(sf::Color::White);
    m_sprite.setOrigin(sf::Vector2f(20.0f, 60.0f)); // Origine en bas au centre
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
    }
    else if (m_isMovingRight && !m_isMovingLeft) {
        float speed = m_isRunning ? RUN_SPEED : WALK_SPEED;
        m_velocity.x = speed;
        m_state = m_isRunning ? State::Running : State::Walking;
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
    }

    // Mise à jour de la position
    m_position += m_velocity * dt;

    // Sol temporaire (y = 500)
    const float groundLevel = 500.0f;
    if (m_position.y >= groundLevel) {
        m_position.y = groundLevel;
        m_velocity.y = 0.0f;
        m_isGrounded = true;
        if (m_state == State::Jumping || m_state == State::Falling) {
            m_state = State::Idle;
        }
    }
    else {
        if (m_velocity.y > 0 && m_state != State::Falling) {
            m_state = State::Falling;
        }
    }

    // Limites de l'écran (temporaire)
    if (m_position.x < 20.0f) m_position.x = 20.0f;
    if (m_position.x > 1260.0f) m_position.x = 1260.0f;
}

void Player::updateAnimation(sf::Time deltaTime) {
    m_animationTime += deltaTime;

    // Placeholder pour l'animation - on change juste la couleur selon l'état
    switch (m_state) {
        case State::Idle:
            m_sprite.setFillColor(sf::Color::White);
            break;
        case State::Walking:
            m_sprite.setFillColor(sf::Color::Green);
            break;
        case State::Running:
            m_sprite.setFillColor(sf::Color::Yellow);
            break;
        case State::Jumping:
            m_sprite.setFillColor(sf::Color::Cyan);
            break;
        case State::Falling:
            m_sprite.setFillColor(sf::Color::Magenta);
            break;
        case State::UsingPower:
            m_sprite.setFillColor(sf::Color::Red);
            break;
    }
}

void Player::render(sf::RenderWindow& window) {
    m_sprite.setPosition(m_position);
    window.draw(m_sprite);
}
