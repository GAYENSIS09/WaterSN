#pragma once
#include <QString>

class Mail {
public:
static bool sendMail(const QString &to, const QString &subject, const QString &body, QString* errorMsg = nullptr);
static void setSmtpConfig(const QString &smtpServer, int port, const QString &user, const QString &password);

private:
    static QString smtpServer_;
    static int smtpPort_;
    static QString smtpUser_;
    static QString smtpPassword_;
};