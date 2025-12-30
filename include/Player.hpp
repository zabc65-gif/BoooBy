#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

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

    sf::Vector2f getVelocity() const { return m_velocity; }
    void setVelocity(const sf::Vector2f& velocity) { m_velocity = velocity; }

    void setGrounded(bool grounded) { m_isGrounded = grounded; }
    bool isGrounded() const { return m_isGrounded; }

    void setState(State state) { m_state = state; }

private:
    void updatePhysics(sf::Time deltaTime);
    void updateAnimation(sf::Time deltaTime);
    void loadAnimationFrames(const std::string& directory, const std::string& prefix, int frameCount, std::vector<std::shared_ptr<sf::Texture>>& textures);

private:
    sf::Vector2f m_position;
    sf::Vector2f m_velocity;

    State m_state;
    State m_previousState;

    bool m_isMovingLeft;
    bool m_isMovingRight;
    bool m_isRunning;
    bool m_isGrounded;
    bool m_facingRight;

    // Physics constants
    static constexpr float GRAVITY = 980.0f;
    static constexpr float WALK_SPEED = 150.0f;
    static constexpr float RUN_SPEED = 300.0f;
    static constexpr float JUMP_FORCE = -500.0f; // Force de saut augmentée

    // Animation constants
    static constexpr float FRAME_TIME = 0.05f; // 50ms par frame = 20 FPS

    // Animations
    std::vector<std::shared_ptr<sf::Texture>> m_idleTextures;
    std::vector<std::shared_ptr<sf::Texture>> m_walkTextures;
    std::vector<std::shared_ptr<sf::Texture>> m_jumpTextures;

    std::unique_ptr<sf::Sprite> m_sprite;
    int m_currentFrame;
    float m_frameTimer;
};
