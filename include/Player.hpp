#pragma once

#include <SFML/Graphics.hpp>

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

    Player();

    void handleInput(sf::Keyboard::Key key, bool isPressed);
    void update(sf::Time deltaTime);
    void render(sf::RenderWindow& window);

    sf::Vector2f getPosition() const { return m_position; }
    void setPosition(const sf::Vector2f& position) { m_position = position; }

private:
    void updatePhysics(sf::Time deltaTime);
    void updateAnimation(sf::Time deltaTime);

private:
    sf::Vector2f m_position;
    sf::Vector2f m_velocity;

    State m_state;

    bool m_isMovingLeft;
    bool m_isMovingRight;
    bool m_isRunning;
    bool m_isGrounded;

    // Physics constants
    static constexpr float GRAVITY = 980.0f;
    static constexpr float WALK_SPEED = 150.0f;
    static constexpr float RUN_SPEED = 300.0f;
    static constexpr float JUMP_FORCE = -500.0f;

    // Sprite and animation
    sf::RectangleShape m_sprite; // Placeholder until we have textures
    sf::Time m_animationTime;
};
