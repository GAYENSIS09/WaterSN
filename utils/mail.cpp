#include "mail.h"
#include <QTextStream>
#include <QDebug>
#include <QByteArray>
#include <QCoreApplication>
#include <QThread>
#include <QString>
#include <QSslSocket>

// Définition des variables statiques
QString Mail::smtpServer_ = "";
int Mail::smtpPort_ = 465;
QString Mail::smtpUser_ = "";
QString Mail::smtpPassword_ = "";

// Définition de la méthode de configuration SMTP
void Mail::setSmtpConfig(const QString &smtpServer, int port, const QString &user, const QString &password)
{
    smtpServer_ = smtpServer;
    smtpPort_ = port;
    smtpUser_ = user;
    smtpPassword_ = password;
}

// Fonctions utilitaires internes
static bool waitForReady(QSslSocket * socket, int timeout = 10000)
{
    return socket->waitForReadyRead(timeout);
}

static bool sendLine(QSslSocket * socket, const QString &line)
{
    QByteArray data = line.toUtf8() + "\r\n";
    return socket->write(data) == data.size() && socket->waitForBytesWritten(5000);
}

static QString readLine(QSslSocket * socket)
{
    if (!socket->canReadLine() && !socket->waitForReadyRead(5000))
        return "";
    return QString::fromUtf8(socket->readLine()).trimmed();
}

// Définition de la méthode d'envoi de mail
bool Mail::sendMail(const QString &to, const QString &subject, const QString &body)
{
    QSslSocket socket;
    socket.connectToHostEncrypted(smtpServer_, smtpPort_);
    if (!socket.waitForEncrypted(10000))
    {
        qWarning() << "[Mail] Connexion SSL échouée:" << socket.errorString();
        return false;
    }
    if (!waitForReady(&socket))
        return false;
    QString banner = readLine(&socket);
    if (!banner.startsWith("220"))
        return false;

    // EHLO
    sendLine(&socket, "EHLO localhost");
    while (true)
    {
        QString l = readLine(&socket);
        if (!l.startsWith("250-"))
            break;
    }

    // AUTH LOGIN
    sendLine(&socket, "AUTH LOGIN");
    if (!readLine(&socket).startsWith("334"))
        return false;
    sendLine(&socket, smtpUser_.toUtf8().toBase64());
    if (!readLine(&socket).startsWith("334"))
        return false;
    sendLine(&socket, smtpPassword_.toUtf8().toBase64());
    if (!readLine(&socket).startsWith("235"))
        return false;

    // MAIL FROM
    sendLine(&socket, "MAIL FROM:<" + smtpUser_ + ">");
    if (!readLine(&socket).startsWith("250"))
        return false;
    // RCPT TO
    sendLine(&socket, "RCPT TO:<" + to + ">");
    if (!readLine(&socket).startsWith("250"))
        return false;
    // DATA
    sendLine(&socket, "DATA");
    if (!readLine(&socket).startsWith("354"))
        return false;

    // Corps du mail
    QString mailData;
    mailData += "From: <" + smtpUser_ + ">\r\n";
    mailData += "To: <" + to + ">\r\n";
    mailData += "Subject: " + subject + "\r\n";
    mailData += "MIME-Version: 1.0\r\n";
    mailData += "Content-Type: text/plain; charset=UTF-8\r\n";
    mailData += "\r\n";
    mailData += body + "\r\n";
    mailData += ".\r\n";
    sendLine(&socket, mailData);
    if (!readLine(&socket).startsWith("250"))
        return false;

    // QUIT
    sendLine(&socket, "QUIT");
    socket.disconnectFromHost();
    return true;
}