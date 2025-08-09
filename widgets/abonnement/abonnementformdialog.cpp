#include "abonnementformdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QDateEdit>
#include <QMessageBox>

AbonnementFormDialog::AbonnementFormDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Ajouter un abonnement");
    setModal(true);
    setMinimumWidth(400);
    setStyleSheet("background: #22332d; color: #e6e6e6; border-radius: 12px;");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(18);

    // Compteur
    QHBoxLayout* compteurLayout = new QHBoxLayout;
    QLabel* compteurLabel = new QLabel("Compteur:", this);
    compteurEdit = new QLineEdit(this);
    compteurEdit->setPlaceholderText("Numéro du compteur");
    compteurLayout->addWidget(compteurLabel);
    compteurLayout->addWidget(compteurEdit);
    mainLayout->addLayout(compteurLayout);

    // Adresse
    QHBoxLayout* adresseLayout = new QHBoxLayout;
    QLabel* adresseLabel = new QLabel("Adresse:", this);
    adresseEdit = new QLineEdit(this);
    adresseEdit->setPlaceholderText("Adresse de l'abonnement");
    adresseLayout->addWidget(adresseLabel);
    adresseLayout->addWidget(adresseEdit);
    mainLayout->addLayout(adresseLayout);

    // Date début
    QHBoxLayout* dateLayout = new QHBoxLayout;
    QLabel* dateLabel = new QLabel("Date début:", this);
    dateDebutEdit = new QDateEdit(this);
    dateDebutEdit->setCalendarPopup(true);
    dateDebutEdit->setDate(QDate::currentDate());
    dateLayout->addWidget(dateLabel);
    dateLayout->addWidget(dateDebutEdit);
    mainLayout->addLayout(dateLayout);

    // Statut
    QHBoxLayout* statutLayout = new QHBoxLayout;
    QLabel* statutLabel = new QLabel("Statut:", this);
    statutCombo = new QComboBox(this);
    statutCombo->addItems({"Actif", "Suspendu", "Résilié"});
    statutLayout->addWidget(statutLabel);
    statutLayout->addWidget(statutCombo);
    mainLayout->addLayout(statutLayout);

    // Consommation
    QHBoxLayout* consoLayout = new QHBoxLayout;
    QLabel* consoLabel = new QLabel("Consommation initiale:", this);
    consoEdit = new QLineEdit(this);
    consoEdit->setPlaceholderText("0 m³");
    consoLayout->addWidget(consoLabel);
    consoLayout->addWidget(consoEdit);
    mainLayout->addLayout(consoLayout);

    // Boutons
    QHBoxLayout* btnLayout = new QHBoxLayout;
    QPushButton* btnAnnuler = new QPushButton("Annuler", this);
    QPushButton* btnValider = new QPushButton("Valider", this);
    btnAnnuler->setStyleSheet("background: #b7e0c0; color: #22332d; border-radius: 6px; padding: 8px 18px;");
    btnValider->setStyleSheet("background: #ffd23f; color: #22332d; font-weight: bold; border-radius: 6px; padding: 8px 18px;");
    btnAnnuler->setCursor(Qt::PointingHandCursor);
    btnValider->setCursor(Qt::PointingHandCursor);
    btnLayout->addStretch();
    btnLayout->addWidget(btnAnnuler);
    btnLayout->addWidget(btnValider);
    mainLayout->addLayout(btnLayout);

    connect(btnAnnuler, &QPushButton::clicked, this, &QDialog::reject);
    connect(btnValider, &QPushButton::clicked, this, &AbonnementFormDialog::onValiderClicked);
}

void AbonnementFormDialog::onValiderClicked()
{
    if (compteurEdit->text().isEmpty() || adresseEdit->text().isEmpty() || consoEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Champs manquants", "Veuillez remplir tous les champs obligatoires.");
        return;
    }
    // Ici, on pourrait ajouter une validation plus poussée
    accept();
}

QString AbonnementFormDialog::getCompteur() const { return compteurEdit->text(); }
QString AbonnementFormDialog::getAdresse() const { return adresseEdit->text(); }
QDate AbonnementFormDialog::getDateDebut() const { return dateDebutEdit->date(); }
QString AbonnementFormDialog::getStatut() const { return statutCombo->currentText(); }
QString AbonnementFormDialog::getConsommation() const { return consoEdit->text(); }
