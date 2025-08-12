#include "widgets/prelevementwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>

PrelevementWidget::PrelevementWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    loadPrelevements();
}

void PrelevementWidget::setupUI() {
    tableView = new QTableView(this);
    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"ID", "Date", "Compteur", "Nouvel Index", "Ancien Index"});
    tableView->setModel(model);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setAlternatingRowColors(true);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Rendre toutes les colonnes en lecture seule
    for (int row = 0; row < model->rowCount(); ++row) {
        for (int col = 0; col < model->columnCount(); ++col) {
            QStandardItem* item = model->item(row, col);
            if (item) {
                item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            }
        }
    }

    addButton = new QPushButton("Ajouter un prélèvement", this);
    filterEdit = new QLineEdit(this);
    filterEdit->setPlaceholderText("Filtrer par compteur...");

    connect(addButton, &QPushButton::clicked, this, &PrelevementWidget::addPrelevement);
    connect(filterEdit, &QLineEdit::textChanged, this, &PrelevementWidget::filterPrelevements);

    QHBoxLayout* topLayout = new QHBoxLayout;
    topLayout->addWidget(filterEdit);
    topLayout->addWidget(addButton);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(tableView);
    setLayout(mainLayout);

    // Formulaire en bas
    QWidget* formWidget = new QWidget(this);
    QFormLayout* formLayout = new QFormLayout(formWidget);

    // ComboBox des compteurs transférés
    QComboBox* compteurCombo = new QComboBox(formWidget);
    QSqlQuery queryCompteurs("SELECT numCompteur FROM Compteur WHERE attributComp = 'Transféré'");
    while (queryCompteurs.next()) {
        compteurCombo->addItem(queryCompteurs.value(0).toString());
    }

    // Ancien index (lecture seule)
    QLineEdit* ancienIndexEdit = new QLineEdit(formWidget);
    ancienIndexEdit->setReadOnly(true);

    // Nouvel index
    QLineEdit* nouvelIndexEdit = new QLineEdit(formWidget);

    // Date (QDateEdit)
    QDateEdit* dateEdit = new QDateEdit(QDate::currentDate(), formWidget);
    dateEdit->setCalendarPopup(true);

    // Bouton d'ajout
    QPushButton* ajouterBtn = new QPushButton("Ajouter", formWidget);

    formLayout->addRow("Compteur", compteurCombo);
    formLayout->addRow("Ancien index", ancienIndexEdit);
    formLayout->addRow("Nouvel index", nouvelIndexEdit);
    formLayout->addRow("Date", dateEdit);
    formLayout->addRow(ajouterBtn);
    mainLayout->addWidget(formWidget);

    // Mise à jour de l'ancien index selon le compteur sélectionné
    connect(compteurCombo, &QComboBox::currentTextChanged, this, [=](const QString& numCompteur){
        QSqlQuery queryIndex;
        queryIndex.prepare("SELECT ancienindex FROM Compteur WHERE numCompteur = ?");
        queryIndex.addBindValue(numCompteur);
        if (queryIndex.exec() && queryIndex.next()) {
            double ancienIndex = queryIndex.value(0).toDouble();
            ancienIndexEdit->setText(QString::number(ancienIndex));
        } else {
            ancienIndexEdit->setText("0");
        }
    });
    // Initialiser l'ancien index au démarrage
    if (compteurCombo->count() > 0)
        emit compteurCombo->currentTextChanged(compteurCombo->currentText());

    // Ajout du prélèvement
    connect(ajouterBtn, &QPushButton::clicked, this, [=]() {
        int newId = model->rowCount() + 1;
        QList<QStandardItem*> items;
        items << new QStandardItem(QString::number(newId));
        items << new QStandardItem(dateEdit->date().toString("yyyy-MM-dd"));
        items << new QStandardItem(compteurCombo->currentText());
        items << new QStandardItem(nouvelIndexEdit->text());
        items << new QStandardItem(ancienIndexEdit->text());
        model->appendRow(items);
        QMessageBox::information(this, "Ajout", "Prélèvement ajouté !");
        // Optionnel : vider le champ nouvel index
        nouvelIndexEdit->clear();
    });
}

void PrelevementWidget::loadPrelevements() {
    // Exemple statique, à remplacer par la récupération depuis la base de données
    QList<QList<QVariant>> data = {
        {1, "2025-08-01", "CPT001", 1200.5, 1100.0},
        {2, "2025-08-05", "CPT002", 800.0, 750.0},
        {3, "2025-08-07", "CPT003", 950.0, 900.0}
    };
    model->setRowCount(0);
    for (const auto& row : data) {
        QList<QStandardItem*> items;
        for (const auto& value : row)
            items << new QStandardItem(value.toString());
        model->appendRow(items);
    }
}

void PrelevementWidget::addPrelevement() {
    // Ouverture du formulaire réel
    PrelevementFormDialog dialog(this);
    // Charger la liste des compteurs transférés depuis la base
    QStringList compteursTransfere;
    QSqlQuery queryCompteurs("SELECT numCompteur FROM Compteur WHERE attributComp = 'Transféré'");
    while (queryCompteurs.next()) {
        compteursTransfere << queryCompteurs.value(0).toString();
    }
    dialog.setCompteurs(compteursTransfere);

    // Connecter la sélection du compteur à la mise à jour de l'ancien index
    QObject::connect(dialog.compteurCombo, &QComboBox::currentTextChanged, &dialog, [&](const QString& numCompteur){
        QSqlQuery queryIndex;
        queryIndex.prepare("SELECT ancienindex FROM Compteur WHERE numCompteur = ?");
        queryIndex.addBindValue(numCompteur);
        if (queryIndex.exec() && queryIndex.next()) {
            double ancienIndex = queryIndex.value(0).toDouble();
            dialog.setAncienIndex(ancienIndex);
        } else {
            dialog.setAncienIndex(0.0);
        }
    });

    if (dialog.exec() == QDialog::Accepted) {
        int newId = model->rowCount() + 1;
        QList<QStandardItem*> items;
        items << new QStandardItem(QString::number(newId));
        items << new QStandardItem(dialog.getDate().toString("yyyy-MM-dd"));
        items << new QStandardItem(dialog.getCompteur());
        items << new QStandardItem(QString::number(dialog.getNouveauIndex()));
        items << new QStandardItem(QString::number(dialog.getAncienIndex()));
        model->appendRow(items);
        QMessageBox::information(this, "Ajout", "Prélèvement ajouté !");
    }
}

void PrelevementWidget::filterPrelevements(const QString& text) {
    for (int row = 0; row < model->rowCount(); ++row) {
        bool match = model->item(row, 2)->text().contains(text, Qt::CaseInsensitive);
        tableView->setRowHidden(row, !match && !text.isEmpty());
    }
}
