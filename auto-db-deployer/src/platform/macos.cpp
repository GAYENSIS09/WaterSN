#include <iostream>
#include "dbmanager.h"

void installDatabase() {
    // Code spécifique à macOS pour installer la base de données
    std::cout << "Installation de la base de données sur macOS..." << std::endl;

    // Appel à la fonction pour exécuter le script SQL d'initialisation
    if (!DatabaseManager::executeScript("src/database/init.sql")) {
        std::cerr << "Erreur lors de l'exécution du script d'initialisation." << std::endl;
    } else {
        std::cout << "Base de données initialisée avec succès." << std::endl;
    }
}