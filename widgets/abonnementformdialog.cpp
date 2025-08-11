#include "widgets/abonnementformdialog.h"

void AbonnementFormDialog::onValiderClicked() {
    // Slot prévu pour la validation, actuellement non utilisé
}
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>


AbonnementFormDialog::AbonnementFormDialog(const QStringList& compteursDisponibles, QWidget* parent)
    : QDialog(parent), compteursDisponibles_(compteursDisponibles)
{
    setWindowTitle("Ajouter un abonnement");
    setModal(true);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Compteur
    QHBoxLayout* compteurLayout = new QHBoxLayout;
    QLabel* compteurLabel = new QLabel("Compteur:", this);
    compteurCombo = new QComboBox(this);
    compteurCombo->addItems(compteursDisponibles_);
    compteurLayout->addWidget(compteurLabel);
    compteurLayout->addWidget(compteurCombo);
    mainLayout->addLayout(compteurLayout);

    // Date début
    QHBoxLayout* dateLayout = new QHBoxLayout;
    QLabel* dateLabel = new QLabel("Date début:", this);
    dateDebutEdit = new QDateEdit(QDate::currentDate(), this);
    dateDebutEdit->setCalendarPopup(true);
    dateLayout->addWidget(dateLabel);
    dateLayout->addWidget(dateDebutEdit);
    mainLayout->addLayout(dateLayout);

    // Boutons OK/Annuler
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}


QString AbonnementFormDialog::getCompteur() const {
    return compteurCombo ? compteurCombo->currentText() : "";
}

// Suppression de la méthode getAdresse obsolète
QDate AbonnementFormDialog::getDateDebut() const {
    return dateDebutEdit ? dateDebutEdit->date() : QDate();
}

QString AbonnementFormDialog::getStatut() const {
    return statutCombo ? statutCombo->currentText() : "";
}

// Suppression de la méthode getConsommation obsolète
