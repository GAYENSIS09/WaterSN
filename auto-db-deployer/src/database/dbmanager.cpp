#include "dbmanager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>

class DBManager {
public:
    DBManager(const QString& dbName);
    bool connect();
    bool executeScript(const QString& scriptPath);
    void close();

private:
    QSqlDatabase db;
};

DBManager::DBManager(const QString& dbName) {
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbName);
}

bool DBManager::connect() {
    if (!db.open()) {
        qWarning() << "Erreur de connexion à la base de données:" << db.lastError().text();
        return false;
    }
    return true;
}

bool DBManager::executeScript(const QString& scriptPath) {
    QFile scriptFile(scriptPath);
    if (!scriptFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Erreur lors de l'ouverture du fichier de script:" << scriptFile.errorString();
        return false;
    }

    QString script = scriptFile.readAll();
    scriptFile.close();

    QSqlQuery query;
    if (!query.exec(script)) {
        qWarning() << "Erreur lors de l'exécution du script SQL:" << query.lastError().text();
        return false;
    }
    return true;
}

void DBManager::close() {
    db.close();
}