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
    // Authentification avec récupération de la taille et de l'état de la fenêtre
    QSize authSize(800, 650);
    bool wasMaximized = false;
    bool authenticated = false;
    while (!authenticated) {
        AuthDialog authDialog;
        QObject::connect(&authDialog, &AuthDialog::forgotPasswordClicked, [&]() {
            ResetPasswordDialog resetDialog;
            resetDialog.exec();
        });
        authDialog.resize(authSize); // Appliquer la dernière taille connue
        if (wasMaximized) authDialog.showMaximized();
        int result = authDialog.exec();
        // Sauvegarder la taille et l'état avant de fermer
        authSize = authDialog.size();
        wasMaximized = authDialog.isMaximized();
        if (result == QDialog::Accepted) {
            if (Auth::login(authDialog.getEmail(), authDialog.getPassword())) {
                authenticated = true;
            } else {
                QMessageBox::warning(nullptr, "Erreur", "Email ou mot de passe incorrect.");
            }
        } else {
            return 0;
        }
    }

    // Contrôleur central
    Controller controller;
   
    // Fenêtre principale : afficher uniquement le Dashboard
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("Gestion Forage - WaterSN");
    // Appliquer la taille et l'état de la fenêtre d'authentification
    mainWindow.resize(authSize);
    if (wasMaximized) mainWindow.showMaximized();
    else mainWindow.show();
    // S'assurer que la fenêtre est visible et a une géométrie correcte avant de créer le dashboard
    QApplication::processEvents();
    DashboardWidget* dashboard = new DashboardWidget(&controller, &mainWindow);
    mainWindow.setCentralWidget(dashboard);

    return app.exec();
}
