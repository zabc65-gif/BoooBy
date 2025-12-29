# BoooBy

Remake du jeu "Sheepy: A Short Adventure" - Un jeu de plateforme 2.5D avec une ambiance sombre et féerique.

## Description

BoooBy est un remake inspiré de "Sheepy: A Short Adventure", mettant en scène un mouton anthropomorphe dans un environnement de plateforme 2D avec rendu 3D. Le jeu met l'accent sur les graphismes, l'ambiance sonore et musicale pour créer une expérience immersive et courte.

### Fonctionnalités

- Mouvement fluide (marche, course, saut)
- Système de pouvoirs
- Physique de plateforme réaliste
- Ambiance sombre et féerique

## Prérequis

- CMake 3.16 ou supérieur
- Compilateur C++17 (GCC, Clang, MSVC)
- SFML 2.5 ou supérieur

### Installation de SFML

#### macOS
```bash
brew install sfml
```

#### Ubuntu/Debian
```bash
sudo apt-get install libsfml-dev
```

#### Windows
Télécharger SFML depuis [sfml-dev.org](https://www.sfml-dev.org/download.php) et configurer les chemins dans CMake.

## Structure du projet

```
BoooBy/
├── src/              # Code source C++
│   ├── main.cpp      # Point d'entrée
│   ├── Game.cpp      # Boucle de jeu principale
│   └── Player.cpp    # Logique du joueur
├── include/          # Fichiers d'en-tête (.hpp)
│   ├── Game.hpp
│   └── Player.hpp
├── assets/           # Ressources (images, sons, musiques)
├── build/            # Fichiers de compilation (généré)
├── CMakeLists.txt    # Configuration CMake
└── README.md         # Ce fichier
```

## Compilation

### Créer le dossier de build
```bash
mkdir build
cd build
```

### Générer les fichiers de build avec CMake
```bash
cmake ..
```

### Compiler le projet
```bash
cmake --build .
```

L'exécutable sera généré dans `build/bin/`.

## Utilisation

### Lancer le jeu
```bash
./build/bin/BoooBy
```

### Contrôles

- **Flèches Gauche/Droite** ou **Q/D** : Déplacer le personnage
- **Espace** : Sauter
- **Shift** : Courir
- **Échap** : Pause

## État du développement

### Fonctionnalités actuelles
- ✅ Structure de base du projet
- ✅ Boucle de jeu principale (60 FPS)
- ✅ Système de mouvement du joueur
- ✅ Physique de base (gravité, collision avec le sol)
- ✅ États du joueur (idle, marche, course, saut, chute)

### À venir
- 🔲 Chargement et affichage de sprites/textures
- 🔲 Système d'animation
- 🔲 Niveaux et décors
- 🔲 Système de pouvoirs
- 🔲 Ennemis et obstacles
- 🔲 Musique et effets sonores
- 🔲 Interface utilisateur
- 🔲 Système de sauvegarde

## License

Projet personnel - Remake non-commercial inspiré de "Sheepy: A Short Adventure".
