#include "widgets/clientformdialog.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QPixmap>
#include <QCoreApplication>
#include "widgets/logowidget.h"
#include <QFile>
#include <QDebug>
#include <QDir>

ClientFormDialog::ClientFormDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Ajouter un client");
    setMinimumWidth(400);
    resize(800, 600); // Taille similaire à l'écran de connexion
    
    // Fond principal du dialogue avec couleur verte
    setStyleSheet("QDialog { background: #5d7c72; }");
    
    // Ajouter le logo en haut à droite de la fenêtre (comme l'écran de connexion)
    LogoWidget* logo = new LogoWidget(this);
    logo->move(width() - logo->width() - 20, 20);
    logo->raise();
    logo->show();
    
    // Créer un layout principal centré
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // Créer un cadre pour contenir le formulaire (similaire au card de l'écran de connexion)
    QFrame* cardFrame = new QFrame(this);
    cardFrame->setObjectName("cardFrame");
    cardFrame->setStyleSheet("QFrame#cardFrame { background: #39544c; border-radius: 16px; min-width: 340px; min-height: 180px; "
                           "box-shadow: 0 8px 24px rgba(0,0,0,0.2); border: 1px solid #2c4036; }");
    
    // Layout interne du cadre
    QVBoxLayout* cardLayout = new QVBoxLayout(cardFrame);
    cardLayout->setContentsMargins(30, 30, 30, 30);
    cardLayout->setSpacing(15);
    
    // Titre du formulaire
    QLabel* titleLabel = new QLabel("Nouveau Client");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setStyleSheet("QLabel#titleLabel { color: #fff; font-size: 24px; font-weight: bold; letter-spacing: 1px; margin-bottom: 4px; }");
    cardLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    
    // Label d'erreur (initialement caché)
    errorLabel = new QLabel();
    errorLabel->setObjectName("errorLabel");
    errorLabel->setStyleSheet("QLabel#errorLabel { color: #ff4444; font-size: 13px; font-weight: bold; padding: 2px 0 2px 0; border-radius: 4px; background: transparent; }");
    errorLabel->setVisible(false);
    cardLayout->addWidget(errorLabel);
    
    // Style pour les champs de saisie (identique à l'écran de connexion)
    QString inputStyle = "QLineEdit { background: #fffbe8; border: 2px solid #e0e0c0; border-radius: 8px; padding: 8px 12px; "
                        "font-size: 15px; color: #39544c; margin-bottom: 8px; } "
                        "QLineEdit:focus { border: 2px solid #ffd23f; background: #fffde4; }";
    
    // Champs du formulaire
    nomLineEdit = new QLineEdit();
    nomLineEdit->setStyleSheet(inputStyle);
    nomLineEdit->setPlaceholderText("Entrez le nom");
    cardLayout->addWidget(nomLineEdit);
    
    prenomLineEdit = new QLineEdit();
    prenomLineEdit->setStyleSheet(inputStyle);
    prenomLineEdit->setPlaceholderText("Entrez le prénom");
    cardLayout->addWidget(prenomLineEdit);
    
    adresseLineEdit = new QLineEdit();
    adresseLineEdit->setStyleSheet(inputStyle);
    adresseLineEdit->setPlaceholderText("Entrez l'adresse");
    cardLayout->addWidget(adresseLineEdit);
    
    telephoneLineEdit = new QLineEdit();
    telephoneLineEdit->setStyleSheet(inputStyle);
    telephoneLineEdit->setPlaceholderText("Entrez le numéro de téléphone");
    cardLayout->addWidget(telephoneLineEdit);
    
    // Boutons d'action (en style vertical comme l'écran de connexion)
    QVBoxLayout* buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(12);
    
    // Bouton Ajouter (jaune, comme le bouton de connexion)
    QPushButton* addButton = new QPushButton("Ajouter");
    addButton->setObjectName("addButton");
    addButton->setFixedHeight(45); // Hauteur similaire au bouton Login
    addButton->setStyleSheet("QPushButton#addButton { background: #ffd23f; color: #39544c; border-radius: 8px; "
                           "font-weight: bold; font-size: 17px; padding: 10px 0; margin-top: 6px; margin-bottom: 2px; "
                           "box-shadow: 0 2px 6px rgba(0,0,0,0.15); transition: all 0.2s; } "
                           "QPushButton#addButton:hover { background: #ffe066; box-shadow: 0 4px 10px rgba(0,0,0,0.25); }");
    
    // Bouton Annuler (gris, plus petit et en bas comme dans l'interface de connexion)
    QPushButton* cancelButton = new QPushButton("Annuler");
    cancelButton->setObjectName("cancelButton");
    cancelButton->setFixedHeight(40);
    cancelButton->setStyleSheet("QPushButton#cancelButton { background: transparent; color: #e6e6e6; border-radius: 8px; "
                              "font-weight: bold; font-size: 14px; padding: 8px 0; margin-top: 0; "
                              "transition: all 0.2s; } "
                              "QPushButton#cancelButton:hover { color: #ffffff; }");
    
    // Ajouter les boutons au layout dans l'ordre
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(cancelButton);
    
    cardLayout->addLayout(buttonLayout);
    
    // Connecter les boutons
    connect(addButton, &QPushButton::clicked, this, &ClientFormDialog::onAddClicked);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    // Ajouter le cardFrame au layout principal, centré
    mainLayout->addWidget(cardFrame, 0, Qt::AlignCenter);
}

void ClientFormDialog::setClient(const Client& client) {
    nomLineEdit->setText(client.nom);
    prenomLineEdit->setText(client.prenom);
    adresseLineEdit->setText(client.adresse);
    telephoneLineEdit->setText(client.telephone);
}

Client ClientFormDialog::getClient() const {
    return Client(0, 
                 nomLineEdit->text().trimmed(),
                 prenomLineEdit->text().trimmed(),
                 adresseLineEdit->text().trimmed(),
                 telephoneLineEdit->text().trimmed());
}

void ClientFormDialog::onAddClicked() {
    // Validation des champs obligatoires
    errorLabel->setVisible(false);
    
    if (nomLineEdit->text().trimmed().isEmpty()) {
        showError("Le nom est obligatoire.");
        nomLineEdit->setFocus();
        return;
    }
    
    if (prenomLineEdit->text().trimmed().isEmpty()) {
        showError("Le prénom est obligatoire.");
        prenomLineEdit->setFocus();
        return;
    }
    
    if (telephoneLineEdit->text().trimmed().isEmpty()) {
        showError("Le numéro de téléphone est obligatoire.");
        telephoneLineEdit->setFocus();
        return;
    }
    
    // Tout est valide, on accepte le dialogue
    accept();
}

void ClientFormDialog::showError(const QString& message) {
    errorLabel->setText(message);
    errorLabel->setVisible(true);
}
