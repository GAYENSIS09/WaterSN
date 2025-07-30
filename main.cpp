#include <QFile>
#include <QTextStream>
#include <cstdlib>

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

// Fonction utilitaire pour charger les variables d'un fichier .env
void chargerDotEnv(const QString& chemin)
{
    QFile file(chemin);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith("#") || !line.contains("="))
            continue;
        QStringList parts = line.split('=', Qt::KeepEmptyParts);
        if (parts.size() < 2)
            continue;
        QByteArray key = parts[0].trimmed().toUtf8();
        QByteArray value = parts.mid(1).join("=").trimmed().toUtf8();
        if (qgetenv(key.constData()).isEmpty()) {
            qputenv(key.constData(), value);
        }
    }
}

int main(int argc, char *argv[])
{

    QApplication app(argc, argv);


    // Connexion à la base de données
    if (!DatabaseManager::connect()) {
        return 1;
    }

    // Charger automatiquement le .env du dossier de build ou du dossier source
    chargerDotEnv(QCoreApplication::applicationDirPath() + "/.env");
    chargerDotEnv(QCoreApplication::applicationDirPath() + "/../.env");

    // Configuration SMTP Mailjet via variables d'environnement
    QString mailjetUser = QString::fromUtf8(qgetenv("MAILJET_USER"));
    QString mailjetPass = QString::fromUtf8(qgetenv("MAILJET_PASS"));
    if (mailjetUser.isEmpty() || mailjetPass.isEmpty()) {
        QMessageBox::critical(nullptr, "Erreur Mailjet", "Les identifiants Mailjet ne sont pas définis dans les variables d'environnement (.env non trouvé ou incomplet).");
        return 1;
    }
    Mail::setSmtpConfig(
        "in-v3.mailjet.com",
        587,
        mailjetUser,
        mailjetPass
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

    // Fenêtre principale : afficher uniquement le Dashboard
    QMainWindow mainWindow;
    DashboardWidget* dashboard = new DashboardWidget(&controller);
    mainWindow.setCentralWidget(dashboard);
    mainWindow.setWindowTitle("Gestion Forage - WaterSN");
    mainWindow.resize(900, 600);
    mainWindow.show();

    return app.exec();
}
