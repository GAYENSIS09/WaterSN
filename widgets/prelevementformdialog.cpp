#include "widgets/prelevementformdialog.h"
#include <QFormLayout>
#include <QMessageBox>

PrelevementFormDialog::PrelevementFormDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUI();
    connectSignals();
}

void PrelevementFormDialog::setupUI() {
    compteurCombo = new QComboBox(this);
    ancienIndexEdit = new QLineEdit(this);
    ancienIndexEdit->setReadOnly(true);
    nouveauIndexEdit = new QLineEdit(this);
    dateEdit = new QDateEdit(QDate::currentDate(), this);
    dateEdit->setCalendarPopup(true);
    okButton = new QPushButton("Valider", this);
    cancelButton = new QPushButton("Annuler", this);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("Compteur transféré:", compteurCombo);
    formLayout->addRow("Ancien index:", ancienIndexEdit);
    formLayout->addRow("Nouveau index:", nouveauIndexEdit);
    formLayout->addRow("Date:", dateEdit);
    formLayout->addRow(okButton, cancelButton);
    setLayout(formLayout);
}

void PrelevementFormDialog::connectSignals() {
    connect(compteurCombo, &QComboBox::currentTextChanged, this, [this](const QString& numCompteur){
        // Charger l'ancien index réel depuis la base de données
        QSqlQuery query;
        query.prepare("SELECT ancienindex FROM Compteur WHERE numCompteur = ?");
        query.addBindValue(numCompteur);
        if (query.exec() && query.next()) {
            double ancienIndex = query.value(0).toDouble();
            setAncienIndex(ancienIndex);
        } else {
            setAncienIndex(0.0);
        }
    });
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

QString PrelevementFormDialog::getCompteur() const {
    return compteurCombo->currentText();
}

double PrelevementFormDialog::getAncienIndex() const {
    return ancienIndexEdit->text().toDouble();
}

double PrelevementFormDialog::getNouveauIndex() const {
    return nouveauIndexEdit->text().toDouble();
}

QDate PrelevementFormDialog::getDate() const {
    return dateEdit->date();
}

void PrelevementFormDialog::setCompteurs(const QStringList& compteurs) {
    compteurCombo->clear();
    compteurCombo->addItems(compteurs);
}

void PrelevementFormDialog::setAncienIndex(double index) {
    ancienIndexEdit->setText(QString::number(index));
}
