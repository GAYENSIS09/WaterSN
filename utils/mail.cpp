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
bool Mail::sendMail(const QString &to, const QString &subject, const QString &body, QString* errorMsg)
{
    QSslSocket socket;
    qDebug() << "[Mail] Connexion au serveur SMTP:" << smtpServer_ << ":" << smtpPort_;
    QString ehloDomain = smtpServer_;
    if (smtpServer_.contains("mailjet", Qt::CaseInsensitive))
        ehloDomain = "in-v3.mailjet.com";
    else if (smtpServer_.contains("gmail", Qt::CaseInsensitive))
        ehloDomain = "gmail.com";
    // Connexion socket
    if (smtpPort_ == 587) {
        socket.connectToHost(smtpServer_, smtpPort_);
        if (!socket.waitForConnected(10000)) {
            qWarning() << "[Mail] Connexion TCP échouée:" << socket.errorString();
            if (errorMsg) *errorMsg = socket.errorString();
            return false;
        }
        qDebug() << "[Mail] Connexion TCP établie (port 587).";
        if (!waitForReady(&socket)) {
            qWarning() << "[Mail] Pas de réponse du serveur après connexion.";
            if (errorMsg) *errorMsg = "Pas de réponse du serveur après connexion.";
            return false;
        }
        QString banner = readLine(&socket);
        qDebug() << "[Mail] Bannière serveur:" << banner;
        if (!banner.startsWith("220")) {
            qWarning() << "[Mail] Bannière inattendue: " << banner;
            if (errorMsg) *errorMsg = banner;
            return false;
        }
        // EHLO
        qDebug() << "[Mail] Envoi: EHLO" << ehloDomain;
        sendLine(&socket, "EHLO " + ehloDomain);
        while (true) {
            QString l = readLine(&socket);
            qDebug() << "[Mail] Réponse EHLO:" << l;
            if (!l.startsWith("250-"))
                break;
        }
        // STARTTLS
        qDebug() << "[Mail] Envoi: STARTTLS";
        sendLine(&socket, "STARTTLS");
        QString resp = readLine(&socket);
        qDebug() << "[Mail] Réponse STARTTLS:" << resp;
        if (!resp.startsWith("220")) {
            qWarning() << "[Mail] STARTTLS échoué: " << resp;
            if (errorMsg) *errorMsg = resp;
            return false;
        }
        socket.startClientEncryption();
        if (!socket.waitForEncrypted(10000)) {
            qWarning() << "[Mail] Négociation TLS échouée:" << socket.errorString();
            if (errorMsg) *errorMsg = socket.errorString();
            return false;
        }
        qDebug() << "[Mail] Connexion TLS établie (après STARTTLS).";
        // Re-EHLO après STARTTLS
        qDebug() << "[Mail] Envoi: EHLO" << ehloDomain << "(après STARTTLS)";
        sendLine(&socket, "EHLO " + ehloDomain);
        while (true) {
            QString l = readLine(&socket);
            qDebug() << "[Mail] Réponse EHLO:" << l;
            if (!l.startsWith("250-"))
                break;
        }
    } else {
        socket.connectToHostEncrypted(smtpServer_, smtpPort_);
        if (!socket.waitForEncrypted(10000)) {
            qWarning() << "[Mail] Connexion SSL échouée:" << socket.errorString();
            if (errorMsg) *errorMsg = socket.errorString();
            return false;
        }
        qDebug() << "[Mail] Connexion SSL établie.";
        if (!waitForReady(&socket)) {
            qWarning() << "[Mail] Pas de réponse du serveur après connexion.";
            if (errorMsg) *errorMsg = "Pas de réponse du serveur après connexion.";
            return false;
        }
        QString banner = readLine(&socket);
        qDebug() << "[Mail] Bannière serveur:" << banner;
        if (!banner.startsWith("220")) {
            qWarning() << "[Mail] Bannière inattendue: " << banner;
            if (errorMsg) *errorMsg = banner;
            return false;
        }
        // EHLO
        qDebug() << "[Mail] Envoi: EHLO" << ehloDomain;
        sendLine(&socket, "EHLO " + ehloDomain);
        while (true) {
            QString l = readLine(&socket);
            qDebug() << "[Mail] Réponse EHLO:" << l;
            if (!l.startsWith("250-"))
                break;
        }
    }

    // AUTH LOGIN (Mailjet: user = API Key, pass = Secret)
    qDebug() << "[Mail] Envoi: AUTH LOGIN";
    sendLine(&socket, "AUTH LOGIN");
    QString resp = readLine(&socket);
    qDebug() << "[Mail] Réponse AUTH LOGIN:" << resp;
    if (!resp.startsWith("334")) {
        qWarning() << "[Mail] AUTH LOGIN échoué: " << resp;
        if (errorMsg) *errorMsg = resp;
        return false;
    }
    qDebug() << "[Mail] Envoi: USER (base64)";
    sendLine(&socket, smtpUser_.toUtf8().toBase64());
    resp = readLine(&socket);
    qDebug() << "[Mail] Réponse USER:" << resp;
    if (!resp.startsWith("334")) {
        qWarning() << "[Mail] USER non accepté: " << resp;
        if (errorMsg) *errorMsg = resp;
        return false;
    }
    qDebug() << "[Mail] Envoi: PASSWORD (base64)";
    sendLine(&socket, smtpPassword_.toUtf8().toBase64());
    resp = readLine(&socket);
    qDebug() << "[Mail] Réponse PASSWORD:" << resp;
    if (!resp.startsWith("235")) {
        qWarning() << "[Mail] Authentification échouée: " << resp;
        if (errorMsg) *errorMsg = resp;
        return false;
    }

    // MAIL FROM (Mailjet: From doit être une adresse validée sur Mailjet, souvent smtpUser_)
    QString fromAddr = smtpUser_;
    if (smtpServer_.contains("mailjet", Qt::CaseInsensitive)) {
        // Forcer l'adresse d'expéditeur à gayensis09@gmail.com
        fromAddr = "\"WaterSN\" <gayensis09@gmail.com>";
    }
    qDebug() << "[Mail] Envoi: MAIL FROM:<" << fromAddr << ">";
    if (smtpServer_.contains("mailjet", Qt::CaseInsensitive))
        sendLine(&socket, "MAIL FROM:<gayensis09@gmail.com>");
    else
        sendLine(&socket, "MAIL FROM:<" + smtpUser_ + ">");
    resp = readLine(&socket);
    qDebug() << "[Mail] Réponse MAIL FROM:" << resp;
    if (!resp.startsWith("250")) {
        qWarning() << "[Mail] MAIL FROM échoué: " << resp;
        if (errorMsg) *errorMsg = resp;
        return false;
    }
    // RCPT TO
    qDebug() << "[Mail] Envoi: RCPT TO:<" << to << ">";
    sendLine(&socket, "RCPT TO:<" + to + ">");
    resp = readLine(&socket);
    qDebug() << "[Mail] Réponse RCPT TO:" << resp;
    if (!resp.startsWith("250")) {
        qWarning() << "[Mail] RCPT TO échoué: " << resp;
        if (errorMsg) *errorMsg = resp;
        return false;
    }
    // DATA
    qDebug() << "[Mail] Envoi: DATA";
    sendLine(&socket, "DATA");
    resp = readLine(&socket);
    qDebug() << "[Mail] Réponse DATA:" << resp;
    if (!resp.startsWith("354")) {
        qWarning() << "[Mail] DATA échoué: " << resp;
        if (errorMsg) *errorMsg = resp;
        return false;
    }

    // Corps du mail (HTML si body contient des balises <html> ou <body>, sinon texte)
    QString mailData;
    if (smtpServer_.contains("mailjet", Qt::CaseInsensitive))
        mailData += "From: \"WaterSN\" <gayensis09@gmail.com>\r\n";
    else
        mailData += "From: <" + smtpUser_ + ">\r\n";
    mailData += "To: <" + to + ">\r\n";
    mailData += "Subject: " + subject + "\r\n";
    mailData += "MIME-Version: 1.0\r\n";
    // Si le corps contient <html> ou <body> ou commence par <, envoyer en HTML
    if (body.trimmed().startsWith("<")) {
        mailData += "Content-Type: text/html; charset=UTF-8\r\n";
    } else {
        mailData += "Content-Type: text/plain; charset=UTF-8\r\n";
    }
    mailData += "\r\n";
    mailData += body + "\r\n";
    mailData += ".\r\n";
    qDebug() << "[Mail] Envoi du corps du mail.";
    sendLine(&socket, mailData);
    resp = readLine(&socket);
    qDebug() << "[Mail] Réponse après DATA (corps):" << resp;
    if (!resp.startsWith("250")) {
        qWarning() << "[Mail] Envoi du mail échoué: " << resp;
        if (errorMsg) *errorMsg = resp;
        return false;
    }

    // QUIT
    qDebug() << "[Mail] Envoi: QUIT";
    sendLine(&socket, "QUIT");
    socket.disconnectFromHost();
    qDebug() << "[Mail] Déconnexion du serveur SMTP.";
    return true;
}