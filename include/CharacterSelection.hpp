#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

enum class CharacterType {
    Wizard,
    Goat
};

class CharacterSelection {
public:
    CharacterSelection();

    void handleInput(sf::Keyboard::Key key, bool isPressed);
    void render(sf::RenderWindow& window);

    bool isSelectionMade() const { return m_selectionMade; }
    CharacterType getSelectedCharacter() const { return m_selectedCharacter; }

private:
    CharacterType m_selectedCharacter;
    bool m_selectionMade;
    sf::Font m_font;

    // Preview textures
    std::unique_ptr<sf::Texture> m_wizardPreview;
    std::unique_ptr<sf::Texture> m_goatPreview;
};
