#include "widgets/factureformdialog.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QSqlDatabase>
#include <QSqlQuery>

FactureFormDialog::FactureFormDialog(const QStringList& compteursActifs, QWidget* parent) : QDialog(parent) {
    setWindowTitle("Ajouter une facture");
    setModal(true);

    QFormLayout* formLayout = new QFormLayout(this);

    numCompteurCombo = new QComboBox(this);
    numCompteurCombo->addItems(compteursActifs);
    soldeAnterieurEdit = new QLineEdit(this);

    consommationEdit = new QLineEdit(this);
    consommationEdit->setReadOnly(true);

    formLayout->addRow("Numéro de compteur:", numCompteurCombo);
    formLayout->addRow("Solde antérieur:", soldeAnterieurEdit);
    formLayout->addRow("Consommation (calculée):", consommationEdit);

    // Calcul automatique de la consommation lors du changement de compteur
    connect(numCompteurCombo, &QComboBox::currentTextChanged, this, [=](const QString& numCompteur) {
        double ancienIndex = 0.0, newIndex = 0.0;
        QSqlDatabase db = QSqlDatabase::database();
        if (db.isOpen()) {
            QSqlQuery query(db);
            // Récupérer le dernier prélèvement pour ce compteur
            query.prepare("SELECT newIndex, ancienIndex FROM Prelevement WHERE numCompteur = :numCompteur ORDER BY dateprelevement DESC LIMIT 1");
            query.bindValue(":numCompteur", numCompteur);
            if (query.exec() && query.next()) {
                newIndex = query.value(0).toDouble();
                ancienIndex = query.value(1).toDouble();
            }
        }
        double consommation = newIndex - ancienIndex;
        consommationEdit->setText(QString::number(consommation, 'f', 2));
    });

    // Initialiser la consommation au démarrage si un compteur est sélectionné
    if (!compteursActifs.isEmpty()) {
        emit numCompteurCombo->currentTextChanged(numCompteurCombo->currentText());
    }

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    formLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

void FactureFormDialog::setFacture(const Facture& facture) {
    int idx = numCompteurCombo->findText(facture.numCompteur);
    if (idx >= 0) numCompteurCombo->setCurrentIndex(idx);
    soldeAnterieurEdit->setText(QString::number(facture.soldeanterieur));
    consommationEdit->setText(QString::number(facture.consommation));
}

Facture FactureFormDialog::getFacture(int idClient) const {
    int idFacture = 0; // Généré par la base ou à définir ailleurs
    QString numCompteur = numCompteurCombo->currentText();
    double soldeanterieur = soldeAnterieurEdit->text().toDouble();
    double consommation = consommationEdit->text().toDouble();
    return Facture(idFacture, numCompteur, idClient, soldeanterieur, consommation);
}
