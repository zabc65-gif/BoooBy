#include "PauseMenu.hpp"
#include <iostream>

PauseMenu::PauseMenu()
    : m_selectedIndex(0)
    , m_currentAction(MenuAction::None)
    , m_isEditorMode(false)
{
    m_menuItems = {"Continuer", "Recommencer", "Quitter"};

    // Charger la police - essayer plusieurs chemins possibles
    bool fontLoaded = false;

    // Chemin 1: relatif au répertoire de travail
    if (m_font.openFromFile("assets/Arial.ttf")) {
        fontLoaded = true;
        std::cout << "Police chargée depuis: assets/Arial.ttf" << std::endl;
    }
    // Chemin 2: dans le bundle macOS
    else if (m_font.openFromFile("../Resources/assets/Arial.ttf")) {
        fontLoaded = true;
        std::cout << "Police chargée depuis: ../Resources/assets/Arial.ttf" << std::endl;
    }
    // Chemin 3: police système
    else if (m_font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf")) {
        fontLoaded = true;
        std::cout << "Police chargée depuis le système" << std::endl;
    }

    if (!fontLoaded) {
        std::cerr << "ERREUR: Impossible de charger la police Arial.ttf" << std::endl;
        std::cerr << "Le texte du menu ne s'affichera pas correctement." << std::endl;
    }

    // Fond semi-transparent
    m_background.setSize(sf::Vector2f(1280.0f, 720.0f));
    m_background.setFillColor(sf::Color(0, 0, 0, 180));

    // Boîte du menu
    m_menuBox.setSize(sf::Vector2f(400.0f, 350.0f));
    m_menuBox.setPosition(sf::Vector2f(440.0f, 185.0f));
    m_menuBox.setFillColor(sf::Color(40, 40, 50));
    m_menuBox.setOutlineThickness(3.0f);
    m_menuBox.setOutlineColor(sf::Color(100, 100, 120));

    // Titre du menu
    m_titleText = std::make_unique<sf::Text>(m_font, "PAUSE", 40);
    m_titleText->setFillColor(sf::Color::White);

    // Centrer le titre "PAUSE" horizontalement
    sf::FloatRect titleBounds = m_titleText->getLocalBounds();
    m_titleText->setPosition(sf::Vector2f(
        640.0f - titleBounds.size.x / 2.0f,  // Centre de l'écran (1280/2 = 640)
        210.0f
    ));

    // Créer les boîtes et textes pour le menu
    m_itemBoxes.resize(m_menuItems.size());
    m_menuTexts.resize(m_menuItems.size());
    for (size_t i = 0; i < m_menuItems.size(); ++i) {
        m_itemBoxes[i].setSize(sf::Vector2f(300.0f, 60.0f));
        m_itemBoxes[i].setPosition(sf::Vector2f(490.0f, 290.0f + i * 80.0f));
        m_itemBoxes[i].setOutlineThickness(2.0f);

        m_menuTexts[i] = std::make_unique<sf::Text>(m_font, m_menuItems[i], 28);
        m_menuTexts[i]->setFillColor(sf::Color::White);

        // Centrer le texte dans la boîte
        sf::FloatRect textBounds = m_menuTexts[i]->getLocalBounds();
        m_menuTexts[i]->setPosition(sf::Vector2f(
            490.0f + (300.0f - textBounds.size.x) / 2.0f,
            290.0f + i * 80.0f + (60.0f - textBounds.size.y) / 2.0f - 8.0f
        ));
    }

    updateSelection();
}

void PauseMenu::handleInput(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Key::Up:
        case sf::Keyboard::Key::Z:
            if (m_selectedIndex > 0) {
                m_selectedIndex--;
                updateSelection();
                std::cout << "Menu: Selection = " << m_menuItems[m_selectedIndex] << std::endl;
            }
            break;

        case sf::Keyboard::Key::Down:
        case sf::Keyboard::Key::S:
            if (m_selectedIndex < static_cast<int>(m_menuItems.size()) - 1) {
                m_selectedIndex++;
                updateSelection();
                std::cout << "Menu: Selection = " << m_menuItems[m_selectedIndex] << std::endl;
            }
            break;

        case sf::Keyboard::Key::Enter:
        case sf::Keyboard::Key::Space:
            if (m_selectedIndex == 0) {
                m_currentAction = MenuAction::Continue;
                std::cout << "Menu: Action = Continue" << std::endl;
            } else if (m_selectedIndex == 1) {
                m_currentAction = MenuAction::Restart;
                std::cout << "Menu: Action = Restart" << std::endl;
            } else if (m_selectedIndex == 2) {
                m_currentAction = MenuAction::Quit;
                std::cout << "Menu: Action = Quit" << std::endl;
            }
            break;

        default:
            break;
    }
}

void PauseMenu::updateSelection() {
    for (size_t i = 0; i < m_itemBoxes.size(); ++i) {
        if (static_cast<int>(i) == m_selectedIndex) {
            // Item sélectionné
            m_itemBoxes[i].setFillColor(sf::Color(100, 150, 200));
            m_itemBoxes[i].setOutlineColor(sf::Color(150, 200, 255));
            m_menuTexts[i]->setFillColor(sf::Color::White);
        } else {
            // Item non sélectionné
            m_itemBoxes[i].setFillColor(sf::Color(60, 60, 80));
            m_itemBoxes[i].setOutlineColor(sf::Color(80, 80, 100));
            m_menuTexts[i]->setFillColor(sf::Color(180, 180, 180));
        }
    }
}

void PauseMenu::render(sf::RenderWindow& window) {
    window.draw(m_background);
    window.draw(m_menuBox);
    window.draw(*m_titleText);

    for (size_t i = 0; i < m_itemBoxes.size(); ++i) {
        window.draw(m_itemBoxes[i]);
        window.draw(*m_menuTexts[i]);
    }
}

PauseMenu::MenuAction PauseMenu::getSelectedAction() const {
    return m_currentAction;
}

void PauseMenu::resetAction() {
    m_currentAction = MenuAction::None;
    m_selectedIndex = 0;
    updateSelection();
}

void PauseMenu::setEditorMode(bool isEditor) {
    m_isEditorMode = isEditor;
}
