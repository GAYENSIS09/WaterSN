#include "widgets/resetpassworddialog.h"
#include <QFormLayout>
#include <QFile>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressDialog>
#include <QTimer>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QRandomGenerator>
#include <QRegularExpressionValidator>
#include "utils/bcryptutils.h"
#include "mail.h"
#include "widgets/logowidget.h"

ResetPasswordDialog::ResetPasswordDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Réinitialisation");
    setModal(true);
    setFixedSize(420, 320);

    // Appliquer le QSS pour cohérence avec la page de connexion
    QFile styleFile("widgets/auth/resetpassworddialog.qss");
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(styleFile.readAll());
    }

    

    // Card centrale (QFrame) harmonisée
    QFrame* cardFrame = new QFrame(this);
    cardFrame->setObjectName("cardFrame");
    cardFrame->setMinimumWidth(340);
    cardFrame->setMaximumWidth(380);

    QVBoxLayout* cardLayout = new QVBoxLayout(cardFrame);
    cardLayout->setContentsMargins(24, 24, 24, 24);
    cardLayout->setSpacing(10);
    // Ajout d'un spacer vertical pour l'aération
    cardLayout->addSpacing(12);

    QLabel* titleLabel = new QLabel("Réinitialisation", cardFrame);
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignHCenter);
    cardLayout->addWidget(titleLabel);

    emailEdit = new QLineEdit(cardFrame);
    emailEdit->setPlaceholderText("Email");
    emailEdit->setClearButtonEnabled(true);
    emailEdit->setValidator(new QRegularExpressionValidator(QRegularExpression(R"(^[\w.-]+@[\w.-]+\.[a-zA-Z]{2,}$)"), this));
    cardLayout->addWidget(emailEdit);

    errorLabel = new QLabel(cardFrame);
    errorLabel->setObjectName("errorLabel");
    errorLabel->setVisible(false);
    cardLayout->addWidget(errorLabel);

    sendButton = new QPushButton("Envoyer", cardFrame);
    sendButton->setObjectName("loginButton"); // Pour le QSS commun
    sendButton->setMinimumHeight(38);
    sendButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    cardLayout->addWidget(sendButton);

    resendButton = new QPushButton("Renvoyer", cardFrame);
    resendButton->setVisible(false);
    cardLayout->addWidget(resendButton);

    infoLabel = new QLabel(cardFrame);
    infoLabel->setObjectName("infoLabel");
    infoLabel->setVisible(false);
    cardLayout->addWidget(infoLabel);

    // ... après la création de infoLabel et avant progressDialog ...

    // Layout principal (card centrée uniquement)
    QVBoxLayout* mainVLayout = new QVBoxLayout(this);
    mainVLayout->addStretch();
    QHBoxLayout* centerLayout = new QHBoxLayout;
    centerLayout->addStretch();
    centerLayout->addWidget(cardFrame);
    centerLayout->addStretch();
    mainVLayout->addLayout(centerLayout);
    mainVLayout->addStretch();
    setLayout(mainVLayout);

    // Logo overlay absolu devant (toujours visible, non déformant)
    LogoWidget* logo = new LogoWidget(this);
    logo->setParent(this);
    logo->raise();
    logo->move(width() - logo->width() - 24, 24);
    logo->show();
    // Suivi du resize pour garder le logo en haut à droite
    installEventFilter(logo);

    progressDialog = new QProgressDialog("Envoi en cours...", QString(), 0, 0, this);
    progressDialog->setWindowModality(Qt::WindowModal);
    progressDialog->setCancelButton(nullptr);
    progressDialog->setMinimumDuration(0);
    progressDialog->close();

    connect(sendButton, &QPushButton::clicked, this, &ResetPasswordDialog::onSendClicked);
    connect(resendButton, &QPushButton::clicked, this, &ResetPasswordDialog::onResendClicked);
}

void ResetPasswordDialog::validateInput() {
    errorLabel->setVisible(false);
    if (emailEdit->text().trimmed().isEmpty()) {
        errorLabel->setText("L'email est obligatoire.");
        errorLabel->setVisible(true);
        return;
    }
    if (!emailEdit->hasAcceptableInput()) {
        errorLabel->setText("Email invalide.");
        errorLabel->setVisible(true);
        return;
    }
}

void ResetPasswordDialog::setLoading(bool loading) {
    if (loading) progressDialog->show();
    else progressDialog->close();
    sendButton->setEnabled(!loading);
    resendButton->setEnabled(!loading);
    emailEdit->setEnabled(!loading);
}

void ResetPasswordDialog::resetUI() {
    infoLabel->setVisible(false);
    errorLabel->setVisible(false);
    resendButton->setVisible(false);
}

void ResetPasswordDialog::onSendClicked() {
    resetUI();
    validateInput();
    if (errorLabel->isVisible()) return;
    setLoading(true);
    // Vérification réelle de l'email dans la base
    QSqlQuery query;
    query.prepare("SELECT id FROM User WHERE email = :email");
    query.bindValue(":email", emailEdit->text().trimmed());
    bool found = false;
    if (query.exec() && query.next()) {
        found = true;
    }
    QTimer::singleShot(300, [this, found]() {
        onEmailCheckFinished(found);
    });
}

void ResetPasswordDialog::onResendClicked() {
    onSendClicked();
}

void ResetPasswordDialog::onEmailCheckFinished(bool found) {
    if (!found) {
        setLoading(false);
        errorLabel->setText("Email non trouvé dans la base.");
        errorLabel->setVisible(true);
        return;
    }
    // Générer un code à 6 chiffres
    QString code = QString::number(QRandomGenerator::global()->bounded(100000, 999999));
    // Mettre à jour le mot de passe (hashé)
    QString hashed = BcryptUtils::hashPassword(code);
    QSqlQuery update;
    update.prepare("UPDATE User SET password = :pwd WHERE email = :email");
    update.bindValue(":pwd", hashed);
    update.bindValue(":email", emailEdit->text().trimmed());
    bool updateOk = update.exec();
    // Envoyer le code par mail
    bool mailOk = false;
    if (updateOk) {
        QString subject = "Réinitialisation de votre mot de passe WaterSN";
        QString body = QString("Votre nouveau mot de passe temporaire est : %1\nMerci de le changer après connexion.").arg(code);
        mailOk = Mail::sendMail(emailEdit->text().trimmed(), subject, body);
    }
    QTimer::singleShot(500, [this, mailOk]() {
        onSendMailFinished(mailOk);
    });
}

void ResetPasswordDialog::onSendMailFinished(bool success) {
    setLoading(false);
    if (success) {
        infoLabel->setText("Un nouveau mot de passe a été envoyé à votre adresse email.");
        infoLabel->setVisible(true);
        resendButton->setVisible(true);
        QMessageBox::information(this, "Succès", "Mot de passe envoyé !");
        emit passwordResetSuccess();
    } else {
        errorLabel->setText("Erreur d'envoi. Vérifiez votre connexion internet.");
        errorLabel->setVisible(true);
        resendButton->setVisible(true);
        QMessageBox::critical(this, "Erreur", "Impossible d'envoyer le mail. Veuillez réessayer.");
    }
}
