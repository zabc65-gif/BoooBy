#include "Game.hpp"
#include <iostream>

const sf::Time Game::TimePerFrame = sf::seconds(1.f / 60.f);

Game::Game()
    : m_window(sf::VideoMode({1280, 720}), "BoooBy - Sheepy Remake", sf::Style::Close)
    , m_player(std::make_unique<Player>())
    , m_entranceDoor(std::make_unique<Door>(sf::Vector2f(50.0f, 380.0f)))
    , m_pauseMenu(std::make_unique<PauseMenu>())
    , m_camera(std::make_unique<Camera>(1280.0f, 720.0f))
    , m_isPaused(false)
{
    m_window.setFramerateLimit(60);

    // Position initiale du joueur (à côté de la porte)
    m_player->setPosition(sf::Vector2f(150.0f, 500.0f));

    // Initialiser la caméra centrée sur le joueur
    m_camera->setPosition(m_player->getPosition());

    std::cout << "Game initialized successfully" << std::endl;
}

Game::~Game() {
}

void Game::run() {
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;

    while (m_window.isOpen()) {
        sf::Time deltaTime = clock.restart();
        timeSinceLastUpdate += deltaTime;

        while (timeSinceLastUpdate > TimePerFrame) {
            timeSinceLastUpdate -= TimePerFrame;

            processEvents();
            if (!m_isPaused) {
                update(TimePerFrame);
            }
        }

        render();
    }
}

void Game::processEvents() {
    while (auto event = m_window.pollEvent()) {
        if (const auto* closed = event->getIf<sf::Event::Closed>()) {
            m_window.close();
        }
        else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            // Gestion du menu pause
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                m_isPaused = !m_isPaused;
                if (m_isPaused) {
                    m_pauseMenu->resetAction();
                    std::cout << "Game paused" << std::endl;
                } else {
                    std::cout << "Game resumed" << std::endl;
                }
            }

            // Si en pause, gérer le menu
            if (m_isPaused) {
                handleMenuInput(keyPressed->code);
            } else {
                // Sinon, gérer les inputs du joueur
                handlePlayerInput(keyPressed->code, true);
            }
        }
        else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
            if (!m_isPaused) {
                handlePlayerInput(keyReleased->code, false);
            }
        }
    }

    // Vérifier l'action du menu
    if (m_isPaused) {
        auto action = m_pauseMenu->getSelectedAction();
        if (action == PauseMenu::MenuAction::Continue) {
            m_isPaused = false;
            m_pauseMenu->resetAction();
            std::cout << "Continue game" << std::endl;
        }
        else if (action == PauseMenu::MenuAction::Quit) {
            std::cout << "Quit game" << std::endl;
            m_window.close();
        }
    }
}

void Game::update(sf::Time deltaTime) {
    m_player->update(deltaTime);

    // Mettre à jour la caméra pour suivre le joueur
    m_camera->update(m_player->getPosition(), deltaTime);
}

void Game::render() {
    m_window.clear(sf::Color(20, 20, 30)); // Fond sombre pour l'ambiance

    // Appliquer la vue de la caméra pour les éléments du monde
    m_window.setView(m_camera->getView());

    // Dessiner la porte d'entrée
    m_entranceDoor->render(m_window);

    // Dessiner le joueur
    m_player->render(m_window);

    // Revenir à la vue par défaut pour l'interface utilisateur (menu pause)
    m_window.setView(m_window.getDefaultView());

    // Dessiner le menu pause si le jeu est en pause
    if (m_isPaused) {
        m_pauseMenu->render(m_window);
    }

    m_window.display();
}

void Game::handlePlayerInput(sf::Keyboard::Key key, bool isPressed) {
    m_player->handleInput(key, isPressed);
}

void Game::handleMenuInput(sf::Keyboard::Key key) {
    m_pauseMenu->handleInput(key);
}
