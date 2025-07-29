
#include <QApplication>
#include <QMainWindow>
#include <QTabWidget>
#include <QMessageBox>
#include "widgets/dashboardwidget.h"
#include "widgets/facturewidget.h"
#include "utils/databasemanager.h"
#include "controller/controller.h"
#include "utils/defaultusermanager.h"
#include "widgets/authdialog.h"
#include "widgets/resetpassworddialog.h"
#include "auth.h"
#include "utils/mail.h"

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);


    // Connexion à la base de données
    if (!DatabaseManager::connect()) {
        return 1;
    }

    // Configuration de l'expéditeur SMTP WaterSN
    // À personnaliser avec le vrai mot de passe du compte !
    Mail::setSmtpConfig(
        "mail.betaenergyafrique.com", // Serveur SMTP
        465,                           // Port SSL
        "mor@betaenergyafrique.com",  // Adresse expéditeur
        "Yw2v,Yd;^NH~"      // Mot de passe du compte mail
    );

    // Insérer l'utilisateur par défaut si besoin
    DefaultUserManager::ensureDefaultUser();

    // Authentification obligatoire
    bool authenticated = false;
    while (!authenticated) {
        AuthDialog authDialog;
        QObject::connect(&authDialog, &AuthDialog::forgotPasswordClicked, [&]() {
            ResetPasswordDialog resetDialog;
            resetDialog.exec();
        });
        if (authDialog.exec() == QDialog::Accepted) {
            // Vérifier les identifiants
            if (Auth::login(authDialog.getEmail(), authDialog.getPassword())) {
                authenticated = true;
            } else {
                QMessageBox::warning(nullptr, "Erreur", "Email ou mot de passe incorrect.");
            }
        } else {
            // Fermeture du formulaire = quitter l'appli
            return 0;
        }
    }

    // Contrôleur central
    Controller controller;
    // (Optionnel) Ajouter quelques données de test
    controller.ajouterClient(Client(1, "Diallo", "Moussa", "Bamako", "77777777"));
    controller.ajouterClient(Client(2, "Traoré", "Awa", "Bamako", "78888888"));
    controller.ajouterCompteur(Compteur("C001", "disponible", 0, "S1"));
    controller.ajouterCompteur(Compteur("C002", "disponible", 0, "S2"));
    controller.ajouterFacture(Facture(1, "C001", 1, 1000, 12));
    controller.ajouterFacture(Facture(2, "C002", 2, 500, 8));

    // Fenêtre principale avec onglets
    QMainWindow mainWindow;
    QTabWidget* tabs = new QTabWidget;
    // Dashboard
    DashboardWidget* dashboard = new DashboardWidget(&controller);
    tabs->addTab(dashboard, "Dashboard");
    // Factures
    FactureWidget* factureWidget = new FactureWidget(&controller);
    tabs->addTab(factureWidget, "Factures");
    // (Vous pourrez ajouter d'autres widgets/pages ici)
    mainWindow.setCentralWidget(tabs);
    mainWindow.setWindowTitle("Gestion Forage - WaterSN");
    mainWindow.resize(900, 600);
    mainWindow.show();

    return app.exec();
}
