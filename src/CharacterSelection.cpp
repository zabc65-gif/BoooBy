#include "CharacterSelection.hpp"
#include <iostream>

CharacterSelection::CharacterSelection()
    : m_selectedCharacter(CharacterType::Wizard)
    , m_selectionMade(false)
{
    // Charger la police
    if (!m_font.openFromFile("assets/Arial.ttf")) {
        std::cerr << "✗ Failed to load font for character selection" << std::endl;
    }

    // Charger les previews des personnages
    m_wizardPreview = std::make_unique<sf::Texture>();
    if (!m_wizardPreview->loadFromFile("assets/tiles/BlueWizard/2BlueWizardIdle/Chara - BlueIdle00000.png")) {
        std::cerr << "✗ Failed to load wizard preview" << std::endl;
    }

    m_goatPreview = std::make_unique<sf::Texture>();
    if (!m_goatPreview->loadFromFile("assets/tiles/Chevre/chevre-face.png")) {
        std::cerr << "✗ Failed to load goat preview" << std::endl;
        m_goatPreview.reset();
    }
}

void CharacterSelection::handleInput(sf::Keyboard::Key key, bool isPressed) {
    if (!isPressed) return;

    switch (key) {
        case sf::Keyboard::Key::Left:
        case sf::Keyboard::Key::Q:
            m_selectedCharacter = CharacterType::Wizard;
            break;

        case sf::Keyboard::Key::Right:
        case sf::Keyboard::Key::D:
            m_selectedCharacter = CharacterType::Goat;
            break;

        case sf::Keyboard::Key::Enter:
        case sf::Keyboard::Key::Space:
            m_selectionMade = true;
            break;

        default:
            break;
    }
}

void CharacterSelection::render(sf::RenderWindow& window) {
    // Fond noir
    window.clear(sf::Color::Black);

    // Vérifier que la font est chargée
    if (!m_font.hasGlyph('A')) {
        std::cerr << "Font not loaded, cannot render character selection" << std::endl;
        return;
    }

    // Titre
    sf::Text title(m_font, "CHOISISSEZ VOTRE PERSONNAGE", 60);
    title.setFillColor(sf::Color(255, 215, 0));
    title.setOutlineColor(sf::Color::White);
    title.setOutlineThickness(3.0f);
    title.setStyle(sf::Text::Bold);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setPosition(sf::Vector2f(640.0f - (titleBounds.position.x + titleBounds.size.x) / 2.0f, 80.0f));
    window.draw(title);

    // Instructions
    sf::Text instructions(m_font, "<- -> : Choisir | ENTREE : Valider", 24);
    instructions.setFillColor(sf::Color::White);
    sf::FloatRect instrBounds = instructions.getLocalBounds();
    instructions.setPosition(sf::Vector2f(640.0f - (instrBounds.position.x + instrBounds.size.x) / 2.0f, 620.0f));
    window.draw(instructions);

    // Position des personnages
    const float wizardX = 320.0f;
    const float goatX = 960.0f;
    const float characterY = 300.0f;

    // Cadre de sélection pour le Magicien
    sf::RectangleShape wizardFrame(sf::Vector2f(300.0f, 300.0f));
    wizardFrame.setPosition(sf::Vector2f(wizardX - 150.0f, characterY - 150.0f));
    wizardFrame.setFillColor(sf::Color::Transparent);
    if (m_selectedCharacter == CharacterType::Wizard) {
        wizardFrame.setOutlineColor(sf::Color(255, 215, 0));
        wizardFrame.setOutlineThickness(5.0f);
    } else {
        wizardFrame.setOutlineColor(sf::Color(100, 100, 100));
        wizardFrame.setOutlineThickness(2.0f);
    }
    window.draw(wizardFrame);

    // Sprite du Magicien
    if (m_wizardPreview) {
        sf::Sprite wizardSprite(*m_wizardPreview);
        wizardSprite.setScale(sf::Vector2f(0.5f, 0.5f));
        sf::FloatRect wizardBounds = wizardSprite.getLocalBounds();
        wizardSprite.setOrigin(sf::Vector2f(wizardBounds.position.x + wizardBounds.size.x / 2.0f,
                                            wizardBounds.position.y + wizardBounds.size.y / 2.0f));
        wizardSprite.setPosition(sf::Vector2f(wizardX, characterY));
        window.draw(wizardSprite);
    }

    // Label Magicien
    sf::Text wizardLabel(m_font, "MAGICIEN", 32);
    wizardLabel.setFillColor(m_selectedCharacter == CharacterType::Wizard ?
                              sf::Color(255, 215, 0) : sf::Color::White);
    wizardLabel.setStyle(sf::Text::Bold);
    sf::FloatRect wizardLabelBounds = wizardLabel.getLocalBounds();
    wizardLabel.setPosition(sf::Vector2f(wizardX - (wizardLabelBounds.position.x + wizardLabelBounds.size.x) / 2.0f,
                                         characterY + 180.0f));
    window.draw(wizardLabel);

    // Cadre de sélection pour la Chèvre
    sf::RectangleShape goatFrame(sf::Vector2f(300.0f, 300.0f));
    goatFrame.setPosition(sf::Vector2f(goatX - 150.0f, characterY - 150.0f));
    goatFrame.setFillColor(sf::Color::Transparent);
    if (m_selectedCharacter == CharacterType::Goat) {
        goatFrame.setOutlineColor(sf::Color(255, 215, 0));
        goatFrame.setOutlineThickness(5.0f);
    } else {
        goatFrame.setOutlineColor(sf::Color(100, 100, 100));
        goatFrame.setOutlineThickness(2.0f);
    }
    window.draw(goatFrame);

    // Sprite de la Chèvre
    if (m_goatPreview) {
        sf::Sprite goatSprite(*m_goatPreview);
        goatSprite.setScale(sf::Vector2f(0.225f, 0.225f));  // Diminué d'un quart (0.3 * 0.75 = 0.225)
        sf::FloatRect goatBounds = goatSprite.getLocalBounds();
        goatSprite.setOrigin(sf::Vector2f(goatBounds.position.x + goatBounds.size.x / 2.0f,
                                          goatBounds.position.y + goatBounds.size.y / 2.0f));
        goatSprite.setPosition(sf::Vector2f(goatX, characterY));
        window.draw(goatSprite);
    }

    // Label Chèvre
    sf::Text goatLabel(m_font, "CHEVRE", 32);
    goatLabel.setFillColor(m_selectedCharacter == CharacterType::Goat ?
                           sf::Color(255, 215, 0) : sf::Color::White);
    goatLabel.setStyle(sf::Text::Bold);
    sf::FloatRect goatLabelBounds = goatLabel.getLocalBounds();
    goatLabel.setPosition(sf::Vector2f(goatX - (goatLabelBounds.position.x + goatLabelBounds.size.x) / 2.0f,
                                       characterY + 180.0f));
    window.draw(goatLabel);
}
