

#include "widgets/authdialog.h"
#include "widgets/auth/ui_authdialog.h"
#include <QFile>
#include <QRegularExpressionValidator>
#include <QMessageBox>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QApplication>
#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QDebug>
#include <QFile>
#include <QStyle>
#include <QDir>
#include <QWidget>
#include <QDialog>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QMetaType>
#include <QMetaMethod>

#include "widgets/logowidget.h"
#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QMetaType>
#include <QMetaMethod>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QMetaType>
#include <QMetaMethod>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QMetaType>
#include <QMetaMethod>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QMetaType>
#include <QMetaMethod>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QMetaType>
#include <QMetaMethod>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QMetaType>
#include <QMetaMethod>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QMetaType>
#include <QMetaMethod>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QMetaType>
#include <QMetaMethod>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QMetaType>
#include <QMetaMethod>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QMetaType>
#include <QMetaMethod>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QMetaType>
#include <QMetaMethod>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QMetaType>
#include <QMetaMethod>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QMetaType>
#include <QMetaMethod>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QMetaType>
#include <QMetaMethod>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaProperty>
#include <QMetaClassInfo>
#include <QMetaType>
#include <QMetaMethod>
#include <QMetaEnum>



AuthDialog::AuthDialog(QWidget* parent)
    : QDialog(parent), emailEdit(nullptr), passwordEdit(nullptr), loginButton(nullptr), forgotButton(nullptr), errorLabel(nullptr), ui(nullptr)
{
    ui = new Ui::AuthDialog;
    ui->setupUi(this);
    resize(800, 650); // Taille par défaut à l'ouverture (encore plus grande)
    // Ajouter le logo en haut à droite de la fenêtre (hors carte centrale)
    LogoWidget* logo = new LogoWidget(this);
    logo->move(width() - logo->width() - 20, 20); // Décalage 20px du bord droit/haut
    logo->raise();
    logo->show();
    setWindowTitle("Connexion à WaterSN");
    setModal(true);
    // setFixedSize(420, 320); // Supprimé pour permettre le redimensionnement

    // Appliquer le QSS
    qDebug() << "[AuthDialog] Current working dir:" << QDir::currentPath();
    QFile styleFile("widgets/auth/authdialog.qss");
    if (!styleFile.open(QFile::ReadOnly)) {
        qWarning("[AuthDialog] Impossible d'ouvrir le fichier QSS widgets/auth/authdialog.qss");
    } else {
        setStyleSheet(styleFile.readAll());
        qDebug() << "[AuthDialog] QSS chargé avec succès.";
    }

    emailEdit = ui->emailEdit;
    passwordEdit = ui->passwordEdit;
    loginButton = ui->loginButton;
    errorLabel = ui->errorLabel;
    forgotButton = ui->forgotButton;

    // Validation email
    if (emailEdit)
        emailEdit->setValidator(new QRegularExpressionValidator(QRegularExpression(R"(^[\w.-]+@[\w.-]+\.[a-zA-Z]{2,}$)"), this));
    if (passwordEdit)
        passwordEdit->setEchoMode(QLineEdit::Password);

    if (loginButton)
        connect(loginButton, &QPushButton::clicked, this, &AuthDialog::onLoginClicked);
    if (forgotButton)
        connect(forgotButton, &QPushButton::clicked, this, &AuthDialog::onForgotPasswordClicked);
}

AuthDialog::~AuthDialog() {
    delete ui;
}

QString AuthDialog::getEmail() const { return emailEdit->text().trimmed(); }
QString AuthDialog::getPassword() const { return passwordEdit->text(); }

void AuthDialog::onLoginClicked() {
    validateInputs();
    if (errorLabel->isVisible()) return;
    accept(); // ou lancer la logique d'authentification
}

void AuthDialog::onForgotPasswordClicked() {
    emit forgotPasswordClicked();
}

void AuthDialog::validateInputs() {
    errorLabel->setVisible(false);
    if (getEmail().isEmpty() || getPassword().isEmpty()) {
        errorLabel->setText("Tous les champs sont obligatoires.");
        errorLabel->setVisible(true);
        return;
    }
    if (!emailEdit->hasAcceptableInput()) {
        errorLabel->setText("Email invalide.");
        errorLabel->setVisible(true);
        return;
    }
}
