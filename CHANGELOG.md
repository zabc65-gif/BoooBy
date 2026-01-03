# BoooBee - Changelog

## Version 1.0 - État Stable (2026-01-03)

### Fonctionnalités Complètes

#### Système de Niveaux
- **Niveau Prologue** : Tutoriel avec porte médiévale à l'entrée et portail de sortie
- **Progression de niveaux** : Système de transition automatique après 3 secondes
- **Portails** : Entrée (porte médiévale) et sortie (portail cyan avec effets visuels)
- **Détection de fin** : Le joueur doit atteindre le centre horizontal du portail de sortie

#### Physique et Collisions
- Collisions sol (pixel-perfect avec boîtes de collision personnalisées)
- Collisions murs (centrées sur le joueur)
- Collisions plafond
- Physique de chute améliorée et fluide
- Hitbox de debug visualisable

#### Interface Utilisateur
- **Menu pause** : Accessible avec ÉCHAP, fonctionnel pendant le jeu
- **Écran de fin de niveau** : "Niveau Termine !" avec transition de 3 secondes
- **Écran de victoire finale** : "Felicitations! Tous les niveaux sont termines!"
  - ENTRÉE pour rejouer
  - ÉCHAP pour quitter
- **Éditeur de niveaux** : Accessible avec 'E', permet de créer/modifier des niveaux

#### Assets
- Tileset Mossy pour les niveaux
- Porte médiévale (Medieval_door_large.png - 156x312 pixels, 3x hauteur du joueur)
- Portails visuels avec effets de lueur cyan

### Corrections Importantes

#### Bug de Transition de Niveau (RÉSOLU)
**Symptôme** : Le niveau suivant ne se chargeait pas après les 3 secondes d'attente
**Cause** : Instruction `return` prématurée dans `Game.cpp:212` qui bloquait le code de transition
**Solution** : Réorganisation du code pour vérifier la transition AVANT le return

#### Bug Menu Victoire Finale (RÉSOLU)
**Symptôme** : La touche ENTRÉE ne relançait pas le jeu quand tous les niveaux étaient terminés
**Cause** : `processEvents()` ne vérifiait pas `m_isGameComplete` avant d'appeler `handleMenuInput()`
**Solution** : Ajout de la condition de vérification dans `processEvents()` ligne 139

### Architecture Technique

#### Fichiers Principaux
- `src/Game.cpp` : Boucle de jeu principale, gestion des états, transitions
- `src/Level.cpp` : Chargement des niveaux, collisions, rendu des portails
- `src/Player.cpp` : Physique du joueur, mouvement, sauts
- `src/LevelEditor.cpp` : Éditeur de niveaux intégré
- `include/*.hpp` : Headers correspondants

#### Format de Niveau (JSON)
```json
{
  "width": 30,
  "height": 20,
  "tiles": [[...]],
  "enemies": [],
  "exitPortal": {"x": 27, "y": 18},
  "entrancePortal": {"x": 3, "y": 18}
}
```

#### Système de Portails
- **Portail d'entrée** : Affiche une porte médiévale, positionne le joueur au centre
- **Portail de sortie** : Affiche 3 cercles concentriques cyan avec effets de lueur
- **Détection** : Collision horizontale uniquement (centre X ± 20 pixels), toute hauteur

### Configuration Technique
- **Moteur** : SFML 3.0
- **Langage** : C++
- **Taille des tuiles** : 64x64 pixels
- **Taille du joueur** : 102 pixels (largeur et hauteur)
- **Taille de la porte** : 156x312 pixels (3x hauteur du joueur)

### Points de Repère du Code

#### Détection de Fin de Niveau
`src/Level.cpp:344-364` - Fonction `isPlayerAtFinish()`
```cpp
// Vérification horizontale uniquement (centre X)
float playerCenterX = playerPos.x + playerWidth / 2.0f;
float portalCenterX = m_exitPortalPosition.x + 32.0f;
float dx = std::abs(playerCenterX - portalCenterX);
return dx < 20.0f;
```

#### Transition de Niveau
`src/Game.cpp:212-232` - Logique de transition avec timer de 3 secondes
```cpp
static sf::Clock transitionClock;
static bool transitionStarted = false;

if (m_isFinished) {
    if (!transitionStarted) {
        transitionClock.restart();
        transitionStarted = true;
    }

    if (transitionClock.getElapsedTime().asSeconds() >= 3.0f) {
        transitionStarted = false;
        loadNextLevel();
    }
    return;
}
```

#### Positionnement du Joueur
`src/Game.cpp:375-379` - Centre le joueur sur le portail d'entrée
```cpp
sf::Vector2f portalPos = m_level->getEntrancePortalPosition();
playerStartPos.x = portalPos.x + 32.0f - 51.0f;  // Centre - demi largeur
playerStartPos.y = portalPos.y + 32.0f - 51.0f;  // Centre - demi hauteur
```

### État du Projet
✅ Compilé et fonctionnel
✅ Tous les bugs connus résolus
✅ Système de niveaux opérationnel
✅ Menu et navigation fonctionnels
✅ Éditeur de niveaux intégré

### Prochaines Étapes Potentielles
- Ajouter plus de niveaux
- Implémenter des ennemis
- Ajouter des power-ups
- Améliorer les effets visuels
- Ajouter du son et de la musique
