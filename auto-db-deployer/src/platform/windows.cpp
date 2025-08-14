#include <iostream>
#include <cstdlib>
#include "dbmanager.h"

void deployDatabase() {
    std::cout << "Déploiement de la base de données sur Windows..." << std::endl;

    // Commande pour exécuter le script SQL
    std::string command = "sqlcmd -S .\\SQLEXPRESS -d master -U sa -P your_password -i ../database/init.sql";
    
    // Exécution de la commande
    int result = system(command.c_str());
    
    if (result == 0) {
        std::cout << "Base de données déployée avec succès." << std::endl;
    } else {
        std::cerr << "Erreur lors du déploiement de la base de données." << std::endl;
    }
}