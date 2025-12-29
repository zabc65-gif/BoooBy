#include "AnimatedSprite.hpp"
#include <iostream>
#include <filesystem>
#include <algorithm>

AnimatedSprite::AnimatedSprite()
    : m_currentFrame(0)
    , m_frameTime(1.0f / DEFAULT_FPS)
    , m_timeSinceLastFrame(0.0f)
    , m_isPlaying(true)
    , m_loop(true)
{
}

bool AnimatedSprite::loadAnimation(const std::string& directory, const std::string& prefix, int frameCount) {
    m_textures.clear();
    m_textures.reserve(frameCount);

    for (int i = 0; i < frameCount; ++i) {
        // Format: "Chara - BlueIdle00000.png"
        std::string filename = directory + "/" + prefix + std::to_string(i).insert(0, 5 - std::to_string(i).length(), '0') + ".png";

        auto texture = std::make_shared<sf::Texture>();
        if (!texture->loadFromFile(filename)) {
            std::cerr << "Failed to load texture: " << filename << std::endl;
            return false;
        }
        m_textures.push_back(texture);
    }

    if (!m_textures.empty()) {
        m_sprite = std::make_unique<sf::Sprite>(*m_textures[0]);
        // PAS DE SCALE - affiche en taille réelle (512x512)
        // m_sprite->setScale(sf::Vector2f(0.2f, 0.2f));

        sf::FloatRect bounds = m_sprite->getLocalBounds();
        std::cout << "Loaded " << frameCount << " frames. Texture size: "
                  << bounds.size.x << "x" << bounds.size.y << std::endl;
        std::cout << "Sprite will be rendered at FULL SIZE (no scale)" << std::endl;
    }

    return !m_textures.empty();
}

void AnimatedSprite::update(sf::Time deltaTime) {
    if (!m_isPlaying || m_textures.empty() || !m_sprite) {
        return;
    }

    m_timeSinceLastFrame += deltaTime.asSeconds();

    if (m_timeSinceLastFrame >= m_frameTime) {
        m_timeSinceLastFrame -= m_frameTime;
        m_currentFrame++;

        if (m_currentFrame >= static_cast<int>(m_textures.size())) {
            if (m_loop) {
                m_currentFrame = 0;
            } else {
                m_currentFrame = static_cast<int>(m_textures.size()) - 1;
                m_isPlaying = false;
            }
        }

        // Save current transform properties before changing texture
        sf::Vector2f currentScale = m_sprite->getScale();
        sf::Vector2f currentPosition = m_sprite->getPosition();

        m_sprite->setTexture(*m_textures[m_currentFrame]);

        // Restore transform properties
        m_sprite->setScale(currentScale);
        m_sprite->setPosition(currentPosition);
    }
}

void AnimatedSprite::render(sf::RenderWindow& window) {
    if (m_sprite) {
        window.draw(*m_sprite);
    }
}

void AnimatedSprite::play() {
    m_isPlaying = true;
}

void AnimatedSprite::pause() {
    m_isPlaying = false;
}

void AnimatedSprite::reset() {
    m_currentFrame = 0;
    m_timeSinceLastFrame = 0.0f;
    if (!m_textures.empty() && m_sprite) {
        // Save current transform properties
        sf::Vector2f currentScale = m_sprite->getScale();
        sf::Vector2f currentPosition = m_sprite->getPosition();

        m_sprite->setTexture(*m_textures[0]);

        // Restore transform properties
        m_sprite->setScale(currentScale);
        m_sprite->setPosition(currentPosition);
    }
}

void AnimatedSprite::setFrameRate(float fps) {
    m_frameTime = 1.0f / fps;
}

void AnimatedSprite::setLoop(bool loop) {
    m_loop = loop;
}

void AnimatedSprite::setPosition(const sf::Vector2f& position) {
    if (m_sprite) {
        m_sprite->setPosition(position);
    }
}

void AnimatedSprite::setScale(const sf::Vector2f& scale) {
    if (m_sprite) {
        m_sprite->setScale(scale);
    }
}

void AnimatedSprite::setOrigin(const sf::Vector2f& origin) {
    if (m_sprite) {
        m_sprite->setOrigin(origin);
    }
}

sf::Vector2f AnimatedSprite::getPosition() const {
    if (m_sprite) {
        return m_sprite->getPosition();
    }
    return sf::Vector2f(0.0f, 0.0f);
}

sf::FloatRect AnimatedSprite::getGlobalBounds() const {
    if (m_sprite) {
        return m_sprite->getGlobalBounds();
    }
    return sf::FloatRect();
}
