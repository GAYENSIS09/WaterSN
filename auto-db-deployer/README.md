# Auto DB Deployer

## Description
Auto DB Deployer est une application multiplateforme conçue pour déployer automatiquement une base de données SQL lors de l'installation du logiciel. Ce projet facilite le partage et les tests de l'application avec vos camarades en automatisant le processus de configuration de la base de données.

## Structure du projet
Le projet est organisé comme suit :

```
auto-db-deployer
├── src
│   ├── main.cpp                # Point d'entrée de l'application
│   ├── database
│   │   ├── init.sql            # Instructions SQL pour initialiser la base de données
│   │   └── dbmanager.cpp        # Gestion des opérations de base de données
│   ├── platform
│   │   ├── linux.cpp           # Code spécifique à la plateforme Linux
│   │   ├── windows.cpp         # Code spécifique à la plateforme Windows
│   │   └── macos.cpp           # Code spécifique à la plateforme macOS
│   └── utils
│       └── fileutils.cpp       # Fonctions utilitaires pour la gestion des fichiers
├── CMakeLists.txt              # Configuration du projet avec CMake
├── README.md                    # Documentation du projet
└── package.json                 # Gestion des dépendances et des scripts
```

## Compilation et Exécution
Pour compiler et exécuter l'application, suivez ces étapes :

1. **Clonez le dépôt** :
   ```bash
   git clone <url_du_dépôt>
   cd auto-db-deployer
   ```

2. **Créez un répertoire de construction** :
   ```bash
   mkdir build
   cd build
   ```

3. **Configurez le projet avec CMake** :
   ```bash
   cmake ..
   ```

4. **Compilez le projet** :
   ```bash
   make
   ```

5. **Exécutez l'application** :
   ```bash
   ./auto-db-deployer
   ```

## Plateformes prises en charge
L'application est conçue pour fonctionner sur les systèmes d'exploitation suivants :
- Linux
- Windows
- macOS

## Contribuer
Les contributions sont les bienvenues ! N'hésitez pas à soumettre des demandes de tirage ou à signaler des problèmes.

## License
Ce projet est sous licence MIT. Veuillez consulter le fichier LICENSE pour plus de détails.