#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <memory>

class AnimatedSprite {
public:
    AnimatedSprite();

    // Load animation frames from a directory
    bool loadAnimation(const std::string& directory, const std::string& prefix, int frameCount);

    // Update animation
    void update(sf::Time deltaTime);

    // Render the current frame
    void render(sf::RenderWindow& window);

    // Control
    void play();
    void pause();
    void reset();
    void setFrameRate(float fps);
    void setLoop(bool loop);

    // Transform
    void setPosition(const sf::Vector2f& position);
    void setScale(const sf::Vector2f& scale);
    void setOrigin(const sf::Vector2f& origin);

    // Getters
    sf::Vector2f getPosition() const;
    sf::FloatRect getGlobalBounds() const;
    bool isPlaying() const { return m_isPlaying; }

private:
    std::vector<std::shared_ptr<sf::Texture>> m_textures;
    std::unique_ptr<sf::Sprite> m_sprite;

    int m_currentFrame;
    float m_frameTime;
    float m_timeSinceLastFrame;
    bool m_isPlaying;
    bool m_loop;

    static constexpr float DEFAULT_FPS = 12.0f;
};
