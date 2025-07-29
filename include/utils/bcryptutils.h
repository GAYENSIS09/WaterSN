#ifndef BCRYPTUTILS_H
#define BCRYPTUTILS_H

#include <QString>

class BcryptUtils {
public:
    static QString hashPassword(const QString& password);
};

#endif // BCRYPTUTILS_H
