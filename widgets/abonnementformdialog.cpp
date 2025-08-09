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

AbonnementFormDialog::AbonnementFormDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Ajouter un abonnement");
    setModal(true);
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Compteur
    QHBoxLayout* compteurLayout = new QHBoxLayout;
    QLabel* compteurLabel = new QLabel("Compteur:", this);
    compteurEdit = new QLineEdit(this);
    compteurLayout->addWidget(compteurLabel);
    compteurLayout->addWidget(compteurEdit);
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
    return compteurEdit ? compteurEdit->text() : "";
}

// Suppression de la méthode getAdresse obsolète
QDate AbonnementFormDialog::getDateDebut() const {
    return dateDebutEdit ? dateDebutEdit->date() : QDate();
}

QString AbonnementFormDialog::getStatut() const {
    return statutCombo ? statutCombo->currentText() : "";
}

// Suppression de la méthode getConsommation obsolète
