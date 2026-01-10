#include "Player.hpp"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>

Player::Player(CharacterType characterType)
    : m_position(0.0f, 0.0f)
    , m_velocity(0.0f, 0.0f)
    , m_state(State::Idle)
    , m_previousState(State::Idle)
    , m_isMovingLeft(false)
    , m_isMovingRight(false)
    , m_isRunning(false)
    , m_isGrounded(false)
    , m_facingRight(true)
    , m_characterType(characterType)
    , m_spriteScale(0.2f)  // Valeur par défaut pour le Magicien
    , m_animationSpeed(1.0f)  // Valeur par défaut
    , m_currentFrame(0)
    , m_frameTimer(0.0f)
    , m_health(100)
    , m_maxHealth(100)
    , m_invincibilityTimer(0.0f)
    , m_isDisintegrating(false)
    , m_hasDoubleJump(false)
    , m_jumpsRemaining(1)
    , m_hasHeroCharge(false)
    , m_isPreparingCharge(false)
    , m_isCharging(false)
    , m_chargeTimer(0.0f)
    , m_chargeCooldown(0.0f)
    , m_prepareTimer(0.0f)
    , m_chargeStartPosition(0.0f, 0.0f)
    , m_firefly1Position(0.0f, 0.0f)
    , m_firefly2Position(0.0f, 0.0f)
    , m_fireflyLagPosition(0.0f, 0.0f)
    , m_fireflyTimer(0.0f)
    , m_fireflyAlpha1(0.0f)
    , m_fireflyAlpha2(0.0f)
{
    // Charger les animations selon le personnage
    if (m_characterType == CharacterType::Wizard) {
        std::cout << "Chargement des animations du Blue Wizard..." << std::endl;
        m_spriteScale = 0.2f;
        m_animationSpeed = 1.0f;  // Vitesse normale
        loadAnimationFrames("assets/tiles/BlueWizard/2BlueWizardIdle", "Chara - BlueIdle", 20, m_idleTextures);
        loadAnimationFrames("assets/tiles/BlueWizard/2BlueWizardWalk", "Chara_BlueWalk", 20, m_walkTextures);
        loadAnimationFrames("assets/tiles/BlueWizard/2BlueWizardJump", "CharaWizardJump_", 8, m_jumpTextures);

        // Créer le sprite avec la première frame de l'animation idle
        if (!m_idleTextures.empty()) {
            m_sprite = std::make_unique<sf::Sprite>(*m_idleTextures[0]);
            m_sprite->setScale(sf::Vector2f(m_spriteScale, m_spriteScale));
            std::cout << "✓ Animations chargées avec succès!" << std::endl;
        } else {
            std::cerr << "✗ Échec du chargement des animations!" << std::endl;
        }
    } else if (m_characterType == CharacterType::Goat) {
        std::cout << "Chargement des animations de la Chèvre..." << std::endl;
        m_spriteScale = 0.363f;  // Agrandi de 10% supplémentaire (0.33 * 1.1 = 0.363)
        m_animationSpeed = 2.2f;  // Animation 2.2x plus lente

        // Idle: chevre-statique-droite (1 frame statique depuis le dossier static)
        auto idleTexture = std::make_shared<sf::Texture>();
        if (idleTexture->loadFromFile("assets/tiles/Chevre/static/chevre-statique-droite-00.png")) {
            m_idleTextures.push_back(idleTexture);
        }

        // Walk: chevre-course (7 frames: 00 à 06 depuis le dossier principal)
        loadAnimationFrames("assets/tiles/Chevre", "chevre-course-", 7, m_walkTextures);

        // Jump: chevre-saute (1 frame: 01 depuis le dossier principal)
        auto jumpTexture = std::make_shared<sf::Texture>();
        if (jumpTexture->loadFromFile("assets/tiles/Chevre/chevre-saute-01.png")) {
            m_jumpTextures.push_back(jumpTexture);
        }

        // Créer le sprite avec la première frame de l'animation idle
        if (!m_idleTextures.empty()) {
            m_sprite = std::make_unique<sf::Sprite>(*m_idleTextures[0]);
            m_sprite->setScale(sf::Vector2f(m_spriteScale, m_spriteScale));
            std::cout << "✓ Chèvre chargée avec succès!" << std::endl;
        } else {
            std::cerr << "✗ Échec du chargement de la chèvre!" << std::endl;
        }
    }

    // Charger le son de saut
    if (m_jumpSoundBuffer.loadFromFile("assets/sounds/jump1.wav")) {
        m_jumpSound = std::make_unique<sf::Sound>(m_jumpSoundBuffer);
        m_jumpSound->setVolume(50.0f); // Volume à 50%
        std::cout << "✓ Son de saut chargé avec succès!" << std::endl;
    } else {
        std::cerr << "✗ Échec du chargement du son de saut (assets/sounds/jump1.wav)" << std::endl;
    }
}

void Player::loadAnimationFrames(const std::string& directory, const std::string& prefix, int frameCount, std::vector<std::shared_ptr<sf::Texture>>& textures) {
    for (int i = 0; i < frameCount; ++i) {
        auto texture = std::make_shared<sf::Texture>();

        // Déterminer le format de numérotation (2 ou 5 chiffres)
        std::string filename;
        if (prefix.find("Blue") != std::string::npos) {
            // Format Wizard: "Chara - BlueIdle00000.png" (5 chiffres)
            filename = directory + "/" + prefix + std::string(5 - std::to_string(i).length(), '0') + std::to_string(i) + ".png";
        } else {
            // Format Goat: "chevre-course-00.png" (2 chiffres)
            filename = directory + "/" + prefix + std::string(2 - std::to_string(i).length(), '0') + std::to_string(i) + ".png";
        }

        if (texture->loadFromFile(filename)) {
            textures.push_back(texture);
        } else {
            std::cerr << "Erreur lors du chargement de: " << filename << std::endl;
        }
    }

    std::cout << "  - " << textures.size() << " frames chargées depuis " << directory << std::endl;
}

void Player::loadSpriteSheet(const std::string& filepath, int frameWidth, int frameHeight, int totalFrames, std::vector<std::shared_ptr<sf::Texture>>& textures) {
    // Charger l'image complète
    sf::Image spriteSheet;
    if (!spriteSheet.loadFromFile(filepath)) {
        std::cerr << "✗ Échec du chargement du sprite sheet: " << filepath << std::endl;
        return;
    }

    int sheetWidth = spriteSheet.getSize().x;
    int sheetHeight = spriteSheet.getSize().y;
    int columns = sheetWidth / frameWidth;
    int rows = sheetHeight / frameHeight;

    std::cout << "  Loading sprite sheet: " << filepath << std::endl;
    std::cout << "    Sheet size: " << sheetWidth << "x" << sheetHeight << std::endl;
    std::cout << "    Frame size: " << frameWidth << "x" << frameHeight << std::endl;
    std::cout << "    Grid: " << columns << "x" << rows << " (max " << columns * rows << " frames)" << std::endl;

    // Extraire chaque frame
    int framesLoaded = 0;
    for (int i = 0; i < totalFrames && framesLoaded < columns * rows; ++i) {
        int row = i / columns;
        int col = i % columns;

        // Créer une texture pour cette frame
        auto texture = std::make_shared<sf::Texture>();

        // Charger juste la région de cette frame depuis le sprite sheet
        sf::IntRect area(sf::Vector2i(col * frameWidth, row * frameHeight), sf::Vector2i(frameWidth, frameHeight));
        if (texture->loadFromImage(spriteSheet, false, area)) {
            textures.push_back(texture);
            framesLoaded++;
        } else {
            std::cerr << "    ✗ Échec de l'extraction de la frame " << i << std::endl;
        }
    }

    std::cout << "    ✓ " << framesLoaded << " frames extraites" << std::endl;
}

void Player::handleInput(sf::Keyboard::Key key, bool isPressed) {
    switch (key) {
        case sf::Keyboard::Key::Left:
        case sf::Keyboard::Key::Q:
            m_isMovingLeft = isPressed;
            break;

        case sf::Keyboard::Key::Right:
        case sf::Keyboard::Key::D:
            m_isMovingRight = isPressed;
            break;

        case sf::Keyboard::Key::LShift:
            m_isRunning = isPressed;
            break;

        case sf::Keyboard::Key::Space:
            if (isPressed) {
                // Saut simple ou double saut
                int maxJumps = m_hasDoubleJump ? 2 : 1;

                if (m_jumpsRemaining > 0) {
                    m_velocity.y = JUMP_FORCE;
                    m_state = State::Jumping;
                    m_isGrounded = false;
                    m_jumpsRemaining--;

                    // Jouer le son de saut
                    if (m_jumpSound) {
                        m_jumpSound->play();
                    }

                    std::cout << "Jump! Jumps remaining: " << m_jumpsRemaining << "/" << maxJumps << std::endl;
                }
            }
            break;

        case sf::Keyboard::Key::RShift:
            // Charge du héros (Shift droit) - déclenche la préparation
            if (isPressed && m_hasHeroCharge && !m_isCharging && !m_isPreparingCharge && m_chargeCooldown <= 0.0f) {
                m_isPreparingCharge = true;
                m_prepareTimer = CHARGE_PREPARE_DURATION;
                m_chargeStartPosition = m_position;
                m_chargeTrailPositions.clear();
                m_explosionParticles.clear();
                std::cout << "Hero Charge preparing..." << std::endl;
            }
            break;

        default:
            break;
    }
}

void Player::update(sf::Time deltaTime) {
    // Si le joueur est en train de se désintégrer, ne plus mettre à jour la physique
    if (m_isDisintegrating) {
        m_particleSystem.update(deltaTime);
        return;
    }

    // Mettre à jour le timer d'invincibilité
    updateInvincibility(deltaTime.asSeconds());

    // Mettre à jour le cooldown de la charge
    if (m_chargeCooldown > 0.0f) {
        m_chargeCooldown -= deltaTime.asSeconds();
    }

    float dt = deltaTime.asSeconds();
    const float playerWidth = 102.0f;
    const float playerHeight = 102.0f;
    sf::Vector2f playerCenter(m_position.x + playerWidth / 2.0f, m_position.y + playerHeight / 2.0f);

    // Phase de préparation de la charge
    if (m_isPreparingCharge) {
        m_prepareTimer -= dt;

        // Bloquer le mouvement pendant la préparation
        m_velocity.x = 0.0f;
        m_velocity.y = 0.0f;

        // Quand la préparation est terminée, déclencher la charge et l'explosion
        if (m_prepareTimer <= 0.0f) {
            m_isPreparingCharge = false;
            m_isCharging = true;
            m_chargeTimer = CHARGE_DURATION;
            m_invincibilityTimer = CHARGE_DURATION + 0.3f;

            // Créer l'explosion de particules
            std::srand(static_cast<unsigned>(std::time(nullptr)));
            for (int i = 0; i < 24; ++i) {
                ExplosionParticle p;
                float angle = (i / 24.0f) * 2.0f * 3.14159f + (std::rand() % 100) / 100.0f * 0.3f;
                float speed = 200.0f + (std::rand() % 200);
                p.position = playerCenter;
                p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
                p.life = 1.0f;
                p.size = 8.0f + (std::rand() % 8);

                // Couleurs variées : orange, jaune, blanc
                int colorType = std::rand() % 3;
                if (colorType == 0) p.color = sf::Color(255, 180, 50, 255);
                else if (colorType == 1) p.color = sf::Color(255, 255, 100, 255);
                else p.color = sf::Color(255, 255, 220, 255);

                m_explosionParticles.push_back(p);
            }

            std::cout << "Hero Charge ACTIVATED!" << std::endl;
        }
    }

    // Mettre à jour les particules d'explosion
    for (auto& p : m_explosionParticles) {
        p.position += p.velocity * dt;
        p.velocity *= 0.95f;  // Friction
        p.life -= dt * 2.5f;  // Disparition rapide
        p.size *= 0.97f;      // Rétrécissement
    }
    // Supprimer les particules mortes
    m_explosionParticles.erase(
        std::remove_if(m_explosionParticles.begin(), m_explosionParticles.end(),
            [](const ExplosionParticle& p) { return p.life <= 0.0f; }),
        m_explosionParticles.end());

    // Phase de charge active
    if (m_isCharging) {
        m_chargeTimer -= dt;

        // Déplacer le joueur dans la direction de la charge
        float chargeDirection = m_facingRight ? 1.0f : -1.0f;
        m_velocity.x = chargeDirection * CHARGE_SPEED;
        m_velocity.y = 0.0f;  // Pas de gravité pendant la charge

        // Ajouter la position actuelle à la traînée (plus fréquemment)
        m_chargeTrailPositions.push_back(playerCenter);
        if (m_chargeTrailPositions.size() > static_cast<size_t>(CHARGE_TRAIL_LENGTH)) {
            m_chargeTrailPositions.erase(m_chargeTrailPositions.begin());
        }

        // Fin de la charge
        if (m_chargeTimer <= 0.0f) {
            m_isCharging = false;
            m_chargeCooldown = CHARGE_COOLDOWN;
            m_velocity.x = 0.0f;
            std::cout << "Hero Charge ended!" << std::endl;
        }
    }

    // Faire disparaître la traînée quand on ne charge plus
    if (!m_isCharging && !m_chargeTrailPositions.empty()) {
        // Supprimer progressivement les points de la traînée
        if (m_chargeTrailPositions.size() > 2) {
            m_chargeTrailPositions.erase(m_chargeTrailPositions.begin());
        } else {
            m_chargeTrailPositions.clear();
        }
    }

    updatePhysics(deltaTime);
    updateAnimation(deltaTime);

    // Mettre à jour la traînée des lucioles et l'inertie
    if (m_hasDoubleJump) {
        // Calculer la position de base des lucioles (centre du joueur)
        const float playerWidth = 102.0f;
        sf::Vector2f playerCenter(m_position.x + playerWidth / 2.0f, m_position.y - 10.0f);

        // Interpoler la position avec retard vers la position actuelle du joueur (inertie)
        // Plus de retard sur l'axe horizontal pour un effet de flottement latéral
        m_fireflyLagPosition.x += (playerCenter.x - m_fireflyLagPosition.x) * FIREFLY_LAG_SMOOTH_X;
        m_fireflyLagPosition.y += (playerCenter.y - m_fireflyLagPosition.y) * FIREFLY_LAG_SMOOTH_Y;

        // Mettre à jour l'opacité des lucioles (fade-in/fade-out)
        float alphaChange = FIREFLY_FADE_SPEED * deltaTime.asSeconds();

        // Luciole 1 : fade-in si disponible, fade-out sinon
        float targetAlpha1 = (m_jumpsRemaining >= 1) ? 1.0f : 0.0f;
        if (m_fireflyAlpha1 < targetAlpha1) {
            m_fireflyAlpha1 = std::min(1.0f, m_fireflyAlpha1 + alphaChange);
        } else if (m_fireflyAlpha1 > targetAlpha1) {
            m_fireflyAlpha1 = std::max(0.0f, m_fireflyAlpha1 - alphaChange);
        }

        // Luciole 2 : fade-in si disponible, fade-out sinon
        float targetAlpha2 = (m_jumpsRemaining >= 2) ? 1.0f : 0.0f;
        if (m_fireflyAlpha2 < targetAlpha2) {
            m_fireflyAlpha2 = std::min(1.0f, m_fireflyAlpha2 + alphaChange);
        } else if (m_fireflyAlpha2 > targetAlpha2) {
            m_fireflyAlpha2 = std::max(0.0f, m_fireflyAlpha2 - alphaChange);
        }

        // Vieillir tous les points de la traînée
        float ageIncrement = TRAIL_FADE_SPEED * deltaTime.asSeconds();
        for (auto& point : m_fireflyTrail1) {
            point.age = std::min(1.0f, point.age + ageIncrement);
        }
        for (auto& point : m_fireflyTrail2) {
            point.age = std::min(1.0f, point.age + ageIncrement);
        }

        // Supprimer les points trop vieux
        m_fireflyTrail1.erase(
            std::remove_if(m_fireflyTrail1.begin(), m_fireflyTrail1.end(),
                [](const FireflyTrailPoint& p) { return p.age >= 1.0f; }),
            m_fireflyTrail1.end());
        m_fireflyTrail2.erase(
            std::remove_if(m_fireflyTrail2.begin(), m_fireflyTrail2.end(),
                [](const FireflyTrailPoint& p) { return p.age >= 1.0f; }),
            m_fireflyTrail2.end());
    }
}

void Player::updatePhysics(sf::Time deltaTime) {
    float dt = deltaTime.asSeconds();

    // Si en train de charger ou de préparer la charge, ne pas modifier la vélocité
    if (!m_isCharging && !m_isPreparingCharge) {
        // Mouvement horizontal
        if (m_isMovingLeft && !m_isMovingRight) {
            float speed = m_isRunning ? RUN_SPEED : WALK_SPEED;
            m_velocity.x = -speed;
            // Ne changer l'état que si on est au sol
            if (m_isGrounded) {
                m_state = m_isRunning ? State::Running : State::Walking;
            }
            m_facingRight = false; // Regarder à gauche
        }
        else if (m_isMovingRight && !m_isMovingLeft) {
            float speed = m_isRunning ? RUN_SPEED : WALK_SPEED;
            m_velocity.x = speed;
            // Ne changer l'état que si on est au sol
            if (m_isGrounded) {
                m_state = m_isRunning ? State::Running : State::Walking;
            }
            m_facingRight = true; // Regarder à droite
        }
        else {
            m_velocity.x = 0.0f;
            if (m_isGrounded) {
                m_state = State::Idle;
            }
        }

        // Gravité (pas pendant la charge)
        if (!m_isGrounded) {
            m_velocity.y += GRAVITY * dt;

            // Limiter la vitesse de chute maximale pour plus de fluidité
            if (m_velocity.y > 600.0f) {  // MAX_FALL_SPEED
                m_velocity.y = 600.0f;
            }
        }
    }

    // Mise à jour de la position
    m_position += m_velocity * dt;

    // La gestion du sol est maintenant entièrement gérée par le système de niveau (Level::handlePlayerCollision)
    // On garde juste la détection de l'état de chute
    if (m_velocity.y > 0 && m_state != State::Falling && !m_isGrounded) {
        m_state = State::Falling;
    }
}

void Player::updateAnimation(sf::Time deltaTime) {
    if (!m_sprite) return;

    // Mettre à jour la position du sprite
    m_sprite->setPosition(m_position);

    // Si l'état a changé, réinitialiser l'animation
    if (m_state != m_previousState) {
        m_currentFrame = 0;
        m_frameTimer = 0.0f;
        m_previousState = m_state;
    }

    // Sélectionner le bon ensemble de textures selon l'état
    std::vector<std::shared_ptr<sf::Texture>>* currentAnimation = nullptr;

    switch (m_state) {
        case State::Idle:
            currentAnimation = &m_idleTextures;
            break;
        case State::Walking:
        case State::Running:
            currentAnimation = &m_walkTextures;
            break;
        case State::Jumping:
        case State::Falling:
            currentAnimation = &m_jumpTextures;
            break;
        default:
            currentAnimation = &m_idleTextures;
            break;
    }

    if (currentAnimation && !currentAnimation->empty()) {
        // Incrémenter le timer
        m_frameTimer += deltaTime.asSeconds();

        // Changer de frame si nécessaire (utiliser m_animationSpeed pour ralentir ou accélérer)
        float effectiveFrameTime = FRAME_TIME * m_animationSpeed;
        if (m_frameTimer >= effectiveFrameTime) {
            m_frameTimer -= effectiveFrameTime;
            m_currentFrame = (m_currentFrame + 1) % currentAnimation->size();

            // Changer la texture du sprite
            m_sprite->setTexture(*(*currentAnimation)[m_currentFrame], false);
        }
    }
}

void Player::render(sf::RenderWindow& window) {
    // Si le joueur est en train de se désintégrer, afficher les particules au lieu du sprite
    if (m_isDisintegrating) {
        m_particleSystem.render(window);
        return;
    }

    if (!m_sprite) return;

    // Appliquer le flip horizontal selon la direction
    float scaleX = m_facingRight ? m_spriteScale : -m_spriteScale;
    m_sprite->setScale(sf::Vector2f(scaleX, m_spriteScale));

    // Ajuster l'origine pour que le flip soit correct
    if (!m_facingRight) {
        m_sprite->setOrigin(sf::Vector2f(m_sprite->getTexture().getSize().x, 0.0f));
    } else {
        m_sprite->setOrigin(sf::Vector2f(0.0f, 0.0f));
    }

    // Dessiner le sprite
    window.draw(*m_sprite);

    // Dessiner les lucioles si le double saut est débloqué
    if (m_hasDoubleJump) {
        // Utiliser la position avec inertie au lieu de la position directe du joueur
        const float playerCenterX = m_fireflyLagPosition.x;
        const float playerTopY = m_fireflyLagPosition.y;

        // Animation de vol réaliste (mouvement en forme de 8 allongé)
        m_fireflyTimer += 0.08f; // Vitesse d'animation

        // Mouvement vertical et horizontal pour un vol naturel
        float offsetY = std::sin(m_fireflyTimer) * 5.0f;
        float offsetX = std::cos(m_fireflyTimer * 0.7f) * 3.0f; // Fréquence différente pour effet de lemniscate

        // Luciole gauche (premier saut disponible)
        if (m_fireflyAlpha1 > 0.0f) {  // Dessiner seulement si au moins un peu visible
            float firefly1X = playerCenterX - 15.0f + offsetX;
            float firefly1Y = playerTopY + offsetY;

            // Mettre à jour la position actuelle et ajouter à la traînée
            m_firefly1Position = sf::Vector2f(firefly1X, firefly1Y);
            if (m_fireflyAlpha1 > 0.1f) {  // Ajouter des points de traînée seulement si assez visible
                m_fireflyTrail1.push_back({m_firefly1Position, 0.0f});
                if (m_fireflyTrail1.size() > MAX_TRAIL_POINTS) {
                    m_fireflyTrail1.erase(m_fireflyTrail1.begin());
                }
            }

            // Dessiner la traînée de la luciole 1 (du plus ancien au plus récent)
            for (size_t i = 0; i < m_fireflyTrail1.size(); ++i) {
                const auto& trailPt = m_fireflyTrail1[i];
                float alpha = (1.0f - trailPt.age) * 150.0f * m_fireflyAlpha1; // Fade out progressif avec alpha global
                float size = (1.0f - trailPt.age) * 3.0f + 1.0f; // Taille diminue avec l'âge

                // Halo de la traînée
                sf::CircleShape trailGlow(size * 2.0f);
                trailGlow.setPosition(trailPt.position);
                trailGlow.setFillColor(sf::Color(255, 255, 100, static_cast<unsigned char>(alpha * 0.3f)));
                trailGlow.setOrigin(sf::Vector2f(size * 2.0f, size * 2.0f));
                window.draw(trailGlow);

                // Point de la traînée
                sf::CircleShape trailCircle(size);
                trailCircle.setPosition(trailPt.position);
                trailCircle.setFillColor(sf::Color(255, 255, 200, static_cast<unsigned char>(alpha)));
                trailCircle.setOrigin(sf::Vector2f(size, size));
                window.draw(trailCircle);
            }

            // Luciole principale avec alpha
            sf::CircleShape firefly1(4.0f);
            firefly1.setPosition(sf::Vector2f(firefly1X, firefly1Y));
            firefly1.setFillColor(sf::Color(255, 255, 200, static_cast<unsigned char>(200.0f * m_fireflyAlpha1)));
            firefly1.setOrigin(sf::Vector2f(4.0f, 4.0f));

            // Lueur autour de la luciole avec alpha
            sf::CircleShape glow1(8.0f);
            glow1.setPosition(sf::Vector2f(firefly1X, firefly1Y));
            glow1.setFillColor(sf::Color(255, 255, 100, static_cast<unsigned char>(80.0f * m_fireflyAlpha1)));
            glow1.setOrigin(sf::Vector2f(8.0f, 8.0f));

            window.draw(glow1);
            window.draw(firefly1);
        }

        // Luciole droite (deuxième saut disponible)
        if (m_fireflyAlpha2 > 0.0f) {  // Dessiner seulement si au moins un peu visible
            // Mouvement déphasé pour un effet naturel (les deux lucioles ne volent pas de la même façon)
            float offsetY2 = std::sin(m_fireflyTimer + 1.5f) * 5.0f; // Déphasage de ~90°
            float offsetX2 = std::cos(m_fireflyTimer * 0.7f + 1.5f) * 3.0f;

            float firefly2X = playerCenterX + 15.0f + offsetX2;
            float firefly2Y = playerTopY + offsetY2;

            // Mettre à jour la position actuelle et ajouter à la traînée
            m_firefly2Position = sf::Vector2f(firefly2X, firefly2Y);
            if (m_fireflyAlpha2 > 0.1f) {  // Ajouter des points de traînée seulement si assez visible
                m_fireflyTrail2.push_back({m_firefly2Position, 0.0f});
                if (m_fireflyTrail2.size() > MAX_TRAIL_POINTS) {
                    m_fireflyTrail2.erase(m_fireflyTrail2.begin());
                }
            }

            // Dessiner la traînée de la luciole 2 (du plus ancien au plus récent)
            for (size_t i = 0; i < m_fireflyTrail2.size(); ++i) {
                const auto& trailPt = m_fireflyTrail2[i];
                float alpha = (1.0f - trailPt.age) * 150.0f * m_fireflyAlpha2; // Fade out progressif avec alpha global
                float size = (1.0f - trailPt.age) * 3.0f + 1.0f; // Taille diminue avec l'âge

                // Halo de la traînée
                sf::CircleShape trailGlow(size * 2.0f);
                trailGlow.setPosition(trailPt.position);
                trailGlow.setFillColor(sf::Color(255, 255, 100, static_cast<unsigned char>(alpha * 0.3f)));
                trailGlow.setOrigin(sf::Vector2f(size * 2.0f, size * 2.0f));
                window.draw(trailGlow);

                // Point de la traînée
                sf::CircleShape trailCircle(size);
                trailCircle.setPosition(trailPt.position);
                trailCircle.setFillColor(sf::Color(255, 255, 200, static_cast<unsigned char>(alpha)));
                trailCircle.setOrigin(sf::Vector2f(size, size));
                window.draw(trailCircle);
            }

            // Luciole principale avec alpha
            sf::CircleShape firefly2(4.0f);
            firefly2.setPosition(sf::Vector2f(firefly2X, firefly2Y));
            firefly2.setFillColor(sf::Color(255, 255, 200, static_cast<unsigned char>(200.0f * m_fireflyAlpha2)));
            firefly2.setOrigin(sf::Vector2f(4.0f, 4.0f));

            // Lueur autour de la luciole avec alpha
            sf::CircleShape glow2(8.0f);
            glow2.setPosition(sf::Vector2f(firefly2X, firefly2Y));
            glow2.setFillColor(sf::Color(255, 255, 100, static_cast<unsigned char>(80.0f * m_fireflyAlpha2)));
            glow2.setOrigin(sf::Vector2f(8.0f, 8.0f));

            window.draw(glow2);
            window.draw(firefly2);
        }
    }

    // Dessiner les effets de la charge du héros
    if (m_hasHeroCharge) {
        const float playerWidth = 102.0f;
        const float playerHeight = 102.0f;
        sf::Vector2f playerCenter(m_position.x + playerWidth / 2.0f, m_position.y + playerHeight / 2.0f);

        // Phase de préparation : particules qui tournent et se concentrent
        if (m_isPreparingCharge) {
            float prepareProgress = 1.0f - (m_prepareTimer / CHARGE_PREPARE_DURATION);  // 0 -> 1

            // Aura grandissante
            float auraSize = 30.0f + prepareProgress * 50.0f;
            sf::CircleShape prepareAura(auraSize);
            prepareAura.setPosition(playerCenter);
            prepareAura.setOrigin(sf::Vector2f(auraSize, auraSize));
            prepareAura.setFillColor(sf::Color(255, 200, 100, static_cast<unsigned char>(40 + prepareProgress * 60)));
            prepareAura.setOutlineColor(sf::Color(255, 150, 50, static_cast<unsigned char>(100 + prepareProgress * 100)));
            prepareAura.setOutlineThickness(2.0f + prepareProgress * 2.0f);
            window.draw(prepareAura);

            // Particules qui tournent de plus en plus vite et se rapprochent
            int particleCount = 12;
            float baseSpeed = 5.0f + prepareProgress * 20.0f;  // Accélération progressive
            float orbitRadius = 80.0f - prepareProgress * 50.0f;  // Se rapprochent du centre

            for (int i = 0; i < particleCount; ++i) {
                float baseAngle = (i / static_cast<float>(particleCount)) * 2.0f * 3.14159f;
                // Timer basé sur prepareProgress pour l'animation
                float animTime = (CHARGE_PREPARE_DURATION - m_prepareTimer) * baseSpeed;
                float angle = baseAngle + animTime;

                float particleX = playerCenter.x + std::cos(angle) * orbitRadius;
                float particleY = playerCenter.y + std::sin(angle) * orbitRadius;

                // Particule principale
                float particleSize = 4.0f + prepareProgress * 4.0f;
                sf::CircleShape particle(particleSize);
                particle.setPosition(sf::Vector2f(particleX, particleY));
                particle.setOrigin(sf::Vector2f(particleSize, particleSize));
                particle.setFillColor(sf::Color(255, 220, 100, 220));
                window.draw(particle);

                // Halo autour de chaque particule
                sf::CircleShape particleGlow(particleSize * 2.0f);
                particleGlow.setPosition(sf::Vector2f(particleX, particleY));
                particleGlow.setOrigin(sf::Vector2f(particleSize * 2.0f, particleSize * 2.0f));
                particleGlow.setFillColor(sf::Color(255, 180, 50, 80));
                window.draw(particleGlow);
            }

            // Éclair central qui pulse
            float pulseIntensity = std::sin((CHARGE_PREPARE_DURATION - m_prepareTimer) * 20.0f) * 0.5f + 0.5f;
            sf::CircleShape centralGlow(15.0f + pulseIntensity * 10.0f);
            centralGlow.setPosition(playerCenter);
            centralGlow.setOrigin(sf::Vector2f(15.0f + pulseIntensity * 10.0f, 15.0f + pulseIntensity * 10.0f));
            centralGlow.setFillColor(sf::Color(255, 255, 200, static_cast<unsigned char>(150 + pulseIntensity * 100)));
            window.draw(centralGlow);
        }

        // Dessiner les particules d'explosion
        for (const auto& p : m_explosionParticles) {
            // Halo
            sf::CircleShape glow(p.size * 1.5f);
            glow.setPosition(p.position);
            glow.setOrigin(sf::Vector2f(p.size * 1.5f, p.size * 1.5f));
            glow.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, static_cast<unsigned char>(p.life * 100)));
            window.draw(glow);

            // Particule principale
            sf::CircleShape particle(p.size);
            particle.setPosition(p.position);
            particle.setOrigin(sf::Vector2f(p.size, p.size));
            particle.setFillColor(sf::Color(p.color.r, p.color.g, p.color.b, static_cast<unsigned char>(p.life * 255)));
            window.draw(particle);
        }

        // Dessiner la traînée de la charge (effet spectaculaire)
        if (m_isCharging || !m_chargeTrailPositions.empty()) {
            // Traînée avec dégradé de couleur (violet -> orange)
            for (size_t i = 0; i < m_chargeTrailPositions.size(); ++i) {
                float progress = static_cast<float>(i) / static_cast<float>(m_chargeTrailPositions.size());
                float alpha = progress * 255.0f;
                float size = progress * 25.0f + 8.0f;

                // Couleur qui passe de violet/magenta à orange/jaune
                unsigned char r = 255;
                unsigned char g = static_cast<unsigned char>(80 + progress * 175);
                unsigned char b = static_cast<unsigned char>(200 - progress * 180);

                // Halo extérieur (effet de flou)
                sf::CircleShape outerGlow(size * 1.8f);
                outerGlow.setPosition(m_chargeTrailPositions[i]);
                outerGlow.setOrigin(sf::Vector2f(size * 1.8f, size * 1.8f));
                outerGlow.setFillColor(sf::Color(r, g, b, static_cast<unsigned char>(alpha * 0.25f)));
                window.draw(outerGlow);

                // Coeur de la traînée
                sf::CircleShape trailCore(size * 0.6f);
                trailCore.setPosition(m_chargeTrailPositions[i]);
                trailCore.setOrigin(sf::Vector2f(size * 0.6f, size * 0.6f));
                trailCore.setFillColor(sf::Color(255, 255, 220, static_cast<unsigned char>(alpha * 0.9f)));
                window.draw(trailCore);
            }

            // Effet d'aura autour du joueur pendant la charge
            if (m_isCharging) {
                // Cercle d'énergie pulsant
                float pulseSize = 55.0f + std::sin(m_chargeTimer * 40.0f) * 15.0f;
                sf::CircleShape energyAura(pulseSize);
                energyAura.setPosition(playerCenter);
                energyAura.setOrigin(sf::Vector2f(pulseSize, pulseSize));
                energyAura.setFillColor(sf::Color(255, 200, 100, 50));
                energyAura.setOutlineColor(sf::Color(255, 180, 50, 180));
                energyAura.setOutlineThickness(4.0f);
                window.draw(energyAura);

                // Particules d'énergie qui suivent le joueur
                for (int i = 0; i < 10; ++i) {
                    float angle = (m_chargeTimer * 25.0f) + (i * 0.628f);  // 2π/10
                    float radius = 45.0f + std::sin(angle * 3.0f) * 10.0f;
                    float particleX = playerCenter.x + std::cos(angle) * radius;
                    float particleY = playerCenter.y + std::sin(angle) * radius;

                    sf::CircleShape energyParticle(4.0f);
                    energyParticle.setPosition(sf::Vector2f(particleX, particleY));
                    energyParticle.setOrigin(sf::Vector2f(4.0f, 4.0f));
                    energyParticle.setFillColor(sf::Color(255, 230, 120, 230));
                    window.draw(energyParticle);
                }

                // Lignes de vitesse (effet de motion blur) plus prononcées
                float lineDirection = m_facingRight ? -1.0f : 1.0f;
                for (int i = 0; i < 7; ++i) {
                    float lineX = playerCenter.x + lineDirection * (20.0f + i * 25.0f);
                    float lineLength = 50.0f - i * 5.0f;
                    float lineAlpha = 180.0f - i * 22.0f;
                    float lineY = playerCenter.y - 30.0f + i * 10.0f;

                    sf::RectangleShape speedLine(sf::Vector2f(lineLength, 3.0f));
                    speedLine.setPosition(sf::Vector2f(lineX, lineY));
                    speedLine.setFillColor(sf::Color(255, 220, 100, static_cast<unsigned char>(lineAlpha)));
                    window.draw(speedLine);
                }
            }
        }
    }

    // Dessiner le contour rouge pour debug (hitbox du joueur)
    // Pour activer: mettre SHOW_DEBUG_HITBOX à true dans Player.hpp
    if constexpr (SHOW_DEBUG_HITBOX) {
        const float playerWidth = 102.0f;
        const float playerHeight = 102.0f;
        sf::RectangleShape debugRect(sf::Vector2f(playerWidth, playerHeight));
        debugRect.setPosition(m_position);
        debugRect.setFillColor(sf::Color::Transparent);
        debugRect.setOutlineColor(sf::Color::Red);
        debugRect.setOutlineThickness(2.0f);
        window.draw(debugRect);
    }
}

void Player::takeDamage(int damage) {
    // Ne pas prendre de dégâts si invincible
    if (m_invincibilityTimer > 0.0f) {
        return;
    }

    m_health -= damage;
    if (m_health < 0) {
        m_health = 0;
    }

    // Activer l'invincibilité temporaire
    m_invincibilityTimer = INVINCIBILITY_DURATION;

    std::cout << "Player took " << damage << " damage. Health: " << m_health << "/" << m_maxHealth << std::endl;
}

void Player::updateInvincibility(float deltaTime) {
    if (m_invincibilityTimer > 0.0f) {
        m_invincibilityTimer -= deltaTime;
        if (m_invincibilityTimer < 0.0f) {
            m_invincibilityTimer = 0.0f;
        }
    }
}

void Player::triggerDisintegration() {
    if (m_isDisintegrating) return; // Éviter de déclencher plusieurs fois

    m_isDisintegrating = true;

    // Calculer la position centrale du joueur pour l'effet
    const float playerWidth = 102.0f;
    const float playerHeight = 102.0f;
    sf::Vector2f centerPosition = m_position + sf::Vector2f(playerWidth / 2.0f, playerHeight / 2.0f);

    // Créer l'effet de désintégration avec une couleur bleue (couleur du wizard)
    m_particleSystem.createDisintegrationEffect(centerPosition, sf::Color(100, 150, 255));

    std::cout << "Player disintegration triggered!" << std::endl;
}

sf::FloatRect Player::getChargeBounds() const {
    if (!m_isCharging) {
        return sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(0, 0));
    }

    const float playerWidth = 102.0f;
    const float playerHeight = 102.0f;

    // Zone d'attaque étendue devant le joueur pendant la charge
    float attackWidth = 150.0f;  // Largeur de la zone d'attaque
    float attackHeight = playerHeight;

    float attackX;
    if (m_facingRight) {
        attackX = m_position.x;
    } else {
        attackX = m_position.x - attackWidth + playerWidth;
    }

    return sf::FloatRect(
        sf::Vector2f(attackX, m_position.y),
        sf::Vector2f(attackWidth, attackHeight)
    );
}
