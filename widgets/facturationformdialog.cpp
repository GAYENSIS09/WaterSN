#include "widgets/facturationformdialog.h"
#include <QFormLayout>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

FacturationFormDialog::FacturationFormDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUI();
    connectSignals();
}

void FacturationFormDialog::setupUI() {
    dateEdit = new QDateEdit(QDate::currentDate(), this);
    dateEdit->setCalendarPopup(true);
    compteurCombo = new QComboBox(this);
    factureCombo = new QComboBox(this);
    clientCombo = new QComboBox(this);
    mensualiteEdit = new QLineEdit(this);
    okButton = new QPushButton("Valider", this);
    cancelButton = new QPushButton("Annuler", this);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("Date de paiement:", dateEdit);
    formLayout->addRow("Compteur transféré:", compteurCombo);
    formLayout->addRow("Facture associée:", factureCombo);
    formLayout->addRow("Client:", clientCombo);
    formLayout->addRow("Mensualité:", mensualiteEdit);
    formLayout->addRow(okButton, cancelButton);
    setLayout(formLayout);
}

void FacturationFormDialog::connectSignals() {
    connect(compteurCombo, &QComboBox::currentTextChanged, this, [this](const QString& numCompteur){
        // Charger les factures associées au compteur sélectionné
        QSqlQuery query;
        query.prepare("SELECT idFacture, CONCAT('Facture ', idFacture, ' (Solde:', soldeanterieur, ', Conso:', consommation, ')') FROM Facture WHERE numCompteur = ?");
        query.addBindValue(numCompteur);
        QList<QPair<int, QString>> factures;
        if (query.exec()) {
            while (query.next()) {
                factures.append(qMakePair(query.value(0).toInt(), query.value(1).toString()));
            }
        }
        setFactures(factures);
        // Charger le client associé à la première facture (si existe)
        if (!factures.isEmpty()) {
            QSqlQuery queryClient;
            queryClient.prepare("SELECT idClient, nom FROM Client WHERE idClient = (SELECT idClient FROM Facture WHERE idFacture = ?)");
            queryClient.addBindValue(factures.first().first);
            QList<QPair<int, QString>> clients;
            if (queryClient.exec() && queryClient.next()) {
                clients.append(qMakePair(queryClient.value(0).toInt(), queryClient.value(1).toString()));
            }
            setClients(clients);
        } else {
            setClients({});
        }
    });
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

QDate FacturationFormDialog::getDatePaiement() const {
    return dateEdit->date();
}

QString FacturationFormDialog::getNumCompteur() const {
    return compteurCombo->currentText();
}

int FacturationFormDialog::getIdFacture() const {
    return factureCombo->currentData().toInt();
}

int FacturationFormDialog::getIdClient() const {
    return clientCombo->currentData().toInt();
}

double FacturationFormDialog::getMensualite() const {
    return mensualiteEdit->text().toDouble();
}

void FacturationFormDialog::setCompteurs(const QStringList& compteurs) {
    compteurCombo->clear();
    compteurCombo->addItems(compteurs);
}

void FacturationFormDialog::setFactures(const QList<QPair<int, QString>>& factures) {
    factureCombo->clear();
    for (const auto& f : factures) {
        factureCombo->addItem(f.second, f.first);
    }
}

void FacturationFormDialog::setClients(const QList<QPair<int, QString>>& clients) {
    clientCombo->clear();
    for (const auto& c : clients) {
        clientCombo->addItem(c.second, c.first);
    }
}
