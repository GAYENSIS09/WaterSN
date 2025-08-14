# WaterSN - Guide d'installation et d'utilisation

## Description
WaterSN est une application de gestion de facturation d'eau multiplateforme.

## Installation (Linux)
1. Décompressez l'archive :
   ```bash
   tar xzvf WaterSN_Linux.tar.gz
   cd WaterSN_Linux
   ```
2. (Optionnel) Placez le raccourci `WaterSN.desktop` sur votre bureau :
   ```bash
   cp WaterSN.desktop ~/Bureau/
   chmod +x ~/Bureau/WaterSN.desktop
   ```
3. Double-cliquez sur l'icône WaterSN sur le bureau, ou lancez dans un terminal :
   ```bash
   ./WaterSN
   ```

## Prérequis
- Aucune installation de Qt n'est nécessaire, toutes les bibliothèques sont incluses.
- Pour la base de données, suivez les instructions affichées au premier lancement.

## Fichiers inclus
- WaterSN : exécutable principal
- logo_watersn.png : icône de l'application
- WaterSN.desktop : raccourci pour le bureau
- icons/ : icônes utilisées par l'application
- WaterSN_schema.sql : script SQL de création de la base

## Support
Contactez l'équipe WaterSN pour toute question ou bug.
