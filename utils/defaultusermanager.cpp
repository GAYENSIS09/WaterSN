#include "utils/defaultusermanager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include "utils/bcryptutils.h"

void DefaultUserManager::ensureDefaultUser() {
    QSqlQuery query;
    // Vérifier si l'utilisateur existe déjà
    query.prepare("SELECT id FROM User WHERE email = :email");
    query.bindValue(":email", "admin@watersn.com");
    int userId = -1;
    if (query.exec() && query.next()) {
        userId = query.value(0).toInt();
    }
    if (userId == -1) {
        // Insérer l'utilisateur par défaut
        QString hashed = BcryptUtils::hashPassword("WaterSN2025!2026");
        QSqlQuery insert;
        insert.prepare("INSERT INTO User (nom, prenom, email, civilite, password) VALUES (:nom, :prenom, :email, :civilite, :password)");
        insert.bindValue(":nom", "Admin");
        insert.bindValue(":prenom", "WaterSN");
        insert.bindValue(":email", "admin@watersn.com");
        insert.bindValue(":civilite", "admin");
        insert.bindValue(":password", hashed);
        if (!insert.exec()) {
            qWarning() << "Erreur insertion utilisateur par défaut:" << insert.lastError().text();
            return;
        } else {
            qDebug() << "Utilisateur par défaut inséré.";
        }
        userId = insert.lastInsertId().toInt();
    }
    // Ajouter les rôles admin et agent
    int adminRoleId = -1, agentRoleId = -1;
    QSqlQuery roleQ;
    roleQ.prepare("SELECT id, nom FROM Role WHERE nom IN ('admin','agent')");
    if (roleQ.exec()) {
        while (roleQ.next()) {
            QString nom = roleQ.value(1).toString();
            if (nom == "admin") adminRoleId = roleQ.value(0).toInt();
            if (nom == "agent") agentRoleId = roleQ.value(0).toInt();
        }
    }
    if (adminRoleId == -1) {
        QSqlQuery rq;
        rq.prepare("INSERT INTO Role (nom) VALUES ('admin')");
        rq.exec();
        adminRoleId = rq.lastInsertId().toInt();
    }
    if (agentRoleId == -1) {
        QSqlQuery rq;
        rq.prepare("INSERT INTO Role (nom) VALUES ('agent')");
        rq.exec();
        agentRoleId = rq.lastInsertId().toInt();
    }
    // Associer l'utilisateur aux rôles
    for (int roleId : {adminRoleId, agentRoleId}) {
        QSqlQuery check;
        check.prepare("SELECT 1 FROM UserRole WHERE user_id = :uid AND role_id = :rid");
        check.bindValue(":uid", userId);
        check.bindValue(":rid", roleId);
        if (check.exec() && !check.next()) {
            QSqlQuery ins;
            ins.prepare("INSERT INTO UserRole (user_id, role_id) VALUES (:uid, :rid)");
            ins.bindValue(":uid", userId);
            ins.bindValue(":rid", roleId);
            ins.exec();
        }
    }
}
