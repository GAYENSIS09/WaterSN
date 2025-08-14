#include <iostream>
#include <cstdlib>
#include "dbmanager.h"

void installDatabase() {
    std::cout << "Installation de la base de données pour Linux..." << std::endl;

    // Exécution du script SQL d'initialisation
    if (DBManager::executeScript("src/database/init.sql")) {
        std::cout << "Base de données initialisée avec succès." << std::endl;
    } else {
        std::cerr << "Erreur lors de l'initialisation de la base de données." << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

int main() {
    installDatabase();
    return 0;
}