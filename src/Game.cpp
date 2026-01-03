#include "Game.hpp"
#include <iostream>

const sf::Time Game::TimePerFrame = sf::seconds(1.f / 60.f);

Game::Game()
    : m_window(sf::VideoMode({1280, 720}), "BoooBee - Sheepy Remake", sf::Style::Close)
    , m_player(std::make_unique<Player>())
    , m_pauseMenu(std::make_unique<PauseMenu>())
    , m_camera(std::make_unique<Camera>(1280.0f, 720.0f))
    , m_level(std::make_unique<Level>())
    , m_editor(std::make_unique<LevelEditor>(64))
    , m_isPaused(false)
    , m_isFinished(false)
    , m_isEditorMode(false)
    , m_isGameComplete(false)
    , m_isGameOver(false)
    , m_currentLevelNumber(0)  // Commencer au prologue
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

    // Position initiale du joueur au centre du portail d'entrée
    sf::Vector2f playerStartPos;
    if (m_level->hasEntrancePortal()) {
        sf::Vector2f portalPos = m_level->getEntrancePortalPosition();
        // Centrer le joueur sur le portail (portail = 64x64, joueur = 102x102)
        playerStartPos.x = portalPos.x + 32.0f - 51.0f;  // Centre du portail - moitié largeur joueur
        playerStartPos.y = portalPos.y + 32.0f - 51.0f;  // Centre du portail - moitié hauteur joueur
        std::cout << "Player starting at entrance portal center: (" << playerStartPos.x << ", " << playerStartPos.y << ")" << std::endl;
    } else {
        // Position par défaut
        playerStartPos = sf::Vector2f(150.0f, 550.0f);
    }
    m_player->setPosition(playerStartPos);

    // Initialiser la caméra centrée sur le joueur
    m_camera->setPosition(m_player->getPosition());

    // Charger et jouer la musique de fond
    if (m_backgroundMusic.openFromFile("assets/music/Melasse des ombres 1.mp3")) {
        m_backgroundMusic.setLooping(true);  // Boucler la musique
        m_backgroundMusic.setVolume(30.0f);  // Volume à 30%
        m_backgroundMusic.play();
        std::cout << "✓ Musique de fond chargée et lancée!" << std::endl;
    } else {
        std::cerr << "✗ Échec du chargement de la musique de fond" << std::endl;
    }

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
            // Basculer en mode éditeur avec F1
            if (keyPressed->code == sf::Keyboard::Key::F1) {
                m_isEditorMode = !m_isEditorMode;
                m_editor->setActive(m_isEditorMode);
                std::cout << (m_isEditorMode ? "Editor mode ON" : "Editor mode OFF") << std::endl;

                // Si on sort de l'éditeur, recharger le niveau prologue
                if (!m_isEditorMode) {
                    std::cout << "Reloading prologue level..." << std::endl;
                    if (m_level->loadFromFile("levels/prologue.json")) {
                        // Repositionner le joueur au centre du portail d'entrée
                        if (m_level->hasEntrancePortal()) {
                            sf::Vector2f portalPos = m_level->getEntrancePortalPosition();
                            // Centrer le joueur sur le portail (portail = 64x64, joueur = 102x102)
                            sf::Vector2f playerStartPos;
                            playerStartPos.x = portalPos.x + 32.0f - 51.0f;  // Centre du portail - moitié largeur joueur
                            playerStartPos.y = portalPos.y + 32.0f - 51.0f;  // Centre du portail - moitié hauteur joueur
                            m_player->setPosition(playerStartPos);
                            m_camera->setPosition(playerStartPos);
                            std::cout << "Player repositioned at entrance portal center" << std::endl;
                        }
                    } else {
                        std::cerr << "Failed to reload prologue level!" << std::endl;
                    }
                }
            }

            // Vérifier si l'éditeur veut quitter le jeu
            if (m_isEditorMode && m_editor->wantsToQuit()) {
                std::cout << "Quitting game from editor..." << std::endl;
                m_window.close();
                return;
            }

            // Gestion du menu pause avec Escape
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                if (m_isEditorMode) {
                    // En mode éditeur, basculer la pause de l'éditeur
                    m_editor->setPaused(!m_editor->isPaused());
                    if (m_editor->isPaused()) {
                        m_pauseMenu->resetAction();
                        std::cout << "Editor paused" << std::endl;
                    } else {
                        std::cout << "Editor resumed" << std::endl;
                    }
                } else {
                    // En mode jeu normal
                    m_isPaused = !m_isPaused;
                    if (m_isPaused) {
                        m_pauseMenu->resetAction();
                        std::cout << "Game paused" << std::endl;
                    } else {
                        std::cout << "Game resumed" << std::endl;
                    }
                }
            }

            // Si le jeu est terminé (game over), gérer le menu de game over
            if (m_isGameOver) {
                handleMenuInput(keyPressed->code);
            }
            // Si le jeu est complété, gérer le menu de victoire finale
            else if (m_isGameComplete) {
                handleMenuInput(keyPressed->code);
            }
            // Passer l'événement à l'éditeur si actif (sauf si l'éditeur est en pause)
            else if (m_isEditorMode && !m_editor->isPaused()) {
                m_editor->handleInput(*event, m_window);
            }
            // Si l'éditeur est en pause, gérer le menu pause
            else if (m_isEditorMode && m_editor->isPaused()) {
                handleMenuInput(keyPressed->code);
            }
            // Si en pause (mode jeu), gérer le menu
            else if (m_isPaused) {
                handleMenuInput(keyPressed->code);
            }
            // Sinon, gérer les inputs du joueur
            else {
                handlePlayerInput(keyPressed->code, true);
            }
        }
        else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>()) {
            if (!m_isPaused && !m_isEditorMode) {
                handlePlayerInput(keyReleased->code, false);
            }
        }
        else {
            // Passer tous les autres événements à l'éditeur (souris, etc.)
            if (m_isEditorMode) {
                m_editor->handleInput(*event, m_window);
            }
        }
    }

    // Vérifier l'action du menu (jeu normal ou éditeur)
    if (m_isPaused || (m_isEditorMode && m_editor->isPaused())) {
        auto action = m_pauseMenu->getSelectedAction();
        if (action == PauseMenu::MenuAction::Continue) {
            if (m_isEditorMode) {
                m_editor->setPaused(false);
                std::cout << "Continue editor" << std::endl;
            } else {
                m_isPaused = false;
                std::cout << "Continue game" << std::endl;
            }
            m_pauseMenu->resetAction();
        }
        else if (action == PauseMenu::MenuAction::Restart) {
            std::cout << "Restarting game (loading prologue)..." << std::endl;

            // Quitter l'éditeur si actif
            if (m_isEditorMode) {
                m_isEditorMode = false;
                m_editor->setActive(false);
                m_editor->setPaused(false);
            }

            // Redémarrer le jeu
            restartGame();

            m_isPaused = false;
            m_pauseMenu->resetAction();
        }
        else if (action == PauseMenu::MenuAction::Quit) {
            std::cout << "Quit game" << std::endl;
            m_window.close();
        }
    }

}

void Game::update(sf::Time deltaTime) {
    // En mode éditeur, mettre à jour l'éditeur
    if (m_isEditorMode) {
        m_editor->update(deltaTime);
        return;
    }

    if (m_isGameComplete) return; // Ne plus rien faire si le jeu est complété
    if (m_isGameOver) return; // Ne rien mettre à jour si le jeu est terminé (game over)
    if (m_isPaused) return; // Ne rien mettre à jour si le jeu est en pause

    // Si le niveau est terminé, gérer la transition
    static sf::Clock transitionClock;
    static bool transitionStarted = false;

    if (m_isFinished) {
        if (!transitionStarted) {
            transitionClock.restart();
            transitionStarted = true;
            std::cout << "Starting 3 second transition timer..." << std::endl;
        }

        if (transitionClock.getElapsedTime().asSeconds() >= 3.0f) {
            std::cout << "3 seconds elapsed, loading next level..." << std::endl;
            transitionStarted = false;
            loadNextLevel();
        }
        // Ne pas mettre à jour le joueur pendant la transition
        return;
    } else {
        // Réinitialiser le flag si le niveau n'est pas terminé
        transitionStarted = false;
    }

    m_player->update(deltaTime);

    // Mettre à jour le niveau
    m_level->update(deltaTime, *m_player);

    // Vérifier si le joueur est tombé trop bas (chute dans le vide)
    // Le niveau fait 20 tiles de haut * 64 pixels = 1280 pixels
    // On considère que le joueur est tombé s'il dépasse cette hauteur
    const float levelHeight = m_level->getHeight() * m_level->getTileSize();
    const float fallThreshold = levelHeight + 200.0f; // 200 pixels de marge

    // Gérer la désintégration et les chutes
    static sf::Clock disintegrationClock;
    static bool disintegrationStarted = false;
    static sf::Vector2f respawnPosition;
    static bool isDeath = false; // true si c'est une mort (0 HP), false si c'est juste une chute

    if (m_player->getPosition().y > fallThreshold && !m_player->isDisintegrating()) {
        // Le joueur est tombé hors du niveau
        std::cout << "Player fell out of bounds! Triggering disintegration..." << std::endl;

        // Perdre 1 point de vie (1/5 de la vie totale)
        m_player->takeDamage(1);

        // Déclencher la désintégration
        m_player->triggerDisintegration();
        disintegrationClock.restart();
        disintegrationStarted = true;

        // Vérifier si le joueur est mort (0 HP)
        if (m_player->isDead()) {
            std::cout << "Player died! Health reached 0." << std::endl;
            isDeath = true;
        } else {
            // Sinon, préparer la position de respawn
            isDeath = false;
            if (m_level->hasEntrancePortal()) {
                sf::Vector2f portalPos = m_level->getEntrancePortalPosition();
                respawnPosition.x = portalPos.x + 32.0f - 51.0f;
                respawnPosition.y = portalPos.y + 32.0f - 51.0f;
            }
        }
    }

    // Si la désintégration est en cours, attendre qu'elle soit terminée
    if (disintegrationStarted && m_player->isDisintegrating()) {
        // Attendre 2 secondes pour laisser l'animation se jouer
        if (disintegrationClock.getElapsedTime().asSeconds() >= 2.0f) {
            std::cout << "Disintegration complete..." << std::endl;
            disintegrationStarted = false;

            if (isDeath) {
                // Mort complète: afficher le menu de game over
                std::cout << "Player is dead! Showing game over menu..." << std::endl;
                m_isGameOver = true;
                m_pauseMenu->resetAction();
            } else {
                // Juste une chute: repositionner le joueur
                std::cout << "Respawning at entrance portal..." << std::endl;
                m_player->resetDisintegration();
                m_player->setPosition(respawnPosition);
                m_player->setVelocity(sf::Vector2f(0.0f, 0.0f));
                m_camera->setPosition(respawnPosition);
            }
        }
    }

    // Vérifier si le joueur a atteint le portail de sortie
    if (m_level->isPlayerAtFinish(*m_player)) {
        m_isFinished = true;
        std::cout << "Level completed!" << std::endl;
    }

    // Mettre à jour la caméra pour suivre le joueur
    m_camera->update(m_player->getPosition(), deltaTime);
}

void Game::render() {
    m_window.clear(sf::Color::Black); // Noir pour mieux voir les tiles

    // Si en mode éditeur, dessiner l'éditeur
    if (m_isEditorMode) {
        m_window.setView(m_window.getDefaultView());
        m_editor->render(m_window);

        // Dessiner le menu pause par-dessus l'éditeur si nécessaire
        if (m_editor->isPaused()) {
            m_pauseMenu->render(m_window);
        }

        m_window.display();
        return;
    }

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

    // Dessiner le joueur
    m_player->render(m_window);

    // Revenir à la vue par défaut pour l'interface utilisateur (menu pause)
    m_window.setView(m_window.getDefaultView());

    // Afficher la barre de vie en haut à gauche (sauf en mode éditeur)
    if (!m_isEditorMode) {
        const float barX = 20.0f;
        const float barY = 20.0f;
        const float barWidth = 200.0f;
        const float barHeight = 15.0f;
        const float borderThickness = 2.0f;

        // Fond de la barre (gris foncé)
        sf::RectangleShape healthBarBg(sf::Vector2f(barWidth, barHeight));
        healthBarBg.setPosition(sf::Vector2f(barX, barY));
        healthBarBg.setFillColor(sf::Color(40, 40, 40));
        healthBarBg.setOutlineColor(sf::Color(200, 200, 200));
        healthBarBg.setOutlineThickness(borderThickness);
        m_window.draw(healthBarBg);

        // Barre de vie (rouge qui devient orange/jaune selon la vie)
        float healthRatio = static_cast<float>(m_player->getHealth()) / static_cast<float>(m_player->getMaxHealth());
        float currentBarWidth = barWidth * healthRatio;

        // Couleur en fonction de la vie restante
        sf::Color healthColor;
        if (healthRatio > 0.6f) {
            healthColor = sf::Color(50, 200, 50); // Vert
        } else if (healthRatio > 0.3f) {
            healthColor = sf::Color(255, 200, 0); // Orange
        } else {
            healthColor = sf::Color(220, 50, 50); // Rouge
        }

        sf::RectangleShape healthBar(sf::Vector2f(currentBarWidth, barHeight));
        healthBar.setPosition(sf::Vector2f(barX, barY));
        healthBar.setFillColor(healthColor);
        m_window.draw(healthBar);

        // Effet de lueur sur la barre
        sf::RectangleShape healthGlow(sf::Vector2f(currentBarWidth, barHeight));
        healthGlow.setPosition(sf::Vector2f(barX, barY));
        healthGlow.setFillColor(sf::Color(255, 255, 255, 50));
        m_window.draw(healthGlow);

        // Texte de la vie (petit, élégant)
        sf::Text healthText(m_font, std::to_string(m_player->getHealth()) + " / " + std::to_string(m_player->getMaxHealth()), 14);
        healthText.setPosition(sf::Vector2f(barX + barWidth + 10.0f, barY - 2.0f));
        healthText.setFillColor(sf::Color::White);
        m_window.draw(healthText);
    }

    // Afficher le titre du jeu si on est au niveau prologue (niveau 0)
    if (m_currentLevelNumber == 0 && !m_isFinished && !m_isGameComplete) {
        // Titre "BoooBee" en gros et en jaune
        sf::Text titleText(m_font, "BoooBee", 140);
        titleText.setFillColor(sf::Color(255, 215, 0)); // Jaune doré
        titleText.setOutlineColor(sf::Color::White);
        titleText.setOutlineThickness(6.0f);
        titleText.setStyle(sf::Text::Bold);

        sf::FloatRect titleBounds = titleText.getLocalBounds();
        titleText.setOrigin(sf::Vector2f(titleBounds.position.x + titleBounds.size.x / 2.0f,
                                          titleBounds.position.y + titleBounds.size.y / 2.0f));
        titleText.setPosition(sf::Vector2f(640.0f, 120.0f)); // En haut au centre

        // Halo lumineux rectangulaire qui prend toute la base du texte (comme si les lettres brillaient)
        float textWidth = titleBounds.size.x;
        float textHeight = titleBounds.size.y;

        // Dessiner plusieurs rectangles avec opacité décroissante pour l'effet de lueur
        for (int i = 6; i >= 1; --i) {
            sf::RectangleShape glow(sf::Vector2f(textWidth + i * 30.0f, textHeight + i * 20.0f));
            glow.setOrigin(sf::Vector2f((textWidth + i * 30.0f) / 2.0f, (textHeight + i * 20.0f) / 2.0f));
            glow.setPosition(sf::Vector2f(640.0f, 120.0f));
            int alpha = 20 - i * 3; // Opacité décroissante
            glow.setFillColor(sf::Color(255, 215, 0, alpha)); // Jaune doré avec transparence
            m_window.draw(glow);
        }

        // Dessiner le titre par-dessus le halo
        m_window.draw(titleText);
    }

    // Afficher le message de victoire si le niveau est terminé
    if (m_isFinished && !m_isGameComplete) {
        showVictoryMessage();
    }

    // Afficher le menu de game over si le joueur est mort
    if (m_isGameOver) {
        showGameOverMenu();
    }

    // Afficher le menu de victoire finale si le jeu est complété
    if (m_isGameComplete) {
        showFinalVictoryMenu();
    }

    // Dessiner le menu pause si le jeu est en pause (au-dessus de tout)
    if (m_isPaused) {
        m_pauseMenu->render(m_window);
    }

    m_window.display();
}

void Game::showVictoryMessage() {
    // Rectangle semi-transparent en arrière-plan
    sf::RectangleShape overlay(sf::Vector2f(1280.0f, 720.0f));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    m_window.draw(overlay);

    // Texte "Niveau Terminé" (SFML 3.0 nécessite la font dans le constructeur)
    sf::Text finishText(m_font, "Niveau Termine !", 120);
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
    // Si le joueur est mort (game over), gérer les inputs du menu de game over
    if (m_isGameOver) {
        if (key == sf::Keyboard::Key::Enter || key == sf::Keyboard::Key::Space) {
            std::cout << "Restarting from game over (Enter pressed)..." << std::endl;
            m_isGameOver = false;
            restartGame();
        } else if (key == sf::Keyboard::Key::Escape) {
            std::cout << "Quitting from game over (Escape pressed)..." << std::endl;
            m_window.close();
        }
    }
    // Si le jeu est complété, gérer les inputs du menu de victoire finale
    else if (m_isGameComplete) {
        if (key == sf::Keyboard::Key::Enter || key == sf::Keyboard::Key::Space) {
            restartGame();
        } else if (key == sf::Keyboard::Key::Escape) {
            m_window.close();
        }
    } else {
        m_pauseMenu->handleInput(key);
    }
}

void Game::loadNextLevel() {
    int nextLevel = m_currentLevelNumber + 1;

    std::cout << "=== loadNextLevel() called ===" << std::endl;
    std::cout << "Current level: " << m_currentLevelNumber << std::endl;
    std::cout << "Trying to load level " << nextLevel << "..." << std::endl;

    // Vérifier si le niveau suivant existe et est valide
    bool isValid = Level::isLevelValid(nextLevel);
    std::cout << "Level " << nextLevel << " is valid: " << (isValid ? "YES" : "NO") << std::endl;

    if (isValid) {
        std::string filename = "levels/level_" + std::to_string(nextLevel) + ".json";
        std::cout << "Loading file: " << filename << std::endl;

        if (m_level->loadFromFile(filename)) {
            std::cout << "File loaded successfully!" << std::endl;
            m_currentLevelNumber = nextLevel;
            m_isFinished = false;

            // Repositionner le joueur au centre du portail d'entrée
            if (m_level->hasEntrancePortal()) {
                sf::Vector2f portalPos = m_level->getEntrancePortalPosition();
                // Centrer le joueur sur le portail (portail = 64x64, joueur = 102x102)
                sf::Vector2f playerStartPos;
                playerStartPos.x = portalPos.x + 32.0f - 51.0f;  // Centre du portail - moitié largeur joueur
                playerStartPos.y = portalPos.y + 32.0f - 51.0f;  // Centre du portail - moitié hauteur joueur
                m_player->setPosition(playerStartPos);
                m_camera->setPosition(playerStartPos);
                std::cout << "Level " << nextLevel << " loaded successfully! Player positioned at portal." << std::endl;
            } else {
                std::cout << "ERROR: No entrance portal found!" << std::endl;
            }
        } else {
            std::cout << "ERROR: Failed to load file " << filename << std::endl;
        }
    } else {
        // Pas de niveau suivant valide, le jeu est terminé
        std::cout << "No more valid levels. Game complete!" << std::endl;
        m_isGameComplete = true;
        m_isFinished = false;
    }
    std::cout << "=== loadNextLevel() finished ===" << std::endl;
}

void Game::restartGame() {
    std::cout << "Restarting game..." << std::endl;

    // Réinitialiser les flags
    m_isFinished = false;
    m_isGameComplete = false;
    m_isGameOver = false;
    m_currentLevelNumber = 0;

    // Réinitialiser la vie et l'état du joueur
    m_player->resetHealth();
    m_player->resetDisintegration();

    // Charger le niveau prologue
    if (m_level->loadFromFile("levels/prologue.json")) {
        if (m_level->hasEntrancePortal()) {
            sf::Vector2f portalPos = m_level->getEntrancePortalPosition();
            // Centrer le joueur sur le portail (portail = 64x64, joueur = 102x102)
            sf::Vector2f playerStartPos;
            playerStartPos.x = portalPos.x + 32.0f - 51.0f;  // Centre du portail - moitié largeur joueur
            playerStartPos.y = portalPos.y + 32.0f - 51.0f;  // Centre du portail - moitié hauteur joueur
            m_player->setPosition(playerStartPos);
            m_player->setVelocity(sf::Vector2f(0.0f, 0.0f));
            m_camera->setPosition(playerStartPos);
            std::cout << "Game restarted successfully!" << std::endl;
        }
    }
}

void Game::showFinalVictoryMenu() {
    // Overlay semi-transparent
    sf::RectangleShape overlay(sf::Vector2f(1280.0f, 720.0f));
    overlay.setFillColor(sf::Color(0, 0, 0, 200));
    m_window.draw(overlay);

    // Titre "FELICITATIONS!"
    sf::Text titleText(m_font, "FELICITATIONS !", 100);
    titleText.setFillColor(sf::Color(255, 215, 0));
    titleText.setOutlineColor(sf::Color::White);
    titleText.setOutlineThickness(5.0f);
    titleText.setStyle(sf::Text::Bold);

    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(sf::Vector2f(titleBounds.position.x + titleBounds.size.x / 2.0f,
                                      titleBounds.position.y + titleBounds.size.y / 2.0f));
    titleText.setPosition(sf::Vector2f(640.0f, 200.0f));
    m_window.draw(titleText);

    // Message "Jeu termine!"
    sf::Text messageText(m_font, "Vous avez termine tous les niveaux !", 40);
    messageText.setFillColor(sf::Color::White);

    sf::FloatRect messageBounds = messageText.getLocalBounds();
    messageText.setOrigin(sf::Vector2f(messageBounds.position.x + messageBounds.size.x / 2.0f,
                                        messageBounds.position.y + messageBounds.size.y / 2.0f));
    messageText.setPosition(sf::Vector2f(640.0f, 350.0f));
    m_window.draw(messageText);

    // Bouton "Recommencer"
    sf::RectangleShape restartButton(sf::Vector2f(300.0f, 60.0f));
    restartButton.setPosition(sf::Vector2f(490.0f, 450.0f));
    restartButton.setFillColor(sf::Color(100, 150, 200));
    restartButton.setOutlineColor(sf::Color::White);
    restartButton.setOutlineThickness(3.0f);
    m_window.draw(restartButton);

    sf::Text restartText(m_font, "Recommencer (ENTREE)", 24);
    restartText.setFillColor(sf::Color::White);
    restartText.setPosition(sf::Vector2f(520.0f, 465.0f));
    m_window.draw(restartText);

    // Bouton "Quitter"
    sf::RectangleShape quitButton(sf::Vector2f(300.0f, 60.0f));
    quitButton.setPosition(sf::Vector2f(490.0f, 540.0f));
    quitButton.setFillColor(sf::Color(150, 50, 50));
    quitButton.setOutlineColor(sf::Color::White);
    quitButton.setOutlineThickness(3.0f);
    m_window.draw(quitButton);

    sf::Text quitText(m_font, "Quitter (ECHAP)", 24);
    quitText.setFillColor(sf::Color::White);
    quitText.setPosition(sf::Vector2f(545.0f, 555.0f));
    m_window.draw(quitText);
}

void Game::showGameOverMenu() {
    // Overlay semi-transparent rouge foncé pour l'ambiance de game over
    sf::RectangleShape overlay(sf::Vector2f(1280.0f, 720.0f));
    overlay.setFillColor(sf::Color(80, 0, 0, 200));
    m_window.draw(overlay);

    // Titre "VOUS ETES MORT !!"
    sf::Text titleText(m_font, "VOUS ETES MORT !!", 100);
    titleText.setFillColor(sf::Color(255, 50, 50));
    titleText.setOutlineColor(sf::Color(150, 0, 0));
    titleText.setOutlineThickness(5.0f);
    titleText.setStyle(sf::Text::Bold);

    sf::FloatRect titleBounds = titleText.getLocalBounds();
    titleText.setOrigin(sf::Vector2f(titleBounds.position.x + titleBounds.size.x / 2.0f,
                                      titleBounds.position.y + titleBounds.size.y / 2.0f));
    titleText.setPosition(sf::Vector2f(640.0f, 200.0f));
    m_window.draw(titleText);

    // Message "Partie terminée"
    sf::Text messageText(m_font, "Partie terminee", 40);
    messageText.setFillColor(sf::Color(200, 200, 200));

    sf::FloatRect messageBounds = messageText.getLocalBounds();
    messageText.setOrigin(sf::Vector2f(messageBounds.position.x + messageBounds.size.x / 2.0f,
                                        messageBounds.position.y + messageBounds.size.y / 2.0f));
    messageText.setPosition(sf::Vector2f(640.0f, 350.0f));
    m_window.draw(messageText);

    // Bouton "Recommencer"
    sf::RectangleShape restartButton(sf::Vector2f(300.0f, 60.0f));
    restartButton.setPosition(sf::Vector2f(490.0f, 450.0f));
    restartButton.setFillColor(sf::Color(100, 150, 200));
    restartButton.setOutlineColor(sf::Color::White);
    restartButton.setOutlineThickness(3.0f);
    m_window.draw(restartButton);

    sf::Text restartText(m_font, "Recommencer (ENTREE)", 24);
    restartText.setFillColor(sf::Color::White);
    restartText.setPosition(sf::Vector2f(520.0f, 465.0f));
    m_window.draw(restartText);

    // Bouton "Quitter"
    sf::RectangleShape quitButton(sf::Vector2f(300.0f, 60.0f));
    quitButton.setPosition(sf::Vector2f(490.0f, 540.0f));
    quitButton.setFillColor(sf::Color(150, 50, 50));
    quitButton.setOutlineColor(sf::Color::White);
    quitButton.setOutlineThickness(3.0f);
    m_window.draw(quitButton);

    sf::Text quitText(m_font, "Quitter (ECHAP)", 24);
    quitText.setFillColor(sf::Color::White);
    quitText.setPosition(sf::Vector2f(545.0f, 555.0f));
    m_window.draw(quitText);
}
