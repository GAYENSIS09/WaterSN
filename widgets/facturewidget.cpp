#include "widgets/facturewidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QLineEdit>
#include <QPushButton>
#include <QTableView>
#include <QStandardItemModel>

FactureWidget::FactureWidget(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* topLayout = new QHBoxLayout;
    QLineEdit* filterEdit = new QLineEdit;
    filterEdit->setPlaceholderText("Filtrer par client ou compteur...");
    QPushButton* addBtn = new QPushButton("Ajouter une facture");
    topLayout->addWidget(filterEdit);
    topLayout->addWidget(addBtn);
    mainLayout->addLayout(topLayout);

    QTableView* tableView = new QTableView(this);
    QStandardItemModel* model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"ID", "Compteur", "Client", "Solde antérieur", "Consommation"});
    tableView->setModel(model);
    tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(tableView);

    // Exemple de données
    auto chargerFactures = [&]() {
        model->setRowCount(0);
        QList<QList<QVariant>> data = {
            {1, "CPT001", "1", 500.0, 100.0},
            {2, "CPT002", "2", 200.0, 50.0},
            {3, "CPT003", "3", 300.0, 80.0}
        };
        for (const auto& row : data) {
            QList<QStandardItem*> items;
            for (const auto& value : row)
                items << new QStandardItem(value.toString());
            model->appendRow(items);
        }
    };
    chargerFactures();

    QObject::connect(addBtn, &QPushButton::clicked, this, [=]() {
        FacturationFormDialog dialog(this);
        // Charger la liste des compteurs transférés
        QStringList compteursTransfere;
        QSqlQuery queryCompteurs("SELECT numCompteur FROM Compteur WHERE attributComp = 'Transféré'");
        while (queryCompteurs.next()) {
            compteursTransfere << queryCompteurs.value(0).toString();
        }
        dialog.setCompteurs(compteursTransfere);
        if (dialog.exec() == QDialog::Accepted) {
            // Insertion réelle en base
            QSqlQuery insert;
            insert.prepare("INSERT INTO Facturation (date_paiement, numCompteur, idFacture, idClient, mensualite) VALUES (?, ?, ?, ?, ?)");
            insert.addBindValue(dialog.getDatePaiement());
            insert.addBindValue(dialog.getNumCompteur());
            insert.addBindValue(dialog.getIdFacture());
            insert.addBindValue(dialog.getIdClient());
            insert.addBindValue(dialog.getMensualite());
            if (insert.exec()) {
                // Ajout dans le tableau local
                int newId = model->rowCount() + 1;
                QList<QStandardItem*> items;
                items << new QStandardItem(QString::number(newId));
                items << new QStandardItem(dialog.getNumCompteur());
                items << new QStandardItem(QString::number(dialog.getIdClient()));
                items << new QStandardItem(QString::number(dialog.getMensualite()));
                items << new QStandardItem(QString::number(dialog.getIdFacture()));
                model->appendRow(items);
                QMessageBox::information(this, "Ajout", "Facturation ajoutée !");
            } else {
                QMessageBox::critical(this, "Erreur", "Échec de l'ajout en base : " + insert.lastError().text());
            }
        }
    });
    QObject::connect(filterEdit, &QLineEdit::textChanged, this, [=](const QString& text) {
        for (int row = 0; row < model->rowCount(); ++row) {
            bool match = model->item(row, 1)->text().contains(text, Qt::CaseInsensitive)
                || model->item(row, 2)->text().contains(text, Qt::CaseInsensitive);
            tableView->setRowHidden(row, !match && !text.isEmpty());
        }
    });
}
