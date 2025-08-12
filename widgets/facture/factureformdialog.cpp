#include "widgets/factureformdialog.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>

FactureFormDialog::FactureFormDialog(const QStringList& compteursActifs, QWidget* parent) : QDialog(parent) {
    setWindowTitle("Ajouter une facture");
    setModal(true);

    QFormLayout* formLayout = new QFormLayout(this);

    numCompteurCombo = new QComboBox(this);
    numCompteurCombo->addItems(compteursActifs);
    soldeAnterieurEdit = new QLineEdit(this);
    soldeAnterieurEdit->setReadOnly(true);

    consommationEdit = new QLineEdit(this);
    consommationEdit->setReadOnly(true);

    formLayout->addRow("Numéro de compteur:", numCompteurCombo);
    formLayout->addRow("Solde antérieur:", soldeAnterieurEdit);
    formLayout->addRow("Consommation (calculée):", consommationEdit);

    // Calcul automatique de la consommation et du solde antérieur lors du changement de compteur
    connect(numCompteurCombo, &QComboBox::currentTextChanged, this, [=](const QString& numCompteur) {
    double newIndex1 = 0.0, ancienIndex1 = 0.0, newIndex2 = 0.0, ancienIndex2 = 0.0;
    int prelevementCount = 0;
    QSqlDatabase db = QSqlDatabase::database();
    if (db.isOpen()) {
        QSqlQuery query(db);
        query.prepare("SELECT newIndex, ancienIndex FROM Prelevement WHERE numCompteur = :numCompteur ORDER BY dateprelevement DESC LIMIT 2");
        query.bindValue(":numCompteur", numCompteur);
        if (query.exec()) {
            if (query.next()) {
                newIndex1 = query.value(0).toDouble();
                ancienIndex1 = query.value(1).toDouble();
                prelevementCount++;
            }
            if (query.next()) {
                newIndex2 = query.value(0).toDouble();
                ancienIndex2 = query.value(1).toDouble();
                prelevementCount++;
            }
        }
    }
    qDebug() << "DEBUG index prélèvements:" << "newIndex1=" << newIndex1 << ", ancienIndex1=" << ancienIndex1 << ", newIndex2=" << newIndex2 << ", ancienIndex2=" << ancienIndex2;
    double consommation = 0.0;
    if (prelevementCount == 2) {
        consommation = newIndex2 - ancienIndex2;
        if (consommation < 0) {
            consommation = 0.0;
            QMessageBox::warning(this, "Index incohérents", "L'ancien index est supérieur au nouvel index. Veuillez vérifier les prélèvements.");
        }
        consommationEdit->setText(QString::number(consommation, 'f', 2));
        // Calcul du solde antérieur selon les tranches SEN'EAU
        double soldeConso = ancienIndex2 - ancienIndex1;
        if (soldeConso < 0) soldeConso = 0.0;
        double montantSolde = 0.0;
        double resteSolde = soldeConso;
        struct Tranche { double max; double prix; } tranches[] = {
            {10, 286.36}, {20, 366.36}, {30, 446.36}, {99999, 526.36}
        };
        for (const auto& t : tranches) {
            double consoTranche = std::min(resteSolde, t.max);
            montantSolde += consoTranche * t.prix;
            resteSolde -= consoTranche;
            if (resteSolde <= 0) break;
        }
        // Récupérer la mensualité liée à la dernière facture pour ce compteur et ce client
        double derniereMensualite = 0.0;
        int idFactureDerniere = 0;
        int idClient = 0;
        // Récupérer l'id du client à partir du compteur sélectionné
        QSqlQuery queryClient(db);
        queryClient.prepare("SELECT idClient FROM Abonnement WHERE numCompteur = :numCompteur ORDER BY date_abonnement DESC LIMIT 1");
        queryClient.bindValue(":numCompteur", numCompteur);
        if (queryClient.exec() && queryClient.next()) {
            idClient = queryClient.value(0).toInt();
        }
        // Récupérer l'id de la dernière facture pour ce compteur et ce client
        QSqlQuery queryFacture(db);
        queryFacture.prepare("SELECT idFacture FROM Facture WHERE numCompteur = :numCompteur AND idClient = :idClient ORDER BY idFacture DESC LIMIT 1");
        queryFacture.bindValue(":numCompteur", numCompteur);
        queryFacture.bindValue(":idClient", idClient);
        if (queryFacture.exec() && queryFacture.next()) {
            idFactureDerniere = queryFacture.value(0).toInt();
        }
        // Récupérer la mensualité de la dernière facturation pour ce compteur, ce client et cette facture
        if (idFactureDerniere > 0 && idClient > 0) {
            QSqlQuery queryMensualite(db);
            queryMensualite.prepare("SELECT mensualite FROM Facturation WHERE numCompteur = :numCompteur AND idClient = :idClient AND idFacture = :idFacture ORDER BY date_paiement DESC LIMIT 1");
            queryMensualite.bindValue(":numCompteur", numCompteur);
            queryMensualite.bindValue(":idClient", idClient);
            queryMensualite.bindValue(":idFacture", idFactureDerniere);
            if (queryMensualite.exec() && queryMensualite.next()) {
                derniereMensualite = queryMensualite.value(0).toDouble();
            }
        }
        double soldeFinal = montantSolde - derniereMensualite;
    if (soldeFinal < 0) soldeFinal = 0.0;
    qDebug() << "DEBUG montantSolde:" << montantSolde;
    qDebug() << "DEBUG numCompteur:" << numCompteur;
    qDebug() << "DEBUG idClient trouvé:" << idClient;
    qDebug() << "DEBUG idFactureDerniere:" << idFactureDerniere;
    qDebug() << "DEBUG derniereMensualite:" << derniereMensualite;
    soldeAnterieurEdit->setText(QString::number(soldeFinal, 'f', 2));
    } else if (prelevementCount == 1) {
        consommation = newIndex1 - ancienIndex1;
        if (consommation < 0) consommation = 0.0;
        consommationEdit->setText(QString::number(consommation, 'f', 2));
        // Pas de calcul du solde antérieur, il reste à 0 par défaut
        soldeAnterieurEdit->setText("0.00");
    } else {
        consommationEdit->setText("0.00");
        soldeAnterieurEdit->setText("0.00");
    }
    });

    // Initialiser la consommation et le solde antérieur au démarrage pour le premier compteur actif
    if (!compteursActifs.isEmpty()) {
        // Force le signal même si l'index est déjà à 0
        QMetaObject::invokeMethod(numCompteurCombo, "currentTextChanged", Qt::QueuedConnection, Q_ARG(QString, numCompteurCombo->currentText()));
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
