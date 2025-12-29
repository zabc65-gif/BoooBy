#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>
#include "Player.hpp"
#include "Door.hpp"
#include "PauseMenu.hpp"
#include "Camera.hpp"

class Game {
public:
    Game();
    ~Game();

    void run();

private:
    void processEvents();
    void update(sf::Time deltaTime);
    void render();

    void handlePlayerInput(sf::Keyboard::Key key, bool isPressed);
    void handleMenuInput(sf::Keyboard::Key key);

private:
    static const sf::Time TimePerFrame;

    sf::RenderWindow m_window;
    std::unique_ptr<Player> m_player;
    std::unique_ptr<Door> m_entranceDoor;
    std::unique_ptr<PauseMenu> m_pauseMenu;
    std::unique_ptr<Camera> m_camera;

    bool m_isPaused;
};
