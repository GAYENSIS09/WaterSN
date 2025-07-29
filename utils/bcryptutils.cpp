#include "utils/bcryptutils.h"
#include <QByteArray>
#include <QCryptographicHash>

// Remplacement temporaire : SHA-256 (à remplacer par une vraie lib bcrypt en prod)
QString BcryptUtils::hashPassword(const QString& password) {
    QByteArray hash = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return QString::fromUtf8(hash.toHex());
}
