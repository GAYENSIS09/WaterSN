#include <iostream>
#include "database/dbmanager.cpp"
#include "platform/linux.cpp" // Include the appropriate platform file based on the OS
#include "platform/windows.cpp"
#include "platform/macos.cpp"

int main() {
    std::cout << "Démarrage de l'application..." << std::endl;

    // Initialiser le gestionnaire de base de données
    DbManager dbManager;

    // Exécuter le script SQL pour initialiser la base de données
    if (!dbManager.initializeDatabase()) {
        std::cerr << "Échec de l'initialisation de la base de données." << std::endl;
        return 1;
    }

    std::cout << "Base de données initialisée avec succès." << std::endl;

    // Autres logiques d'application peuvent être ajoutées ici

    return 0;
}