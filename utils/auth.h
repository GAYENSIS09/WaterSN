#ifndef AUTH_H
#define AUTH_H

#include <QString>

class Auth {
public:
    static bool login(const QString& username, const QString& password);
    static void logout();
};

#endif // AUTH_H
