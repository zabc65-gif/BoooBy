#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

class PauseMenu {
public:
    enum class MenuAction {
        None,
        Continue,
        Restart,
        Quit
    };

    PauseMenu();

    void handleInput(sf::Keyboard::Key key);
    void render(sf::RenderWindow& window);

    MenuAction getSelectedAction() const;
    void resetAction();
    void setEditorMode(bool isEditor);

private:
    void updateSelection();

private:
    int m_selectedIndex;
    std::vector<std::string> m_menuItems;

    // Éléments visuels
    sf::RectangleShape m_background;
    sf::RectangleShape m_menuBox;
    std::vector<sf::RectangleShape> m_itemBoxes;

    // Police et textes
    sf::Font m_font;
    std::vector<std::unique_ptr<sf::Text>> m_menuTexts;
    std::unique_ptr<sf::Text> m_titleText;

    MenuAction m_currentAction;
    bool m_isEditorMode;
};
