#include "LevelEditor.hpp"
#include "TileProperties.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

LevelEditor::LevelEditor(int tileSize)
    : m_tileSize(tileSize)
    , m_width(30)
    , m_height(20)
    , m_hasExitPortal(false)
    , m_exitPortalPosition(0, 0)
    , m_hasEntrancePortal(false)
    , m_entrancePortalPosition(0, 0)
    , m_isActive(false)
    , m_hasUnsavedChanges(false)
    , m_mode(EditorMode::Tile)
    , m_selectedTile(1)
    , m_selectedEnemyType(0)
    , m_showGrid(true)
    , m_showPalette(true)
    , m_currentLevelNumber(1)
    , m_levelInputBuffer("")
    , m_previousMode(EditorMode::Tile)
    , m_wantsToQuit(false)
    , m_wantsToRestart(false)
    , m_isPaused(false)
    , m_tilesetWidthInTiles(14)
{
    // Initialiser le niveau vide
    m_levelData.resize(m_height, std::vector<int>(m_width, -1));

    // Charger le tileset
    m_tileset = std::make_shared<sf::Texture>();
    if (!m_tileset->loadFromFile("assets/tiles/Mossy Tileset/Mossy - TileSet.png")) {
        std::cerr << "Failed to load tileset for editor" << std::endl;
    }
    m_tileset->setSmooth(false);

    // Charger la police
    if (!m_font.openFromFile("assets/Arial.ttf")) {
        std::cerr << "Failed to load font for editor" << std::endl;
    }

    std::cout << "Level Editor initialized (" << m_width << "x" << m_height << ")" << std::endl;
}

void LevelEditor::handleInput(sf::Event& event, const sf::RenderWindow& window) {
    if (!m_isActive) return;

    handleMouseInput(event, window);
    handleKeyboardInput(event);
}

void LevelEditor::handleMouseInput(sf::Event& event, const sf::RenderWindow& window) {
    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        sf::Vector2i mousePos = sf::Mouse::getPosition(window);

        // Convertir en coordonnées de grille
        int tileX = mousePos.x / m_tileSize;
        int tileY = mousePos.y / m_tileSize;

        // Vérifier si on clique dans la palette
        if (m_showPalette && mousePos.x >= window.getSize().x - PALETTE_WIDTH) {
            // Clic dans la palette
            int paletteX = (mousePos.x - (window.getSize().x - PALETTE_WIDTH)) / PALETTE_TILE_SIZE;
            int paletteY = mousePos.y / PALETTE_TILE_SIZE;
            int selectedTile = paletteY * (PALETTE_WIDTH / PALETTE_TILE_SIZE) + paletteX;

            if (selectedTile < MAX_TILES_DISPLAY) {
                m_selectedTile = selectedTile;
                std::cout << "Selected tile: " << m_selectedTile << std::endl;
            }
            return;
        }

        // Clic dans le niveau
        if (tileX >= 0 && tileX < m_width && tileY >= 0 && tileY < m_height) {
            if (mousePressed->button == sf::Mouse::Button::Left) {
                if (m_mode == EditorMode::Tile) {
                    placeTile(tileX, tileY);
                } else if (m_mode == EditorMode::Enemy) {
                    placeEnemy(tileX, tileY);
                } else if (m_mode == EditorMode::ExitPortal) {
                    placeExitPortal(tileX, tileY);
                } else if (m_mode == EditorMode::EntrancePortal) {
                    placeEntrancePortal(tileX, tileY);
                }
            } else if (mousePressed->button == sf::Mouse::Button::Right) {
                if (m_mode == EditorMode::Tile) {
                    eraseTile(tileX, tileY);
                } else if (m_mode == EditorMode::Enemy) {
                    removeEnemy(tileX, tileY);
                } else if (m_mode == EditorMode::ExitPortal) {
                    removeExitPortal();
                } else if (m_mode == EditorMode::EntrancePortal) {
                    removeEntrancePortal();
                }
            }
        }
    }
}

void LevelEditor::handleKeyboardInput(sf::Event& event) {
    // Gestion de la saisie de texte pour le sélecteur de niveau
    if (m_mode == EditorMode::LevelSelect) {
        if (const auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
            if (textEntered->unicode < 128) {
                char c = static_cast<char>(textEntered->unicode);

                // Backspace
                if (c == 8 && !m_levelInputBuffer.empty()) {
                    m_levelInputBuffer.pop_back();
                }
                // Chiffres seulement
                else if (c >= '0' && c <= '9' && m_levelInputBuffer.length() < 2) {
                    m_levelInputBuffer += c;
                }
            }
        }
    }

    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        // Gestion spéciale pour le mode sélection de niveau
        if (m_mode == EditorMode::LevelSelect) {
            if (keyPressed->code == sf::Keyboard::Key::Enter) {
                // Valider la sélection
                if (!m_levelInputBuffer.empty()) {
                    int levelNumber = std::stoi(m_levelInputBuffer);
                    if (levelNumber >= 1 && levelNumber <= 99) {
                        loadLevel(levelNumber);
                        m_mode = m_previousMode;
                    }
                }
                m_levelInputBuffer.clear();
            }
            else if (keyPressed->code == sf::Keyboard::Key::Escape) {
                // Annuler
                cancelLevelSelection();
            }
            return;  // Ignorer les autres touches en mode sélection
        }
        switch (keyPressed->code) {
            case sf::Keyboard::Key::T:
                m_mode = EditorMode::Tile;
                std::cout << "Mode: Tile placement" << std::endl;
                break;

            case sf::Keyboard::Key::E:
                m_mode = EditorMode::Enemy;
                std::cout << "Mode: Enemy placement" << std::endl;
                break;

            case sf::Keyboard::Key::O:
                m_mode = EditorMode::ExitPortal;
                std::cout << "Mode: Exit Portal placement" << std::endl;
                break;

            case sf::Keyboard::Key::I:
                m_mode = EditorMode::EntrancePortal;
                std::cout << "Mode: Entrance Portal placement" << std::endl;
                break;

            case sf::Keyboard::Key::R:
                m_mode = EditorMode::Resize;
                std::cout << "Mode: Resize level" << std::endl;
                break;

            case sf::Keyboard::Key::Escape:
                m_isPaused = !m_isPaused;
                if (m_isPaused) {
                    std::cout << "Editor paused" << std::endl;
                } else {
                    std::cout << "Editor resumed" << std::endl;
                }
                break;

            case sf::Keyboard::Key::Q:
                if (keyPressed->control) {
                    std::cout << "Quit request from editor (Ctrl+Q)" << std::endl;
                    m_wantsToQuit = true;
                    m_isActive = false;
                }
                break;

            case sf::Keyboard::Key::N:
                if (keyPressed->control) {
                    newLevel();
                }
                break;

            case sf::Keyboard::Key::G:
                m_showGrid = !m_showGrid;
                break;

            case sf::Keyboard::Key::P:
                m_showPalette = !m_showPalette;
                break;

            // Redimensionnement du niveau
            case sf::Keyboard::Key::Up:
                if (m_mode == EditorMode::Resize) {
                    resizeLevel(m_width, m_height + 1);
                }
                break;

            case sf::Keyboard::Key::Down:
                if (m_mode == EditorMode::Resize && m_height > 5) {
                    resizeLevel(m_width, m_height - 1);
                }
                break;

            case sf::Keyboard::Key::Right:
                if (m_mode == EditorMode::Resize) {
                    resizeLevel(m_width + 1, m_height);
                }
                break;

            case sf::Keyboard::Key::Left:
                if (m_mode == EditorMode::Resize && m_width > 5) {
                    resizeLevel(m_width - 1, m_height);
                }
                break;

            // Sauvegarde/Chargement
            case sf::Keyboard::Key::S:
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
                    saveCurrentLevel();
                }
                break;

            case sf::Keyboard::Key::L:
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl)) {
                    // Ctrl+L pour changer de niveau à éditer
                    startLevelSelection();
                }
                break;

            default:
                break;
        }
    }
}

void LevelEditor::placeTile(int x, int y) {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        m_levelData[y][x] = m_selectedTile;
        markAsModified();
        std::cout << "Placed tile " << m_selectedTile << " at (" << x << "," << y << ")" << std::endl;
    }
}

void LevelEditor::eraseTile(int x, int y) {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        m_levelData[y][x] = -1;
        markAsModified();
        std::cout << "Erased tile at (" << x << "," << y << ")" << std::endl;
    }
}

void LevelEditor::placeEnemy(int x, int y) {
    if (!canPlaceEnemy(x, y)) {
        std::cout << "Cannot place enemy at (" << x << "," << y << ") - invalid location" << std::endl;
        return;
    }

    // Vérifier si un ennemi existe déjà à cet emplacement
    for (const auto& enemy : m_enemies) {
        if (enemy.x == x && enemy.y == y) {
            std::cout << "Enemy already exists at this location" << std::endl;
            return;
        }
    }

    // Ajouter l'ennemi
    m_enemies.push_back({x, y, m_selectedEnemyType});
    markAsModified();
    std::cout << "Placed enemy type " << m_selectedEnemyType << " at (" << x << "," << y << ")" << std::endl;
}

void LevelEditor::removeEnemy(int x, int y) {
    auto it = std::remove_if(m_enemies.begin(), m_enemies.end(),
        [x, y](const EnemyPlacement& enemy) {
            return enemy.x == x && enemy.y == y;
        });

    if (it != m_enemies.end()) {
        m_enemies.erase(it, m_enemies.end());
        markAsModified();
        std::cout << "Removed enemy at (" << x << "," << y << ")" << std::endl;
    }
}

bool LevelEditor::canPlaceEnemy(int x, int y) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return false;
    }

    int tileId = m_levelData[y][x];

    // Ne peut pas placer sur une tile vide (l'ennemi tomberait)
    if (tileId == -1) {
        // Vérifier s'il y a un sol en dessous
        if (y + 1 >= m_height) return false;
        int tileBelowId = m_levelData[y + 1][x];
        if (tileBelowId == -1) return false;

        // Vérifier que la tile en dessous n'est pas un mur
        CollisionType belowType = TilePropertiesManager::getInstance().getCollisionType(tileBelowId);
        if (belowType == CollisionType::WALL_LEFT || belowType == CollisionType::WALL_RIGHT) {
            return false;
        }

        return true;  // OK si c'est de l'air avec un sol en dessous
    }

    // Ne peut pas placer dans un mur, sol ou plafond
    CollisionType tileType = TilePropertiesManager::getInstance().getCollisionType(tileId);

    if (tileType == CollisionType::WALL_LEFT ||
        tileType == CollisionType::WALL_RIGHT ||
        tileType == CollisionType::SOLID ||
        tileType == CollisionType::PLATFORM ||
        tileType == CollisionType::CEILING ||
        tileType == CollisionType::HALF_BLOCK) {
        return false;
    }

    return true;
}

void LevelEditor::placeExitPortal(int x, int y) {
    if (!canPlacePortal(x, y)) {
        std::cout << "Cannot place exit portal at (" << x << "," << y << ")" << std::endl;
        return;
    }

    m_hasExitPortal = true;
    m_exitPortalPosition = sf::Vector2i(x, y);
    markAsModified();
    std::cout << "Exit portal placed at (" << x << "," << y << ")" << std::endl;
}

void LevelEditor::removeExitPortal() {
    if (m_hasExitPortal) {
        m_hasExitPortal = false;
        markAsModified();
        std::cout << "Exit portal removed" << std::endl;
    }
}

void LevelEditor::placeEntrancePortal(int x, int y) {
    if (!canPlacePortal(x, y)) {
        std::cout << "Cannot place entrance portal at (" << x << "," << y << ")" << std::endl;
        return;
    }

    m_hasEntrancePortal = true;
    m_entrancePortalPosition = sf::Vector2i(x, y);
    markAsModified();
    std::cout << "Entrance portal placed at (" << x << "," << y << ")" << std::endl;
}

void LevelEditor::removeEntrancePortal() {
    if (m_hasEntrancePortal) {
        m_hasEntrancePortal = false;
        markAsModified();
        std::cout << "Entrance portal removed" << std::endl;
    }
}

bool LevelEditor::canPlacePortal(int x, int y) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return false;
    }

    int tileId = m_levelData[y][x];

    // Le portail peut être placé sur une tile vide (il sera au-dessus du sol)
    if (tileId == -1) {
        // Vérifier s'il y a un sol en dessous pour que le portail soit posé
        if (y + 1 >= m_height) return false;
        int tileBelowId = m_levelData[y + 1][x];
        if (tileBelowId == -1) return false;

        // Vérifier que la tile en dessous n'est pas un mur vertical
        CollisionType belowType = TilePropertiesManager::getInstance().getCollisionType(tileBelowId);
        if (belowType == CollisionType::WALL_LEFT || belowType == CollisionType::WALL_RIGHT) {
            return false;
        }

        return true;  // OK si c'est de l'air avec un sol solide en dessous
    }

    // Ne peut pas placer dans un mur, sol ou plafond
    CollisionType tileType = TilePropertiesManager::getInstance().getCollisionType(tileId);

    if (tileType == CollisionType::WALL_LEFT ||
        tileType == CollisionType::WALL_RIGHT ||
        tileType == CollisionType::SOLID ||
        tileType == CollisionType::PLATFORM ||
        tileType == CollisionType::CEILING ||
        tileType == CollisionType::HALF_BLOCK) {
        return false;
    }

    return true;
}

void LevelEditor::resizeLevel(int newWidth, int newHeight) {
    std::cout << "Resizing level from " << m_width << "x" << m_height
              << " to " << newWidth << "x" << newHeight << std::endl;

    std::vector<std::vector<int>> newData(newHeight, std::vector<int>(newWidth, -1));

    // Copier les données existantes
    for (int y = 0; y < std::min(m_height, newHeight); ++y) {
        for (int x = 0; x < std::min(m_width, newWidth); ++x) {
            newData[y][x] = m_levelData[y][x];
        }
    }

    m_levelData = newData;
    m_width = newWidth;
    m_height = newHeight;

    // Supprimer les ennemis hors limites
    auto it = std::remove_if(m_enemies.begin(), m_enemies.end(),
        [this](const EnemyPlacement& enemy) {
            return enemy.x >= m_width || enemy.y >= m_height;
        });
    m_enemies.erase(it, m_enemies.end());

    // Supprimer les portails s'ils sont hors limites
    if (m_hasExitPortal && (m_exitPortalPosition.x >= m_width || m_exitPortalPosition.y >= m_height)) {
        m_hasExitPortal = false;
        std::cout << "Exit portal removed (out of bounds after resize)" << std::endl;
    }
    if (m_hasEntrancePortal && (m_entrancePortalPosition.x >= m_width || m_entrancePortalPosition.y >= m_height)) {
        m_hasEntrancePortal = false;
        std::cout << "Entrance portal removed (out of bounds after resize)" << std::endl;
    }

    markAsModified();
}

void LevelEditor::update(sf::Time deltaTime) {
    // L'éditeur n'a pas besoin de mise à jour pour l'instant
}

void LevelEditor::render(sf::RenderWindow& window) {
    if (!m_isActive) return;

    // Dessiner le niveau avec les tiles
    const int sectionSize = 256;
    const float scale = 0.25f;
    const float displaySize = sectionSize * scale;

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            int tileNumber = m_levelData[y][x];
            if (tileNumber < 0) continue;

            int sectionX = tileNumber % m_tilesetWidthInTiles;
            int sectionY = tileNumber / m_tilesetWidthInTiles;

            sf::Sprite tileSprite(*m_tileset);
            tileSprite.setTextureRect(sf::IntRect(
                sf::Vector2i(sectionX * sectionSize, sectionY * sectionSize),
                sf::Vector2i(sectionSize, sectionSize)
            ));
            tileSprite.setPosition(sf::Vector2f(x * displaySize, y * displaySize));
            tileSprite.setScale(sf::Vector2f(scale, scale));
            window.draw(tileSprite);
        }
    }

    // Dessiner la grille
    if (m_showGrid) {
        renderGrid(window);
    }

    // Dessiner les ennemis
    renderEnemies(window);

    // Dessiner les portails
    renderPortals(window);

    // Dessiner la palette
    if (m_showPalette) {
        renderTilePalette(window);
    }

    // Dessiner l'UI
    renderUI(window);

    // Dessiner le sélecteur de niveau par-dessus tout
    if (m_mode == EditorMode::LevelSelect) {
        renderLevelSelector(window);
    }
}

void LevelEditor::renderGrid(sf::RenderWindow& window) {
    sf::RectangleShape gridLine;
    gridLine.setFillColor(sf::Color(255, 255, 255, 30));

    // Lignes verticales
    for (int x = 0; x <= m_width; ++x) {
        gridLine.setSize(sf::Vector2f(1, m_height * m_tileSize));
        gridLine.setPosition(sf::Vector2f(x * m_tileSize, 0));
        window.draw(gridLine);
    }

    // Lignes horizontales
    for (int y = 0; y <= m_height; ++y) {
        gridLine.setSize(sf::Vector2f(m_width * m_tileSize, 1));
        gridLine.setPosition(sf::Vector2f(0, y * m_tileSize));
        window.draw(gridLine);
    }
}

void LevelEditor::renderTilePalette(sf::RenderWindow& window) {
    // Fond de la palette
    sf::RectangleShape paletteBg(sf::Vector2f(PALETTE_WIDTH, window.getSize().y));
    paletteBg.setPosition(sf::Vector2f(window.getSize().x - PALETTE_WIDTH, 0));
    paletteBg.setFillColor(sf::Color(40, 40, 50, 200));
    window.draw(paletteBg);

    // Dessiner les tiles dans la palette
    int tilesPerRow = PALETTE_WIDTH / PALETTE_TILE_SIZE;
    const int sectionSize = 256;
    const float scale = PALETTE_TILE_SIZE / static_cast<float>(sectionSize);

    for (int i = 0; i < MAX_TILES_DISPLAY; ++i) {
        int px = i % tilesPerRow;
        int py = i / tilesPerRow;

        int sectionX = i % m_tilesetWidthInTiles;
        int sectionY = i / m_tilesetWidthInTiles;

        sf::Sprite tileSprite(*m_tileset);
        tileSprite.setTextureRect(sf::IntRect(
            sf::Vector2i(sectionX * sectionSize, sectionY * sectionSize),
            sf::Vector2i(sectionSize, sectionSize)
        ));
        tileSprite.setPosition(sf::Vector2f(
            window.getSize().x - PALETTE_WIDTH + px * PALETTE_TILE_SIZE,
            py * PALETTE_TILE_SIZE
        ));
        tileSprite.setScale(sf::Vector2f(scale, scale));
        window.draw(tileSprite);

        // Highlight si sélectionné
        if (i == m_selectedTile) {
            sf::RectangleShape highlight(sf::Vector2f(PALETTE_TILE_SIZE, PALETTE_TILE_SIZE));
            highlight.setPosition(tileSprite.getPosition());
            highlight.setFillColor(sf::Color::Transparent);
            highlight.setOutlineColor(sf::Color::Yellow);
            highlight.setOutlineThickness(2.0f);
            window.draw(highlight);
        }
    }
}

void LevelEditor::renderEnemies(sf::RenderWindow& window) {
    for (const auto& enemy : m_enemies) {
        sf::CircleShape enemyMarker(m_tileSize / 3.0f);
        enemyMarker.setPosition(sf::Vector2f(
            enemy.x * m_tileSize + m_tileSize / 6.0f,
            enemy.y * m_tileSize + m_tileSize / 6.0f
        ));
        enemyMarker.setFillColor(sf::Color(255, 0, 0, 150));
        enemyMarker.setOutlineColor(sf::Color::Red);
        enemyMarker.setOutlineThickness(2.0f);
        window.draw(enemyMarker);
    }
}

void LevelEditor::renderPortals(sf::RenderWindow& window) {
    // Dessiner le portail de sortie (cyan/bleu)
    if (m_hasExitPortal) {
        float centerX = m_exitPortalPosition.x * m_tileSize + m_tileSize / 2.0f;
        float centerY = m_exitPortalPosition.y * m_tileSize + m_tileSize / 2.0f;

        // Cercle extérieur (lueur cyan)
        sf::CircleShape outerGlow(m_tileSize * 0.6f);
        outerGlow.setOrigin(sf::Vector2f(m_tileSize * 0.6f, m_tileSize * 0.6f));
        outerGlow.setPosition(sf::Vector2f(centerX, centerY));
        outerGlow.setFillColor(sf::Color(0, 255, 255, 50));
        outerGlow.setOutlineColor(sf::Color(0, 200, 255, 100));
        outerGlow.setOutlineThickness(3.0f);
        window.draw(outerGlow);

        sf::CircleShape middleRing(m_tileSize * 0.4f);
        middleRing.setOrigin(sf::Vector2f(m_tileSize * 0.4f, m_tileSize * 0.4f));
        middleRing.setPosition(sf::Vector2f(centerX, centerY));
        middleRing.setFillColor(sf::Color(0, 150, 255, 100));
        middleRing.setOutlineColor(sf::Color(0, 255, 255, 200));
        middleRing.setOutlineThickness(2.0f);
        window.draw(middleRing);

        sf::CircleShape innerCore(m_tileSize * 0.2f);
        innerCore.setOrigin(sf::Vector2f(m_tileSize * 0.2f, m_tileSize * 0.2f));
        innerCore.setPosition(sf::Vector2f(centerX, centerY));
        innerCore.setFillColor(sf::Color(255, 255, 255, 200));
        innerCore.setOutlineColor(sf::Color(0, 255, 255));
        innerCore.setOutlineThickness(1.0f);
        window.draw(innerCore);

        sf::Text portalText(m_font, "EXIT", 12);
        portalText.setPosition(sf::Vector2f(centerX - 15.0f, centerY - m_tileSize * 0.8f));
        portalText.setFillColor(sf::Color::Cyan);
        portalText.setOutlineColor(sf::Color::Black);
        portalText.setOutlineThickness(1.0f);
        window.draw(portalText);
    }

    // Dessiner le portail d'entrée (vert/jaune)
    if (m_hasEntrancePortal) {
        float centerX = m_entrancePortalPosition.x * m_tileSize + m_tileSize / 2.0f;
        float centerY = m_entrancePortalPosition.y * m_tileSize + m_tileSize / 2.0f;

        // Cercle extérieur (lueur verte)
        sf::CircleShape outerGlow(m_tileSize * 0.6f);
        outerGlow.setOrigin(sf::Vector2f(m_tileSize * 0.6f, m_tileSize * 0.6f));
        outerGlow.setPosition(sf::Vector2f(centerX, centerY));
        outerGlow.setFillColor(sf::Color(0, 255, 0, 50));
        outerGlow.setOutlineColor(sf::Color(100, 255, 0, 100));
        outerGlow.setOutlineThickness(3.0f);
        window.draw(outerGlow);

        sf::CircleShape middleRing(m_tileSize * 0.4f);
        middleRing.setOrigin(sf::Vector2f(m_tileSize * 0.4f, m_tileSize * 0.4f));
        middleRing.setPosition(sf::Vector2f(centerX, centerY));
        middleRing.setFillColor(sf::Color(150, 255, 0, 100));
        middleRing.setOutlineColor(sf::Color(0, 255, 0, 200));
        middleRing.setOutlineThickness(2.0f);
        window.draw(middleRing);

        sf::CircleShape innerCore(m_tileSize * 0.2f);
        innerCore.setOrigin(sf::Vector2f(m_tileSize * 0.2f, m_tileSize * 0.2f));
        innerCore.setPosition(sf::Vector2f(centerX, centerY));
        innerCore.setFillColor(sf::Color(255, 255, 255, 200));
        innerCore.setOutlineColor(sf::Color(0, 255, 0));
        innerCore.setOutlineThickness(1.0f);
        window.draw(innerCore);

        sf::Text portalText(m_font, "START", 12);
        portalText.setPosition(sf::Vector2f(centerX - 18.0f, centerY - m_tileSize * 0.8f));
        portalText.setFillColor(sf::Color::Green);
        portalText.setOutlineColor(sf::Color::Black);
        portalText.setOutlineThickness(1.0f);
        window.draw(portalText);
    }
}

void LevelEditor::renderUI(sf::RenderWindow& window) {
    // Afficher le mode actuel
    std::string modeText = "Mode: ";
    switch (m_mode) {
        case EditorMode::Tile: modeText += "Tile"; break;
        case EditorMode::Enemy: modeText += "Enemy"; break;
        case EditorMode::ExitPortal: modeText += "Exit Portal"; break;
        case EditorMode::EntrancePortal: modeText += "Entrance Portal"; break;
        case EditorMode::Resize: modeText += "Resize"; break;
        case EditorMode::LevelSelect: modeText += "Level Select"; break;
    }
    if (m_hasUnsavedChanges) {
        modeText += " *";  // Indiquer les modifications non sauvegardées
    }

    sf::Text text(m_font, modeText, 20);
    text.setPosition(sf::Vector2f(10, 10));
    text.setFillColor(sf::Color::White);
    window.draw(text);

    // Afficher le numéro du niveau
    std::string levelText = "Level: " + std::to_string(m_currentLevelNumber);
    sf::Text levelDisplay(m_font, levelText, 20);
    levelDisplay.setPosition(sf::Vector2f(10, 40));
    levelDisplay.setFillColor(sf::Color::Yellow);
    window.draw(levelDisplay);

    // Afficher les dimensions
    std::string sizeText = "Size: " + std::to_string(m_width) + "x" + std::to_string(m_height);
    sf::Text sizeDisplay(m_font, sizeText, 20);
    sizeDisplay.setPosition(sf::Vector2f(10, 70));
    sizeDisplay.setFillColor(sf::Color::White);
    window.draw(sizeDisplay);

    // Afficher les contrôles
    std::string controls = "T=Tile E=Enemy O=ExitPortal I=EntrPortal R=Resize | Esc=Editor Ctrl+Q=Quit | Ctrl+N=New Ctrl+S=Save Ctrl+L=Load";
    sf::Text controlsText(m_font, controls, 14);
    controlsText.setPosition(sf::Vector2f(10, window.getSize().y - 30));
    controlsText.setFillColor(sf::Color(200, 200, 200));
    window.draw(controlsText);
}

void LevelEditor::setLevelData(const std::vector<std::vector<int>>& data) {
    m_levelData = data;
    m_height = data.size();
    m_width = data.empty() ? 0 : data[0].size();
}

bool LevelEditor::saveToFile(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for saving: " << filepath << std::endl;
        return false;
    }

    file << "{\n";
    file << "  \"width\": " << m_width << ",\n";
    file << "  \"height\": " << m_height << ",\n";
    file << "  \"tiles\": [\n";

    for (int y = 0; y < m_height; ++y) {
        file << "    [";
        for (int x = 0; x < m_width; ++x) {
            file << m_levelData[y][x];
            if (x < m_width - 1) file << ", ";
        }
        file << "]";
        if (y < m_height - 1) file << ",";
        file << "\n";
    }

    file << "  ],\n";
    file << "  \"enemies\": [\n";

    for (size_t i = 0; i < m_enemies.size(); ++i) {
        const auto& enemy = m_enemies[i];
        file << "    {\"x\": " << enemy.x << ", \"y\": " << enemy.y
             << ", \"type\": " << enemy.enemyType << "}";
        if (i < m_enemies.size() - 1) file << ",";
        file << "\n";
    }

    file << "  ],\n";
    file << "  \"exitPortal\": ";
    if (m_hasExitPortal) {
        file << "{\"x\": " << m_exitPortalPosition.x << ", \"y\": " << m_exitPortalPosition.y << "},\n";
    } else {
        file << "null,\n";
    }
    file << "  \"entrancePortal\": ";
    if (m_hasEntrancePortal) {
        file << "{\"x\": " << m_entrancePortalPosition.x << ", \"y\": " << m_entrancePortalPosition.y << "}\n";
    } else {
        file << "null\n";
    }
    file << "}\n";

    file.close();
    m_hasUnsavedChanges = false;  // Marquer comme sauvegardé
    std::cout << "Level saved to " << filepath << std::endl;
    return true;
}

bool LevelEditor::loadFromFile(const std::string& filepath) {
    std::cout << "Loading level from: " << filepath << std::endl;

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filepath << std::endl;
        return false;
    }

    // Parser le JSON simple ligne par ligne
    std::string line;
    std::vector<std::vector<int>> levelData;
    std::vector<EnemyPlacement> enemies;
    int width = 0, height = 0;
    bool inTiles = false;
    bool hasExitPortal = false;
    sf::Vector2i exitPortalPos(0, 0);
    bool hasEntrancePortal = false;
    sf::Vector2i entrancePortalPos(0, 0);

    while (std::getline(file, line)) {
        // Enlever les espaces
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

        if (line.find("\"width\":") != std::string::npos) {
            size_t pos = line.find(":");
            width = std::stoi(line.substr(pos + 1, line.find(",") - pos - 1));
        }
        else if (line.find("\"height\":") != std::string::npos) {
            size_t pos = line.find(":");
            height = std::stoi(line.substr(pos + 1, line.find(",") - pos - 1));
        }
        else if (line.find("\"tiles\":") != std::string::npos) {
            inTiles = true;
        }
        else if (inTiles && line.find("[") == 0) {
            // C'est une ligne de tiles
            std::vector<int> row;
            std::string content = line.substr(1);
            size_t endPos = content.find_last_of("]");
            if (endPos != std::string::npos) {
                content = content.substr(0, endPos);
            }

            if (!content.empty()) {
                std::stringstream ss(content);
                std::string token;
                while (std::getline(ss, token, ',')) {
                    row.push_back(std::stoi(token));
                }
                levelData.push_back(row);
            }
        }
        else if (line.find("],") == 0 || line.find("]") == 0) {
            inTiles = false;
        }
        else if (line.find("\"exitPortal\":{") != std::string::npos) {
            hasExitPortal = true;
            size_t xPos = line.find("\"x\":");
            size_t yPos = line.find("\"y\":");
            if (xPos != std::string::npos && yPos != std::string::npos) {
                int x = std::stoi(line.substr(xPos + 4, line.find(",", xPos) - xPos - 4));
                int y = std::stoi(line.substr(yPos + 4, line.find("}", yPos) - yPos - 4));
                exitPortalPos = sf::Vector2i(x, y);
            }
        }
        else if (line.find("\"entrancePortal\":{") != std::string::npos) {
            hasEntrancePortal = true;
            size_t xPos = line.find("\"x\":");
            size_t yPos = line.find("\"y\":");
            if (xPos != std::string::npos && yPos != std::string::npos) {
                int x = std::stoi(line.substr(xPos + 4, line.find(",", xPos) - xPos - 4));
                int y = std::stoi(line.substr(yPos + 4, line.find("}", yPos) - yPos - 4));
                entrancePortalPos = sf::Vector2i(x, y);
            }
        }
    }

    file.close();

    if (levelData.empty()) {
        std::cerr << "No level data found in file" << std::endl;
        return false;
    }

    // Charger les données dans l'éditeur
    m_width = width;
    m_height = height;
    m_levelData = levelData;
    m_enemies = enemies;
    m_hasExitPortal = hasExitPortal;
    m_exitPortalPosition = exitPortalPos;
    m_hasEntrancePortal = hasEntrancePortal;
    m_entrancePortalPosition = entrancePortalPos;

    std::cout << "Level loaded: " << width << "x" << height << std::endl;
    std::cout << "Exit portal: " << (hasExitPortal ? "Yes" : "No") << std::endl;
    std::cout << "Entrance portal: " << (hasEntrancePortal ? "Yes" : "No") << std::endl;

    return true;
}

void LevelEditor::markAsModified() {
    m_hasUnsavedChanges = true;
}


void LevelEditor::newLevel() {
    std::cout << "Creating new level..." << std::endl;

    // Réinitialiser le niveau
    m_width = 30;
    m_height = 20;
    m_levelData.clear();
    m_levelData.resize(m_height, std::vector<int>(m_width, -1));

    // Effacer tous les ennemis et portails
    m_enemies.clear();
    m_hasExitPortal = false;
    m_hasEntrancePortal = false;

    m_hasUnsavedChanges = false;

    std::cout << "New level created (" << m_width << "x" << m_height << ")" << std::endl;
}

void LevelEditor::saveCurrentLevel() {
    std::string filename = "levels/level_" + std::to_string(m_currentLevelNumber) + ".json";
    if (saveToFile(filename)) {
        std::cout << "Level " << m_currentLevelNumber << " saved successfully!" << std::endl;
    } else {
        std::cout << "Failed to save level " << m_currentLevelNumber << std::endl;
    }
}

void LevelEditor::loadLevel(int levelNumber) {
    std::string filename = "levels/level_" + std::to_string(levelNumber) + ".json";

    std::cout << "Loading level " << levelNumber << "..." << std::endl;

    // Vérifier si le fichier existe
    std::ifstream testFile(filename);
    if (!testFile.good()) {
        std::cout << "Level " << levelNumber << " doesn't exist. Creating new level." << std::endl;
        m_currentLevelNumber = levelNumber;
        newLevel();
        return;
    }
    testFile.close();

    // Charger le niveau depuis le fichier
    if (loadFromFile(filename)) {
        m_currentLevelNumber = levelNumber;
        m_hasUnsavedChanges = false;
        std::cout << "Level " << levelNumber << " loaded successfully!" << std::endl;
    } else {
        std::cout << "Failed to load level " << levelNumber << std::endl;
    }
}

void LevelEditor::startLevelSelection() {
    m_previousMode = m_mode;
    m_mode = EditorMode::LevelSelect;
    m_levelInputBuffer.clear();
    std::cout << "Level selection mode activated" << std::endl;
}

void LevelEditor::cancelLevelSelection() {
    m_mode = m_previousMode;
    m_levelInputBuffer.clear();
    std::cout << "Level selection cancelled" << std::endl;
}

void LevelEditor::renderLevelSelector(sf::RenderWindow& window) {
    // Overlay semi-transparent
    sf::RectangleShape overlay(sf::Vector2f(window.getSize().x, window.getSize().y));
    overlay.setFillColor(sf::Color(0, 0, 0, 180));
    window.draw(overlay);

    // Boîte de dialogue
    float boxWidth = 400.0f;
    float boxHeight = 200.0f;
    float boxX = (window.getSize().x - boxWidth) / 2.0f;
    float boxY = (window.getSize().y - boxHeight) / 2.0f;

    sf::RectangleShape dialogBox(sf::Vector2f(boxWidth, boxHeight));
    dialogBox.setPosition(sf::Vector2f(boxX, boxY));
    dialogBox.setFillColor(sf::Color(40, 40, 40));
    dialogBox.setOutlineColor(sf::Color::White);
    dialogBox.setOutlineThickness(3.0f);
    window.draw(dialogBox);

    // Titre
    sf::Text title(m_font, "SELECT LEVEL", 24);
    title.setPosition(sf::Vector2f(boxX + 20, boxY + 20));
    title.setFillColor(sf::Color::Yellow);
    window.draw(title);

    // Instructions
    sf::Text instructions(m_font, "Enter level number (1-99):", 18);
    instructions.setPosition(sf::Vector2f(boxX + 20, boxY + 70));
    instructions.setFillColor(sf::Color::White);
    window.draw(instructions);

    // Champ de saisie
    sf::RectangleShape inputBox(sf::Vector2f(100, 40));
    inputBox.setPosition(sf::Vector2f(boxX + 150, boxY + 100));
    inputBox.setFillColor(sf::Color(60, 60, 60));
    inputBox.setOutlineColor(sf::Color::Cyan);
    inputBox.setOutlineThickness(2.0f);
    window.draw(inputBox);

    // Texte saisi
    std::string displayText = m_levelInputBuffer.empty() ? "_" : m_levelInputBuffer;
    sf::Text inputText(m_font, displayText, 24);
    inputText.setPosition(sf::Vector2f(boxX + 165, boxY + 105));
    inputText.setFillColor(sf::Color::White);
    window.draw(inputText);

    // Niveau actuel
    std::string currentText = "Current: " + std::to_string(m_currentLevelNumber);
    sf::Text currentLevel(m_font, currentText, 16);
    currentLevel.setPosition(sf::Vector2f(boxX + 20, boxY + 110));
    currentLevel.setFillColor(sf::Color(150, 150, 150));
    window.draw(currentLevel);

    // Instructions touches
    sf::Text keys(m_font, "ENTER to confirm | ESC to cancel", 14);
    keys.setPosition(sf::Vector2f(boxX + 50, boxY + 160));
    keys.setFillColor(sf::Color(200, 200, 200));
    window.draw(keys);
}
