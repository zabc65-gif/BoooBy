#include "Game.hpp"
#include <iostream>

const sf::Time Game::TimePerFrame = sf::seconds(1.f / 60.f);

Game::Game()
    : m_window(sf::VideoMode({1280, 720}), "BoooBy - Sheepy Remake", sf::Style::Close)
    , m_player(std::make_unique<Player>())
    , m_entranceDoor(std::make_unique<Door>(sf::Vector2f(50.0f, 380.0f)))
    , m_pauseMenu(std::make_unique<PauseMenu>())
    , m_camera(std::make_unique<Camera>(1280.0f, 720.0f))
    , m_level(std::make_unique<Level>())
    , m_isPaused(false)
    , m_isFinished(false)
{
    m_window.setFramerateLimit(60);

    // Charger la police
    if (!m_font.openFromFile("assets/Arial.ttf")) {
        std::cerr << "Failed to load font for victory message" << std::endl;
    }

    // Charger le niveau
    if (!m_level->load()) {
        std::cerr << "Failed to load level" << std::endl;
    }

    // Position initiale du joueur (à côté de la porte, au niveau du sol)
    // Le niveau fait 12 tiles de haut (12*64=768px), le sol est à la ligne 11 (y=11*64=704)
    // Le joueur fait 102px de haut, donc position Y = 704 - 102 = 602
    // Mais on le met légèrement en l'air pour que la gravité le place correctement
    m_player->setPosition(sf::Vector2f(150.0f, 550.0f));

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
    if (m_isFinished) return; // Ne plus rien faire si le jeu est terminé

    m_player->update(deltaTime);

    // Mettre à jour le niveau
    m_level->update(deltaTime, *m_player);

    // Vérifier si le joueur a atteint la ligne d'arrivée
    if (m_level->isPlayerAtFinish(*m_player)) {
        m_isFinished = true;
        std::cout << "Level completed!" << std::endl;
    }

    // Mettre à jour la caméra pour suivre le joueur
    m_camera->update(m_player->getPosition(), deltaTime);
}

void Game::render() {
    m_window.clear(sf::Color::Black); // Noir pour mieux voir les tiles

    // Appliquer la vue de la caméra pour les éléments du monde
    m_window.setView(m_camera->getView());

    // DEBUG: Afficher la position de la caméra
    static int frameCount = 0;
    if (frameCount % 60 == 0) {
        auto camCenter = m_camera->getView().getCenter();
        std::cout << "Camera center: (" << camCenter.x << ", " << camCenter.y << ")" << std::endl;
    }
    frameCount++;

    // Dessiner le niveau
    m_level->render(m_window);

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

    // Afficher le message de victoire si le niveau est terminé
    if (m_isFinished) {
        showVictoryMessage();
    }

    m_window.display();
}

void Game::showVictoryMessage() {
    // Rectangle semi-transparent en arrière-plan
    sf::RectangleShape overlay(sf::Vector2f(1280.0f, 720.0f));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    m_window.draw(overlay);

    // Texte "Finish!!" (SFML 3.0 nécessite la font dans le constructeur)
    sf::Text finishText(m_font, "Finish !!", 120);
    finishText.setFillColor(sf::Color(255, 215, 0)); // Or
    finishText.setOutlineColor(sf::Color::White);
    finishText.setOutlineThickness(5.0f);
    finishText.setStyle(sf::Text::Bold);

    // Centrer le texte
    sf::FloatRect textBounds = finishText.getLocalBounds();
    finishText.setOrigin(sf::Vector2f(textBounds.position.x + textBounds.size.x / 2.0f,
                                       textBounds.position.y + textBounds.size.y / 2.0f));
    finishText.setPosition(sf::Vector2f(640.0f, 360.0f));

    m_window.draw(finishText);
}

void Game::handlePlayerInput(sf::Keyboard::Key key, bool isPressed) {
    m_player->handleInput(key, isPressed);
}

void Game::handleMenuInput(sf::Keyboard::Key key) {
    m_pauseMenu->handleInput(key);
}
