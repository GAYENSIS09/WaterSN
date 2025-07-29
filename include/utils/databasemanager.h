#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>
#include <QSqlError>
#include <QDebug>

class DatabaseManager {
public:
    static bool connect() {
        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
        db.setHostName("localhost");
        db.setDatabaseName("WaterSN");
        db.setUserName("watersn_admin");
        db.setPassword("WaterSN2025!2026");
        if (!db.open()) {
            qCritical() << "Erreur de connexion MySQL:" << db.lastError().text();
            return false;
        }
        qDebug() << "Connexion MySQL réussie !";
        return true;
    }
};

#endif // DATABASEMANAGER_H
