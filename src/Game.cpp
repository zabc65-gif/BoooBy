#include "Game.hpp"
#include <iostream>

const sf::Time Game::TimePerFrame = sf::seconds(1.f / 60.f);

Game::Game()
    : m_window(sf::VideoMode({1280, 720}), "BoooBee - Sheepy Remake", sf::Style::Close)
    , m_characterSelection(std::make_unique<CharacterSelection>())
    , m_player(nullptr)  // Sera créé après la sélection
    , m_pauseMenu(std::make_unique<PauseMenu>())
    , m_camera(std::make_unique<Camera>(1280.0f, 720.0f))
    , m_level(std::make_unique<Level>())
    , m_editor(std::make_unique<LevelEditor>(64))
    , m_isSelectingCharacter(true)
    , m_isPaused(false)
    , m_isFinished(false)
    , m_isEditorMode(false)
    , m_isGameComplete(false)
    , m_isGameOver(false)
    , m_isLevelSelectOpen(false)
    , m_currentLevelNumber(0)  // Commencer au prologue
    , m_selectedLevelInMenu(0)
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

    // Le joueur et la caméra seront initialisés après la sélection de personnage

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
            // Si on est en train de sélectionner le personnage
            if (m_isSelectingCharacter) {
                m_characterSelection->handleInput(keyPressed->code, true);

                // Vérifier si la sélection est terminée
                if (m_characterSelection->isSelectionMade()) {
                    m_isSelectingCharacter = false;
                    CharacterType selectedChar = m_characterSelection->getSelectedCharacter();

                    // Créer le joueur avec le personnage sélectionné
                    m_player = std::make_unique<Player>(selectedChar);

                    // Positionner le joueur au portail d'entrée
                    sf::Vector2f playerStartPos;
                    if (m_level->hasEntrancePortal()) {
                        sf::Vector2f portalPos = m_level->getEntrancePortalPosition();
                        playerStartPos.x = portalPos.x + 32.0f - 51.0f;
                        playerStartPos.y = portalPos.y + 32.0f - 51.0f;
                    } else {
                        playerStartPos = sf::Vector2f(150.0f, 550.0f);
                    }
                    m_player->setPosition(playerStartPos);
                    m_camera->setPosition(m_player->getPosition());
                }
                continue;  // Ne pas traiter les autres touches pendant la sélection
            }

            // Basculer en mode éditeur avec F1
            if (keyPressed->code == sf::Keyboard::Key::F1) {
                m_isEditorMode = !m_isEditorMode;
                m_editor->setActive(m_isEditorMode);

                // Si on sort de l'éditeur, recharger le niveau prologue
                if (!m_isEditorMode) {
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
                        }
                    } else {
                        std::cerr << "Failed to reload prologue level!" << std::endl;
                    }
                }
            }

            // Vérifier si l'éditeur veut quitter le jeu
            if (m_isEditorMode && m_editor->wantsToQuit()) {
                m_window.close();
                return;
            }

            // Cheat code: Ctrl+L pour ouvrir le menu de sélection de niveau (développement)
            if (keyPressed->code == sf::Keyboard::Key::L &&
                (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
                 sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl))) {
                m_isLevelSelectOpen = !m_isLevelSelectOpen;
                if (m_isLevelSelectOpen) {
                    m_selectedLevelInMenu = m_currentLevelNumber;
                }
            }

            // Gestion du menu pause avec Escape
            if (keyPressed->code == sf::Keyboard::Key::Escape) {
                if (m_isEditorMode) {
                    // En mode éditeur, basculer la pause de l'éditeur
                    m_editor->setPaused(!m_editor->isPaused());
                    if (m_editor->isPaused()) {
                        m_pauseMenu->resetAction();
                    }
                } else {
                    // En mode jeu normal
                    m_isPaused = !m_isPaused;
                    if (m_isPaused) {
                        m_pauseMenu->resetAction();
                    }
                }
            }

            // Si le menu de sélection de niveau est ouvert
            if (m_isLevelSelectOpen) {
                if (keyPressed->code == sf::Keyboard::Key::Left || keyPressed->code == sf::Keyboard::Key::Q) {
                    m_selectedLevelInMenu = (m_selectedLevelInMenu - 1 + 11) % 11; // 0-10
                } else if (keyPressed->code == sf::Keyboard::Key::Right || keyPressed->code == sf::Keyboard::Key::D) {
                    m_selectedLevelInMenu = (m_selectedLevelInMenu + 1) % 11; // 0-10
                } else if (keyPressed->code == sf::Keyboard::Key::Enter || keyPressed->code == sf::Keyboard::Key::Space) {
                    loadLevel(m_selectedLevelInMenu);
                    m_isLevelSelectOpen = false;
                }
            }
            // Si le jeu est terminé (game over), gérer le menu de game over
            else if (m_isGameOver) {
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
            // Ne traiter les événements KeyReleased que si le joueur existe et n'est pas en sélection
            if (!m_isSelectingCharacter && !m_isPaused && !m_isEditorMode && m_player) {
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
    // Si on est en train de sélectionner le personnage, ne rien mettre à jour
    if (m_isSelectingCharacter || !m_player) return;

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

    // Gérer la désintégration et les chutes/morts
    static sf::Clock disintegrationClock;
    static bool disintegrationStarted = false;
    static sf::Vector2f respawnPosition;
    static bool isDeath = false; // true si c'est une mort (0 HP), false si c'est juste une chute

    // Vérifier si le joueur est mort (par les ennemis) - sans tomber
    if (m_player->isDead() && !m_player->isDisintegrating() && !disintegrationStarted) {
        std::cout << "Player killed by enemy! Triggering death..." << std::endl;
        m_player->triggerDisintegration();
        disintegrationClock.restart();
        disintegrationStarted = true;
        isDeath = true;
    }

    if (m_player->getPosition().y > fallThreshold && !m_player->isDisintegrating()) {
        // Le joueur est tombé hors du niveau
        std::cout << "Player fell out of bounds! Triggering disintegration..." << std::endl;

        // Perdre 20% de vie par chute
        m_player->takeDamage(20);

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
    // Détecter si le joueur se déplace (vitesse horizontale > seuil)
    sf::Vector2f velocity = m_player->getVelocity();
    bool playerIsMoving = std::abs(velocity.x) > 10.0f;  // Seuil de 10 pixels/s
    m_camera->update(m_player->getPosition(), deltaTime, playerIsMoving);
}

void Game::render() {
    m_window.clear(sf::Color::Black); // Noir pour mieux voir les tiles

    // Si on est en train de sélectionner le personnage
    if (m_isSelectingCharacter) {
        m_characterSelection->render(m_window);
        m_window.display();
        return;
    }

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

    // Afficher "NOUVEAU POUVOIR" si on est au niveau 4 ou 8
    if ((m_currentLevelNumber == 4 || m_currentLevelNumber == 8) && !m_isFinished && !m_isGameComplete) {
        // Titre "NOUVEAU POUVOIR" en jaune (2 fois plus petit que le titre principal)
        sf::Text powerText(m_font, "NOUVEAU POUVOIR", 70);
        powerText.setFillColor(sf::Color(255, 215, 0)); // Jaune doré
        powerText.setOutlineColor(sf::Color::White);
        powerText.setOutlineThickness(3.0f);
        powerText.setStyle(sf::Text::Bold);

        sf::FloatRect powerBounds = powerText.getLocalBounds();
        powerText.setOrigin(sf::Vector2f(powerBounds.position.x + powerBounds.size.x / 2.0f,
                                          powerBounds.position.y + powerBounds.size.y / 2.0f));
        powerText.setPosition(sf::Vector2f(640.0f, 120.0f)); // En haut au centre

        // Halo lumineux rectangulaire
        float textWidth = powerBounds.size.x;
        float textHeight = powerBounds.size.y;

        // Dessiner plusieurs rectangles avec opacité décroissante pour l'effet de lueur
        for (int i = 4; i >= 1; --i) {
            sf::RectangleShape glow(sf::Vector2f(textWidth + i * 15.0f, textHeight + i * 10.0f));
            glow.setOrigin(sf::Vector2f((textWidth + i * 15.0f) / 2.0f, (textHeight + i * 10.0f) / 2.0f));
            glow.setPosition(sf::Vector2f(640.0f, 120.0f));
            int alpha = 20 - i * 3; // Opacité décroissante
            glow.setFillColor(sf::Color(255, 215, 0, alpha)); // Jaune doré avec transparence
            m_window.draw(glow);
        }

        // Dessiner le texte par-dessus le halo
        m_window.draw(powerText);

        // Sous-titre indiquant le pouvoir spécifique
        std::string powerName = (m_currentLevelNumber == 4) ? "Double Saut (Espace)" : "Charge du Heros (Shift Droit)";
        sf::Text subText(m_font, powerName, 30);
        subText.setFillColor(sf::Color(255, 255, 200));
        subText.setOutlineColor(sf::Color(100, 80, 0));
        subText.setOutlineThickness(2.0f);

        sf::FloatRect subBounds = subText.getLocalBounds();
        subText.setOrigin(sf::Vector2f(subBounds.position.x + subBounds.size.x / 2.0f,
                                        subBounds.position.y + subBounds.size.y / 2.0f));
        subText.setPosition(sf::Vector2f(640.0f, 170.0f));
        m_window.draw(subText);
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

    // Afficher le menu de sélection de niveau (cheat code)
    if (m_isLevelSelectOpen) {
        showLevelSelectMenu();
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
            m_isGameOver = false;
            restartGame();
        } else if (key == sf::Keyboard::Key::Escape) {
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

            // Générer les ennemis pour ce niveau (à partir du niveau 5)
            m_level->generateEnemies(nextLevel);

            // Niveau 4 : activer le double saut
            if (nextLevel == 4) {
                m_player->unlockDoubleJump();
                std::cout << "*** LEVEL 4: Double Jump unlocked! ***" << std::endl;
            }

            // Niveau 8 : activer la charge du héros
            if (nextLevel == 8) {
                m_player->unlockHeroCharge();
                std::cout << "*** LEVEL 8: Hero Charge unlocked! ***" << std::endl;
            }

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
    m_player->lockDoubleJump();  // Retour au prologue = pas de pouvoirs

    // Charger le niveau prologue
    if (m_level->loadFromFile("levels/prologue.json")) {
        // Générer les ennemis pour le prologue (aucun car niveau 0)
        m_level->generateEnemies(0);

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

void Game::showLevelSelectMenu() {
    // Fond semi-transparent
    sf::RectangleShape overlay(sf::Vector2f(1280.0f, 720.0f));
    overlay.setFillColor(sf::Color(0, 0, 0, 200));
    m_window.draw(overlay);

    // Titre
    sf::Text title(m_font, "SELECTION DE NIVEAU (DEV)", 48);
    title.setFillColor(sf::Color(255, 215, 0));
    title.setOutlineColor(sf::Color::White);
    title.setOutlineThickness(2.0f);
    title.setStyle(sf::Text::Bold);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setPosition(sf::Vector2f(640.0f - (titleBounds.position.x + titleBounds.size.x) / 2.0f, 80.0f));
    m_window.draw(title);

    // Instructions
    sf::Text instructions(m_font, "<- -> : Naviguer | ENTREE : Charger | CTRL+L : Fermer", 20);
    instructions.setFillColor(sf::Color::White);
    sf::FloatRect instrBounds = instructions.getLocalBounds();
    instructions.setPosition(sf::Vector2f(640.0f - (instrBounds.position.x + instrBounds.size.x) / 2.0f, 640.0f));
    m_window.draw(instructions);

    // Grille de niveaux (3 lignes x 4 colonnes = 12 slots pour 0-10)
    const int cols = 4;
    const int rows = 3;
    const float buttonWidth = 150.0f;
    const float buttonHeight = 80.0f;
    const float spacing = 30.0f;
    const float startX = 640.0f - (cols * buttonWidth + (cols - 1) * spacing) / 2.0f;
    const float startY = 200.0f;

    for (int level = 0; level <= 10; ++level) {
        int row = level / cols;
        int col = level % cols;

        float x = startX + col * (buttonWidth + spacing);
        float y = startY + row * (buttonHeight + spacing);

        // Bouton
        sf::RectangleShape button(sf::Vector2f(buttonWidth, buttonHeight));
        button.setPosition(sf::Vector2f(x, y));

        // Couleur différente pour le niveau sélectionné
        if (level == m_selectedLevelInMenu) {
            button.setFillColor(sf::Color(255, 215, 0, 200));
            button.setOutlineColor(sf::Color::White);
            button.setOutlineThickness(4.0f);
        } else {
            button.setFillColor(sf::Color(70, 70, 100, 200));
            button.setOutlineColor(sf::Color(150, 150, 150));
            button.setOutlineThickness(2.0f);
        }
        m_window.draw(button);

        // Texte du niveau
        std::string levelText = (level == 0) ? "PROLOGUE" : "NIVEAU " + std::to_string(level);
        sf::Text text(m_font, levelText, 18);
        text.setFillColor(sf::Color::White);
        text.setStyle(sf::Text::Bold);
        sf::FloatRect textBounds = text.getLocalBounds();
        text.setPosition(sf::Vector2f(x + buttonWidth / 2.0f - (textBounds.position.x + textBounds.size.x) / 2.0f,
                                       y + buttonHeight / 2.0f - (textBounds.position.y + textBounds.size.y) / 2.0f - 5.0f));
        m_window.draw(text);

        // Indicateur du niveau actuel
        if (level == m_currentLevelNumber) {
            sf::Text currentIndicator(m_font, "(ACTUEL)", 14);
            currentIndicator.setFillColor(sf::Color::Green);
            sf::FloatRect currentBounds = currentIndicator.getLocalBounds();
            currentIndicator.setPosition(sf::Vector2f(x + buttonWidth / 2.0f - (currentBounds.position.x + currentBounds.size.x) / 2.0f,
                                                       y + buttonHeight - 25.0f));
            m_window.draw(currentIndicator);
        }
    }
}

void Game::loadLevel(int levelNumber) {
    // Validation du niveau
    if (!Level::isLevelValid(levelNumber)) {
        std::cout << "Niveau invalide ou inexistant: " << levelNumber << std::endl;
        return;
    }

    // Charger le nouveau niveau dans l'objet existant (pour garder le tileset)
    std::string levelPath = (levelNumber == 0) ? "levels/prologue.json" :
                            "levels/level_" + std::to_string(levelNumber) + ".json";

    if (!m_level->loadFromFile(levelPath)) {
        std::cout << "Erreur lors du chargement du niveau: " << levelNumber << std::endl;
        return;
    }

    m_currentLevelNumber = levelNumber;

    // Générer les ennemis pour ce niveau (à partir du niveau 5)
    m_level->generateEnemies(levelNumber);

    // Réinitialiser le joueur
    m_player->resetHealth();
    m_player->resetDisintegration();

    // Gérer les pouvoirs en fonction du niveau
    if (levelNumber >= 4) {
        m_player->unlockDoubleJump();
    } else {
        m_player->lockDoubleJump();
    }

    if (levelNumber >= 8) {
        m_player->unlockHeroCharge();
        std::cout << "*** Hero Charge unlocked! (level " << levelNumber << ") ***" << std::endl;
    } else {
        m_player->lockHeroCharge();
    }

    // Positionner le joueur au centre du portail d'entrée
    if (m_level->hasEntrancePortal()) {
        sf::Vector2f portalPos = m_level->getEntrancePortalPosition();
        // Centrer le joueur sur le portail (portail = 64x64, joueur = 102x102)
        sf::Vector2f playerStartPos;
        playerStartPos.x = portalPos.x + 32.0f - 51.0f;  // Centre du portail - moitié largeur joueur
        playerStartPos.y = portalPos.y + 32.0f - 51.0f;  // Centre du portail - moitié hauteur joueur
        m_player->setPosition(playerStartPos);
        m_player->setVelocity(sf::Vector2f(0.0f, 0.0f));
        m_camera->setPosition(playerStartPos);
    }

    // Réinitialiser les états du jeu
    m_isFinished = false;
    m_isGameOver = false;
    m_isGameComplete = false;

    std::cout << "Niveau " << levelNumber << " chargé (cheat code)" << std::endl;
}
