
#include "auth.h"
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include "utils/bcryptutils.h"

bool Auth::login(const QString& username, const QString& password) {
    QSqlQuery query;
    query.prepare("SELECT password FROM User WHERE email = :email");
    query.bindValue(":email", username.trimmed());
    if (!query.exec() || !query.next()) {
        return false;
    }
    QString storedHash = query.value(0).toString();
    QString inputHash = BcryptUtils::hashPassword(password);
    return storedHash == inputHash;
}

void Auth::logout() {
    // ...
}
