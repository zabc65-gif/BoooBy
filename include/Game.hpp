#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <memory>
#include "Player.hpp"
#include "PauseMenu.hpp"
#include "Camera.hpp"
#include "Level.hpp"
#include "LevelEditor.hpp"
#include "CharacterSelection.hpp"

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

    void showVictoryMessage();
    void showFinalVictoryMenu();
    void showGameOverMenu();
    void showLevelSelectMenu();
    void loadNextLevel();
    void restartGame();
    void loadLevel(int levelNumber);

private:
    static const sf::Time TimePerFrame;

    sf::RenderWindow m_window;
    std::unique_ptr<CharacterSelection> m_characterSelection;
    std::unique_ptr<Player> m_player;
    std::unique_ptr<PauseMenu> m_pauseMenu;
    std::unique_ptr<Camera> m_camera;
    std::unique_ptr<Level> m_level;
    std::unique_ptr<LevelEditor> m_editor;

    bool m_isSelectingCharacter;
    bool m_isPaused;
    bool m_isFinished;
    bool m_isEditorMode;
    bool m_isGameComplete;  // Vrai quand tous les niveaux sont terminés
    bool m_isGameOver;      // Vrai quand le joueur est mort (0 HP)
    bool m_isLevelSelectOpen; // Menu de sélection de niveau (cheat code)
    int m_currentLevelNumber;  // 0 = prologue, 1+ = niveaux numérotés
    int m_selectedLevelInMenu; // Niveau sélectionné dans le menu
    sf::Font m_font;

    // Musique
    sf::Music m_backgroundMusic;
};
