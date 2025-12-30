# Mossy Tileset - Référence des Tuiles

**Tileset**: Mossy - TileSet.png
**Dimensions**: 3584x3584 pixels
**Taille d'une section**: 256x256 pixels
**Sections par ligne**: 14
**Sections par colonne**: 14
**Total de sections**: 196 (14x14)

## Affichage dans le jeu
- Échelle: 0.25 (chaque section de 256x256px est affichée en 64x64px)
- Taille des tiles à l'écran: 64x64 pixels

---

## Ligne 0 (Sections 0-13) - Sol avec herbe

| ID  | Position | Description | Type de collision | Profondeur herbe |
|-----|----------|-------------|-------------------|------------------|
| 0   | (0,0)    | À définir | À définir | - |
| 1   | (1,0)    | **Angle supérieur gauche du sol** | Solide | 25% (16px) |
| 2   | (2,0)    | **Sol milieu - variante 1** | Solide | 25% (16px) |
| 3   | (3,0)    | **Sol milieu - variante 2** | Solide | 25% (16px) |
| 4   | (4,0)    | **Sol milieu - variante 3** | Solide | 25% (16px) |
| 5   | (5,0)    | **Sol milieu - variante 4** | Solide | 25% (16px) |
| 6   | (6,0)    | **Angle supérieur droit du sol** | Solide | 25% (16px) |
| 7   | (7,0)    | À définir | À définir | - |
| 8   | (8,0)    | À définir | À définir | - |
| 9   | (9,0)    | À définir | À définir | - |
| 10  | (10,0)   | À définir | À définir | - |
| 11  | (11,0)   | À définir | À définir | - |
| 12  | (12,0)   | À définir | À définir | - |
| 13  | (13,0)   | À définir | À définir | - |

## Ligne 1 (Sections 14-27)

| ID  | Position | Description | Type de collision | Notes |
|-----|----------|-------------|-------------------|-------|
| 14  | (0,1)    | À définir | À définir | - |
| 15  | (1,1)    | À définir | À définir | - |
| 16  | (2,1)    | À définir | À définir | - |
| 17  | (3,1)    | À définir | À définir | - |
| 18  | (4,1)    | À définir | À définir | - |
| 19  | (5,1)    | À définir | À définir | - |
| 20  | (6,1)    | À définir | À définir | - |
| 21  | (7,1)    | À définir | À définir | - |
| 22  | (8,1)    | À définir | À définir | - |
| 23  | (9,1)    | À définir | À définir | - |
| 24  | (10,1)   | À définir | À définir | - |
| 25  | (11,1)   | À définir | À définir | - |
| 26  | (12,1)   | À définir | À définir | - |
| 27  | (13,1)   | À définir | À définir | - |

## Ligne 2 (Sections 28-41)

| ID  | Position | Description | Type de collision | Notes |
|-----|----------|-------------|-------------------|-------|
| 28  | (0,2)    | À définir | À définir | - |
| 29  | (1,2)    | À définir | À définir | - |
| 30  | (2,2)    | À définir | À définir | - |
| 31  | (3,2)    | À définir | À définir | - |
| 32  | (4,2)    | À définir | À définir | - |
| 33  | (5,2)    | À définir | À définir | - |
| 34  | (6,2)    | À définir | À définir | - |
| 35  | (7,2)    | À définir | À définir | - |
| 36  | (8,2)    | À définir | À définir | - |
| 37  | (9,2)    | À définir | À définir | - |
| 38  | (10,2)   | À définir | À définir | - |
| 39  | (11,2)   | À définir | À définir | - |
| 40  | (12,2)   | À définir | À définir | - |
| 41  | (13,2)   | À définir | À définir | - |

## Lignes 3-13 (Sections 42-195)

*À compléter après inspection visuelle du tileset*

---

## Types de collision

### 1. **Solide** (solid)
- Bloque complètement le joueur
- Utilisé pour les sols, murs, plateformes

### 2. **Vide** (empty)
- Aucune collision
- Le joueur peut passer à travers

### 3. **Plateforme** (platform)
- Collision uniquement par le dessus
- Le joueur peut sauter à travers par en dessous

### 4. **Demi-bloc** (half-block)
- Collision sur la moitié de la tile
- Utilisé pour des variations de terrain

---

## Notes d'implémentation

### Profondeur d'herbe
- **25% du haut de la tile** (16px sur 64px)
- Le joueur marche "dans" l'herbe pour un effet visuel naturel
- Formule: `grassDepth = tileSize * 0.25f`

### Offset des pieds du joueur
- **25 pixels du bas** du sprite
- Les vrais pieds sont plus hauts que le bas du sprite
- Formule: `feetOffset = 25.0f`

### Calcul de position
```cpp
float feetY = position.y + playerHeight - feetOffset;
int bottomTile = static_cast<int>(feetY / tileSize);
float groundY = bottomTile * tileSize + grassDepth - (playerHeight - feetOffset);
```

---

## TODO
- [ ] Identifier visuellement toutes les sections du tileset
- [ ] Catégoriser chaque tile (sol, mur, plateforme, décor, etc.)
- [ ] Définir les règles de collision pour chaque type
- [ ] Ajouter des notes sur les tiles animées (si présentes)
- [ ] Documenter les tiles de décoration (sans collision)
- [ ] Créer des groupes logiques (ex: coins, bords, milieux)
