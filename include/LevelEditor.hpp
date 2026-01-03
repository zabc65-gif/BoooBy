#pragma once

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "Tilemap.hpp"

class LevelEditor {
public:
    enum class EditorMode {
        Tile,          // Mode placement de tiles
        Enemy,         // Mode placement d'ennemis
        ExitPortal,    // Mode placement de portail de sortie
        EntrancePortal,// Mode placement de portail d'entrée
        Resize,        // Mode redimensionnement
        LevelSelect    // Mode sélection de niveau
    };

    struct EnemyPlacement {
        int x;
        int y;
        int enemyType;  // Type d'ennemi (0 = basique, etc.)
    };

    LevelEditor(int tileSize = 64);

    void handleInput(sf::Event& event, const sf::RenderWindow& window);
    void update(sf::Time deltaTime);
    void render(sf::RenderWindow& window);

    // Gestion du niveau
    void setLevelData(const std::vector<std::vector<int>>& data);
    std::vector<std::vector<int>> getLevelData() const { return m_levelData; }
    std::vector<EnemyPlacement> getEnemies() const { return m_enemies; }

    // Portails
    bool hasExitPortal() const { return m_hasExitPortal; }
    sf::Vector2i getExitPortalPosition() const { return m_exitPortalPosition; }
    bool hasEntrancePortal() const { return m_hasEntrancePortal; }
    sf::Vector2i getEntrancePortalPosition() const { return m_entrancePortalPosition; }

    // Sauvegarde/chargement
    bool saveToFile(const std::string& filepath);
    bool loadFromFile(const std::string& filepath);

    // Getters
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    bool isActive() const { return m_isActive; }
    void setActive(bool active) { m_isActive = active; }
    bool hasUnsavedChanges() const { return m_hasUnsavedChanges; }
    bool wantsToQuit() const { return m_wantsToQuit; }
    bool wantsToRestart() const { return m_wantsToRestart; }
    bool isPaused() const { return m_isPaused; }
    void setPaused(bool paused) { m_isPaused = paused; }

    // Gestion des niveaux
    void newLevel();
    void loadLevel(int levelNumber);
    void saveCurrentLevel();
    void startLevelSelection();
    void cancelLevelSelection();

private:
    void handleMouseInput(sf::Event& event, const sf::RenderWindow& window);
    void handleKeyboardInput(sf::Event& event);
    void placeTile(int x, int y);
    void eraseTile(int x, int y);
    void placeEnemy(int x, int y);
    void removeEnemy(int x, int y);
    bool canPlaceEnemy(int x, int y) const;
    void placeExitPortal(int x, int y);
    void removeExitPortal();
    void placeEntrancePortal(int x, int y);
    void removeEntrancePortal();
    bool canPlacePortal(int x, int y) const;
    void resizeLevel(int newWidth, int newHeight);
    void renderGrid(sf::RenderWindow& window);
    void renderTilePalette(sf::RenderWindow& window);
    void renderUI(sf::RenderWindow& window);
    void renderEnemies(sf::RenderWindow& window);
    void renderPortals(sf::RenderWindow& window);
    void renderLevelSelector(sf::RenderWindow& window);
    void markAsModified();

private:
    int m_tileSize;
    int m_width;
    int m_height;
    std::vector<std::vector<int>> m_levelData;
    std::vector<EnemyPlacement> m_enemies;

    // Portails
    bool m_hasExitPortal;
    sf::Vector2i m_exitPortalPosition;
    bool m_hasEntrancePortal;
    sf::Vector2i m_entrancePortalPosition;

    // État de l'éditeur
    bool m_isActive;
    bool m_hasUnsavedChanges;
    EditorMode m_mode;
    int m_selectedTile;
    int m_selectedEnemyType;
    bool m_showGrid;
    bool m_showPalette;
    int m_currentLevelNumber;  // Numéro du niveau en cours d'édition
    std::string m_levelInputBuffer;  // Buffer pour la saisie du numéro de niveau
    EditorMode m_previousMode;  // Mode précédent avant la sélection de niveau
    bool m_wantsToQuit;  // Flag pour indiquer que l'utilisateur veut quitter le jeu
    bool m_wantsToRestart;  // Flag pour indiquer que l'utilisateur veut redémarrer
    bool m_isPaused;  // Flag pour indiquer que l'éditeur est en pause

    // Interface
    sf::Font m_font;
    std::shared_ptr<sf::Texture> m_tileset;
    int m_tilesetWidthInTiles;

    // Constantes de la palette
    static constexpr int PALETTE_WIDTH = 200;
    static constexpr int PALETTE_TILE_SIZE = 32;
    static constexpr int MAX_TILES_DISPLAY = 20;
};
