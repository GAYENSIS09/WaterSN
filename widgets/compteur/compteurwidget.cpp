#include "widgets/compteurwidget.h"
#include <QVBoxLayout>
#include <QTabWidget>
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>

#include <QDateEdit>
#include <QSqlQuery>
#include <QSqlError>


#include "widgets/actionsdelegate.h"

// ...existing code...

void CompteurWidget::refreshAllTabs()
{
    // Rafraîchit tous les onglets (utile pour navigation principale)
    // chargerListeCompteurs();
    // chargerListePrelevements();
    // chargerListeFactures();
}

CompteurWidget::CompteurWidget(Controller *controller, QWidget *parent)
    : QWidget(parent)
{
    // ...existing code...
    // Création des widgets de filtre et du tableau
    // ...existing code...
    // Membres de la classe pour widgets de filtrage et table
    this->filterCombo = new QComboBox;
    this->filterCombo->addItem("Tous");
    this->filterCombo->addItem("Disponible");
    this->filterCombo->addItem("Archivé");
    this->filterCombo->addItem("Transféré");
    this->searchCompteurEdit = new QLineEdit;
    this->searchCompteurEdit->setPlaceholderText("Rechercher numéro compteur...");
    this->searchCompteurEdit->setMinimumWidth(160);
    this->compteursTable = new QTableView;
    this->compteursTable->setObjectName("compteursTable");
    // Le modèle sera créé dans chargerListeCompteurs
    QStandardItemModel *compteursModel = new QStandardItemModel(0, 5, this);
    compteursModel->setHorizontalHeaderLabels({"Numéro", "Attribut", "Ancien index", "Série", "Actions"});
    this->compteursTable->setModel(compteursModel);
    this->compteursTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->compteursTable->setAlternatingRowColors(true);
    this->compteursTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    this->compteursTable->setColumnWidth(4, 90);
    // ...existing code...
    // Déclaration de la lambda et connexions juste après la création des widgets
    // Lambda de filtrage utilisant les membres
    auto filtrerTableCompteurs = [this]() {
        QAbstractItemModel *rawModel = this->compteursTable->model();
        QStandardItemModel *model = qobject_cast<QStandardItemModel *>(rawModel);
        if (!model)
            return;
        QString text = this->searchCompteurEdit->text();
        QString etat = this->filterCombo->currentText();
        for (int row = 0; row < model->rowCount(); ++row) {
            bool matchNum = model->item(row, 0)->text().contains(text, Qt::CaseInsensitive);
            bool matchEtat = (etat == "Tous") || (model->item(row, 1)->text() == etat);
            this->compteursTable->setRowHidden(row, !(matchNum && matchEtat));
        }
    };
    QObject::connect(this->searchCompteurEdit, &QLineEdit::textChanged, this, filtrerTableCompteurs);
    QObject::connect(this->filterCombo, &QComboBox::currentTextChanged, this, filtrerTableCompteurs);
    qDebug() << "[mlog] Début constructeur CompteurWidget";
    qDebug() << "[mlog] Adresse this=" << this;
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QTabWidget *tabWidget = new QTabWidget(this);

    // Onglet Compteur avec filtrage par état
    QWidget *compteurTab = new QWidget;
    QVBoxLayout *ajoutLayout = new QVBoxLayout(compteurTab);
    // ... formulaire d'ajout ...
    // Filtre d'état déplacé sous le formulaire
    QHBoxLayout *filterLayout = new QHBoxLayout;
    QLabel *filterLabel = new QLabel("Filtrer par état :");
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(this->filterCombo);
    filterLayout->addWidget(this->searchCompteurEdit);
    ajoutLayout->addLayout(filterLayout);
    // Tableau des compteurs
    ajoutLayout->addWidget(this->compteursTable);

    // Formulaire d'ajout de compteur
    QWidget *formWidget = new QWidget;
    QFormLayout *formLayout = new QFormLayout(formWidget);
    QLineEdit *numCompteurEdit = new QLineEdit;
    QComboBox *attributCompCombo = new QComboBox;
    attributCompCombo->addItems({"Disponible", "Archivé", "Transféré"});
    // Ancien index retiré du formulaire, valeur par défaut 0
    QLineEdit *serieEdit = new QLineEdit;
    formLayout->addRow("Numéro de compteur:", numCompteurEdit);
    formLayout->addRow("Attribut:", attributCompCombo);
    // Ancien index non affiché, sera 0 par défaut
    formLayout->addRow("Série:", serieEdit);
    QPushButton *addBtn = new QPushButton("Ajouter");
    formLayout->addRow(addBtn);
    ajoutLayout->addWidget(formWidget);

    // Méthode pour insérer une ligne dans le modèle de la table compteurs
    auto insererLigneCompteurDansTable = [this](const QString &numCompteur, const QString &attributComp, double ancienIndex, const QString &serie)
    {
        qDebug() << "[mlog] insererLigneCompteurDansTable appelé";
        qDebug() << "[mlog] Paramètres reçus : numCompteur=" << numCompteur << ", attributComp=" << attributComp << ", ancienIndex=" << ancienIndex << ", serie=" << serie;
        if (!this->compteursTable)
        {
            qDebug() << "[mlog] ERREUR: compteursTable est nul ! Impossible d'accéder au modèle.";
            return;
        }
        QAbstractItemModel *rawModel = this->compteursTable->model();
        qDebug() << "[mlog] compteursTable->model() =" << rawModel << ", type =" << (rawModel ? rawModel->metaObject()->className() : "nullptr");
        QStandardItemModel *model = qobject_cast<QStandardItemModel *>(rawModel);
        if (!model)
        {
            qDebug() << "[mlog] ERREUR: Le modèle n'est pas un QStandardItemModel ou est nul ! On recrée un modèle vierge pour éviter le crash.";
            model = new QStandardItemModel(0, 5, this->compteursTable);
            model->setHorizontalHeaderLabels({"Numéro", "Attribut", "Ancien index", "Série", "Actions"});
            this->compteursTable->setModel(model);
        }
        // Vérifier si le compteur existe déjà dans le modèle
        for (int i = 0; i < model->rowCount(); ++i)
        {
            if (model->item(i, 0) && model->item(i, 0)->text() == numCompteur)
            {
                qDebug() << "[mlog] Doublon détecté dans le modèle, insertion ignorée.";
                return;
            }
        }
        int row = model->rowCount();
        qDebug() << "[mlog] Insertion nouvelle ligne row=" << row << ", numCompteur=" << numCompteur;
        model->insertRow(row);
        {
            QStandardItem *item0 = new QStandardItem(numCompteur);
            item0->setFlags(item0->flags() & ~Qt::ItemIsEditable);
            model->setItem(row, 0, item0);
            QStandardItem *item1 = new QStandardItem(attributComp);
            item1->setFlags(item1->flags() & ~Qt::ItemIsEditable);
            model->setItem(row, 1, item1);
            QStandardItem *item2 = new QStandardItem(QString::number(ancienIndex));
            item2->setFlags(item2->flags() | Qt::ItemIsEditable);
            model->setItem(row, 2, item2);
            QStandardItem *item3 = new QStandardItem(serie);
            item3->setFlags(item3->flags() | Qt::ItemIsEditable);
            model->setItem(row, 3, item3);
            QStandardItem *actionItem = new QStandardItem;
            actionItem->setFlags(actionItem->flags() & ~Qt::ItemIsEditable);
            model->setItem(row, 4, actionItem);
        }
        qDebug() << "[mlog] Ligne insérée dans le modèle :" << numCompteur << attributComp << ancienIndex << serie;
    };

    // Chargement réel des compteurs depuis la BDD
    auto chargerListeCompteurs = [this, filtrerTableCompteurs]() {
        qDebug() << "[mlog] chargerListeCompteurs()";
        if (!this->compteursTable) {
            qDebug() << "[mlog] ERREUR: compteursTable introuvable !";
            return;
        }
        QStandardItemModel *newModel = new QStandardItemModel(0, 5, this->compteursTable);
        newModel->setHorizontalHeaderLabels({"Numéro", "Attribut", "Ancien index", "Série", "Actions"});
        QSqlQuery query("SELECT numCompteur, attributComp, ancienindex, serie FROM Compteur");
        int row = 0;
        while (query.next()) {
            newModel->insertRow(row);
            int modelCols = newModel->columnCount();
            for (int col = 0; col < 4 && col < modelCols; ++col) {
                QStandardItem *item = new QStandardItem;
                item->setText(query.value(col).toString());
                if (col == 3)
                    item->setFlags(item->flags() | Qt::ItemIsEditable);
                else
                    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                newModel->setItem(row, col, item);
            }
            if (modelCols > 4) {
                QStandardItem *actionItem = new QStandardItem;
                actionItem->setFlags(actionItem->flags() & ~Qt::ItemIsEditable);
                newModel->setItem(row, 4, actionItem);
            }
            row++;
        }
        QAbstractItemModel *oldModel = this->compteursTable->model();
        this->compteursTable->setModel(newModel);
        if (oldModel && oldModel != newModel) {
            int colCount = oldModel->columnCount();
            for (int col = 0; col < colCount; ++col) {
                QAbstractItemDelegate *oldDelegate = this->compteursTable->itemDelegateForColumn(col);
                if (oldDelegate) {
                    this->compteursTable->setItemDelegateForColumn(col, nullptr);
                    QObject::disconnect(oldDelegate, nullptr, nullptr, nullptr);
                    delete oldDelegate;
                }
            }
            delete oldModel;
        }
        this->compteursTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        this->compteursTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
        this->compteursTable->setColumnWidth(4, 120);
        MyActionsDelegate *actionsDelegate = new MyActionsDelegate(this->compteursTable);
        this->compteursTable->setItemDelegateForColumn(4, actionsDelegate);
        QObject::connect(actionsDelegate, &MyActionsDelegate::emitArchiver, this,
                         [this, actionsDelegate, newModel](const QModelIndex &index) {
            if (!newModel) return;
            QString numCompteur = newModel->item(index.row(), 0)->text();
            int ret = QMessageBox::question(this, "Archiver", "Voulez-vous vraiment archiver le compteur : " + numCompteur + " ?", QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE Compteur SET attributComp = 'Archivé' WHERE numCompteur = ?");
                updateQuery.addBindValue(numCompteur);
                if (!updateQuery.exec()) {
                    QMessageBox::critical(this, "Erreur SQL", "Impossible d'archiver le compteur : " + updateQuery.lastError().text());
                    return;
                }
                newModel->setItem(index.row(), 1, new QStandardItem("Archivé"));
                QMessageBox::information(this, "Archivé", "Le compteur a été archivé.");
            }
        });
        QObject::connect(newModel, &QStandardItemModel::itemChanged, this,
                         [this, newModel](QStandardItem *item) {
            if (!newModel || !item) return;
            int row = item->row();
            int col = item->column();
            if (col == 3) {
                QString numCompteur = newModel->item(row, 0)->text();
                QString nouvelleSerie = item->text();
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE Compteur SET serie = ? WHERE numCompteur = ?");
                updateQuery.addBindValue(nouvelleSerie);
                updateQuery.addBindValue(numCompteur);
                if (updateQuery.exec()) {
                    QMessageBox::information(this, "Modification", "La série a été modifiée dans la base de données.");
                } else {
                    QMessageBox::critical(this, "Erreur SQL", "Impossible de modifier la série : " + updateQuery.lastError().text());
                }
            }
        });
        QObject::connect(actionsDelegate, &MyActionsDelegate::emitTransferer, this,
                         [this, actionsDelegate, newModel](const QModelIndex &index) {
            if (!newModel) return;
            QString numCompteur = newModel->item(index.row(), 0)->text();
            QDialog dialog(this);
            dialog.setWindowTitle("Transférer compteur");
            QVBoxLayout *layout = new QVBoxLayout(&dialog);
            QLabel *label = new QLabel("Sélectionnez le client destinataire :", &dialog);
            QComboBox *clientCombo = new QComboBox(&dialog);
            QSqlQuery queryClients("SELECT idClient, nom, prenom FROM Client");
            while (queryClients.next()) {
                QString labelText = queryClients.value(0).toString() + " - " + queryClients.value(1).toString() + " " + queryClients.value(2).toString();
                clientCombo->addItem(labelText, queryClients.value(0));
            }
            layout->addWidget(label);
            layout->addWidget(clientCombo);
            QPushButton *okBtn = new QPushButton("OK", &dialog);
            layout->addWidget(okBtn);
            QObject::connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
            if (dialog.exec() == QDialog::Accepted && clientCombo->currentIndex() >= 0) {
                QVariant idClient = clientCombo->currentData();
                QSqlQuery checkQuery;
                checkQuery.prepare("SELECT id FROM Abonnement WHERE numCompteur = ? AND idClient = ?");
                checkQuery.addBindValue(numCompteur);
                checkQuery.addBindValue(idClient);
                if (checkQuery.exec() && checkQuery.next()) {
                    QMessageBox::information(this, "Transfert", "Ce compteur est déjà relié à ce client (abonnement existant).");
                } else {
                    QSqlQuery insertQuery;
                    insertQuery.prepare("INSERT INTO Abonnement (idClient, numCompteur, date_abonnement) VALUES (?, ?, ?)");
                    insertQuery.addBindValue(idClient);
                    insertQuery.addBindValue(numCompteur);
                    insertQuery.addBindValue(QDate::currentDate());
                    if (!insertQuery.exec()) {
                        QMessageBox::critical(this, "Erreur SQL", "Impossible de créer l'abonnement : " + insertQuery.lastError().text());
                        return;
                    }
                    QMessageBox::information(this, "Transfert", "Abonnement créé entre le compteur et le client.");
                }
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE Compteur SET attributComp = 'Transféré' WHERE numCompteur = ?");
                updateQuery.addBindValue(numCompteur);
                if (!updateQuery.exec()) {
                    QMessageBox::critical(this, "Erreur SQL", "Impossible de transférer le compteur : " + updateQuery.lastError().text());
                    return;
                }
                double lastIndex = 0.0;
                QSqlQuery indexQuery;
                indexQuery.prepare("SELECT newIndex FROM Prelevement WHERE numCompteur = ? ORDER BY dateprelevement DESC LIMIT 1");
                indexQuery.addBindValue(numCompteur);
                if (indexQuery.exec() && indexQuery.next()) {
                    lastIndex = indexQuery.value(0).toDouble();
                }
                QSqlQuery updateIndexQuery;
                updateIndexQuery.prepare("UPDATE Compteur SET ancienindex = ? WHERE numCompteur = ?");
                updateIndexQuery.addBindValue(lastIndex);
                updateIndexQuery.addBindValue(numCompteur);
                if (!updateIndexQuery.exec()) {
                    QMessageBox::critical(this, "Erreur SQL", "Impossible de mettre à jour l'index du compteur : " + updateIndexQuery.lastError().text());
                }
                newModel->setItem(index.row(), 1, new QStandardItem("Transféré"));
            }
        });
        filtrerTableCompteurs();
        qDebug() << "[mlog] Fin chargerListeCompteurs()";
    };

    connect(addBtn, &QPushButton::clicked, this, [=]()
            {
                // --- Début du slot d'ajout de compteur ---
                qDebug() << "[mlog] Ajout compteur :" << numCompteurEdit->text() << attributCompCombo->currentText() << serieEdit->text();
                qDebug() << "[mlog] Adresse compteursTable (addBtn)=" << compteursTable;
                QString numCompteur = numCompteurEdit->text();
                QString attributComp = attributCompCombo->currentText();
                QString serie = serieEdit->text();
                double ancienIndex = 0; // valeur par défaut

                // Vérification des champs du formulaire
                if (numCompteur.isEmpty() || attributComp.isEmpty() || serie.isEmpty())
                {
                    qDebug() << "[mlog] Champs manquants lors de l'ajout compteur";
                    QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs.");
                    return;
                }

                // Préparation et exécution de la requête SQL d'insertion
                QSqlQuery insertCompteur;
                insertCompteur.prepare("INSERT INTO Compteur (numCompteur, attributComp, ancienindex, serie) VALUES (?, ?, ?, ?)");
                insertCompteur.addBindValue(numCompteur);
                insertCompteur.addBindValue(attributComp);
                insertCompteur.addBindValue(ancienIndex);
                insertCompteur.addBindValue(serie);
                if (!insertCompteur.exec())
                {
                    // Si erreur SQL, afficher le message et arrêter
                    qDebug() << "[mlog] Erreur SQL lors de l'ajout compteur :" << insertCompteur.lastError().text();
                    QMessageBox::critical(this, "Erreur SQL", "Impossible d'insérer le compteur : " + insertCompteur.lastError().text());
                    return;
                }

                // Si l'insertion SQL réussit, afficher une confirmation
                qDebug() << "[mlog] Compteur ajouté en base :" << numCompteur << attributComp << ancienIndex << serie;
                QMessageBox::information(this, "Ajout", "Compteur ajouté en base : " + numCompteur + ", " + attributComp + ", " + QString::number(ancienIndex) + ", " + serie);

                // Réinitialiser le formulaire
                numCompteurEdit->clear();
                attributCompCombo->setCurrentIndex(0);
                serieEdit->clear();

                // --- Étape critique : insertion dans le modèle Qt (affichage) ---
                // Log des paramètres transmis à la méthode d'insertion
                qDebug() << "[mlog] Appel insererLigneCompteurDansTable avec : numCompteur=" << numCompteur << ", attributComp=" << attributComp << ", ancienIndex=" << ancienIndex << ", serie=" << serie;
                // Cette méthode ajoute la ligne dans le tableau sans recharger toute la liste
                // Si un crash survient ici, il est probablement lié à la gestion mémoire ou au modèle
                insererLigneCompteurDansTable(numCompteur, attributComp, ancienIndex, serie);
                // --- Fin du slot d'ajout de compteur ---
                // Remarque : on n'appelle pas chargerListeCompteurs() pour éviter de recréer le modèle complet
                // Cela permet d'avoir un affichage instantané et d'éviter les ralentissements
            });
    tabWidget->addTab(compteurTab, "Compteur");

    // Onglet Prélèvements
    QWidget *prelevTab = new QWidget;
    QVBoxLayout *prelevLayout = new QVBoxLayout(prelevTab);
    QHBoxLayout *prelevTopLayout = new QHBoxLayout;
    this->prelevementSearchBar = new QLineEdit;
    this->prelevementSearchBar->setPlaceholderText("Filtrer par compteur...");
    prelevTopLayout->addWidget(this->prelevementSearchBar);
    prelevLayout->addLayout(prelevTopLayout);
    QTableView *prelevTable = new QTableView;
    prelevTable->setObjectName("prelevTable");
    QStandardItemModel *prelevModel = new QStandardItemModel(0, 5, this);
    prelevModel->setHorizontalHeaderLabels({"ID", "Date", "Compteur", "Nouvel Index", "Ancien Index"});
    prelevTable->setModel(prelevModel);
    prelevTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    prelevTable->setSelectionMode(QAbstractItemView::SingleSelection);
    prelevTable->setAlternatingRowColors(true);
    prelevTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    prelevTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    prelevLayout->addWidget(prelevTable);
    // Connexion du signal textChanged une seule fois
    QObject::connect(this->prelevementSearchBar, &QLineEdit::textChanged, this, [prelevTable, this]() {
        QAbstractItemModel *rawModel = prelevTable->model();
        QStandardItemModel *model = qobject_cast<QStandardItemModel *>(rawModel);
        if (!model) return;
        QString text = this->prelevementSearchBar->text();
        for (int row = 0; row < model->rowCount(); ++row) {
            bool match = model->item(row, 2)->text().contains(text, Qt::CaseInsensitive);
            prelevTable->setRowHidden(row, !match && !text.isEmpty());
        }
    });

    // Formulaire d'ajout sous le tableau Prélèvements
    // Méthode pour insérer une ligne dans le modèle de la table prélèvements
    auto insererLignePrelevementDansTable = [prelevTable](const QString &id, const QString &date, const QString &compteur, const QString &nouvelIndex, const QString &ancienIndex)
    {
        qDebug() << "[mlog] insererLignePrelevementDansTable appelé";
        if (!prelevTable)
        {
            qDebug() << "[mlog] ERREUR: prelevTable est nul ! Impossible d'accéder au modèle.";
            return;
        }
        QAbstractItemModel *rawModel = prelevTable->model();
        QStandardItemModel *model = qobject_cast<QStandardItemModel *>(rawModel);
        if (!model)
        {
            qDebug() << "[mlog] ERREUR: Le modèle n'est pas un QStandardItemModel ou est nul ! On recrée un modèle vierge pour éviter le crash.";
            model = new QStandardItemModel(0, 5, prelevTable);
            model->setHorizontalHeaderLabels({"ID", "Date", "Compteur", "Nouvel Index", "Ancien Index"});
            prelevTable->setModel(model);
        }
        int row = model->rowCount();
        model->insertRow(row);
        model->setItem(row, 0, new QStandardItem(id));
        model->setItem(row, 1, new QStandardItem(date));
        model->setItem(row, 2, new QStandardItem(compteur));
        model->setItem(row, 3, new QStandardItem(nouvelIndex));
        model->setItem(row, 4, new QStandardItem(ancienIndex));
        qDebug() << "[mlog] Ligne insérée dans le modèle prélèvements :" << id << date << compteur << nouvelIndex << ancienIndex;
    };
    QWidget *prelevFormWidget = new QWidget;
    QFormLayout *prelevFormLayout = new QFormLayout(prelevFormWidget);
    // QDateEdit pour la date
    QDateEdit *prelevDateEdit = new QDateEdit(QDate::currentDate(), prelevFormWidget);
    prelevDateEdit->setCalendarPopup(true);
    // QComboBox pour compteur transféré
    QComboBox *prelevCompteurCombo = new QComboBox(prelevFormWidget);
    // Fonction pour rafraîchir la liste des compteurs transférés
    auto updatePrelevCompteurCombo = [prelevCompteurCombo]() {
        prelevCompteurCombo->clear();
        QSqlQuery queryCompteursPrelev("SELECT numCompteur FROM Compteur WHERE attributComp = 'Transféré'");
        while (queryCompteursPrelev.next()) {
            prelevCompteurCombo->addItem(queryCompteursPrelev.value(0).toString());
        }
    };
    updatePrelevCompteurCombo();
    // Ancien index (lecture seule)
    QLineEdit *prelevAncienIndexEdit = new QLineEdit(prelevFormWidget);
    prelevAncienIndexEdit->setReadOnly(true);
    // Nouvel index
    QLineEdit *prelevNouvelIndexEdit = new QLineEdit(prelevFormWidget);
    QPushButton *prelevFormAddBtn = new QPushButton("Ajouter");
    prelevFormLayout->addRow("Date:", prelevDateEdit);
    prelevFormLayout->addRow("Compteur:", prelevCompteurCombo);
    prelevFormLayout->addRow("Ancien index:", prelevAncienIndexEdit);
    prelevFormLayout->addRow("Nouvel index:", prelevNouvelIndexEdit);
    prelevFormLayout->addRow(prelevFormAddBtn);
    prelevLayout->addWidget(prelevFormWidget);

    // Chargement réel des prélèvements depuis la BDD
    auto chargerListePrelevements = [&]()
    {
        qDebug() << "[mlog] chargerListePrelevements()";

        QTableView *prelevTable = this->findChild<QTableView *>("prelevTable");
        if (!prelevTable)
        {
            qDebug() << "[mlog] ERREUR: prelevTable introuvable !";
            return;
        }
        if (!QSqlDatabase::database().isValid() || !QSqlDatabase::database().isOpen())
        {
            qDebug() << "[mlog] ERREUR: La connexion à la base de données est invalide ou fermée !";
            return;
        }
        QStandardItemModel *newPrelevModel = new QStandardItemModel(0, 5, prelevTable);
        newPrelevModel->setHorizontalHeaderLabels({"ID", "Date", "Compteur", "Nouvel Index", "Ancien Index"});
        QSqlQuery query;
        if (!query.prepare("SELECT idpreleve, dateprelevement, numCompteur, newIndex, ancienIndex FROM Prelevement"))
        {
            qDebug() << "[mlog] ERREUR: prepare SQL échoué :" << query.lastError().text();
            return;
        }
        if (!query.exec())
        {
            qDebug() << "[mlog] ERREUR: exec SQL échoué :" << query.lastError().text();
            return;
        }
        int rowCount = 0;
        while (query.next())
        {
            QList<QStandardItem *> items;
            for (int i = 0; i < 5; ++i)
            {
                QStandardItem *item = new QStandardItem(query.value(i).toString());
                if (i == 3 || i == 4)
                    item->setFlags(item->flags() | Qt::ItemIsEditable);
                else
                    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                items << item;
            }
            newPrelevModel->appendRow(items);
            rowCount++;
        }
        prelevTable->setModel(newPrelevModel);
        prelevTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        prelevTable->setSelectionMode(QAbstractItemView::SingleSelection);
        prelevTable->setAlternatingRowColors(true);
        prelevTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        prelevTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
        QObject::connect(newPrelevModel, &QStandardItemModel::itemChanged, this, [=](QStandardItem *item)
                         {
            if (!item) return;
            int row = item->row();
            int col = item->column();
            if (col == 3 || col == 4) {
                QStandardItem *idItem = newPrelevModel->item(row, 0);
                QStandardItem *nouvelItem = newPrelevModel->item(row, 3);
                QStandardItem *ancienItem = newPrelevModel->item(row, 4);
                if (!idItem || !nouvelItem || !ancienItem) return;
                QString idPreleve = idItem->text();
                QString nouvelIndex = nouvelItem->text();
                QString ancienIndex = ancienItem->text();
                if (nouvelIndex.trimmed().isEmpty() || ancienIndex.trimmed().isEmpty()) {
                    QMessageBox::warning(this, "Erreur", "Les index ne doivent pas être vides.");
                    return;
                }
                bool okAncien = false, okNouvel = false;
                double valAncien = ancienIndex.toDouble(&okAncien);
                double valNouvel = nouvelIndex.toDouble(&okNouvel);
                if (!okAncien || !okNouvel || nouvelIndex.trimmed().isEmpty() || ancienIndex.trimmed().isEmpty()) {
                    QMessageBox::warning(this, "Erreur", "Les index doivent être des valeurs numériques et non vides.");
                    return;
                }
                if (valAncien >= valNouvel) {
                    QMessageBox::warning(this, "Erreur", "Le nouvel index doit être strictement supérieur à l'ancien index.");
                    QSqlQuery query;
                    query.prepare("SELECT newIndex, ancienIndex FROM Prelevement WHERE idpreleve = ?");
                    query.addBindValue(idPreleve);
                    if (query.exec() && query.next()) {
                        nouvelItem->setText(query.value(0).toString());
                        ancienItem->setText(query.value(1).toString());
                    }
                    return;
                }
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE Prelevement SET newIndex = ?, ancienIndex = ? WHERE idpreleve = ?");
                updateQuery.addBindValue(nouvelIndex);
                updateQuery.addBindValue(ancienIndex);
                updateQuery.addBindValue(idPreleve);
                if (updateQuery.exec()) {
                    QMessageBox::information(this, "Modification", "Les index ont été modifiés dans la base de données.");
                } else {
                    QMessageBox::critical(this, "Erreur SQL", "Impossible de modifier les index : " + updateQuery.lastError().text());
                }
            }
        });
    };

    // Mise à jour de l'ancien index selon le compteur sélectionné
    QObject::connect(prelevCompteurCombo, &QComboBox::currentTextChanged, prelevFormWidget, [=](const QString &numCompteur)
                     {
        QSqlQuery queryIndex;
        queryIndex.prepare("SELECT ancienindex FROM Compteur WHERE numCompteur = ?");
        queryIndex.addBindValue(numCompteur);
        if (queryIndex.exec() && queryIndex.next()) {
            double ancienIndex = queryIndex.value(0).toDouble();
            prelevAncienIndexEdit->setText(QString::number(ancienIndex));
        } else {
            prelevAncienIndexEdit->setText("0");
        } });
    // Initialiser l'ancien index au démarrage
    if (prelevCompteurCombo->count() > 0)
        emit prelevCompteurCombo->currentTextChanged(prelevCompteurCombo->currentText());

    connect(prelevFormAddBtn, &QPushButton::clicked, this, [=]()
            {
        qDebug() << "[mlog] Ajout prélèvement :" << prelevDateEdit->date() << prelevCompteurCombo->currentText() << prelevAncienIndexEdit->text() << prelevNouvelIndexEdit->text();
        QString date = prelevDateEdit->date().toString("yyyy-MM-dd");
        QString compteur = prelevCompteurCombo->currentText();
        QString ancienIndex = prelevAncienIndexEdit->text();
        QString nouvelIndex = prelevNouvelIndexEdit->text();
        if (date.isEmpty() || compteur.isEmpty() || nouvelIndex.isEmpty() || ancienIndex.isEmpty()) {
            qDebug() << "[mlog] Champs manquants lors de l'ajout prélèvement";
            QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs.");
            return;
        }
        bool okAncien = false, okNouvel = false;
        double ancienVal = ancienIndex.toDouble(&okAncien);
        double nouvelVal = nouvelIndex.toDouble(&okNouvel);
        if (!okAncien || !okNouvel) {
            QMessageBox::warning(this, "Erreur", "Les index doivent être des valeurs numériques.");
            return;
        }
        if (ancienVal >= nouvelVal) {
            QMessageBox::warning(this, "Erreur", "Le nouvel index doit être strictement supérieur à l'ancien index.");
            return;
        }
        QSqlQuery insertPrelevement;
        insertPrelevement.prepare("INSERT INTO Prelevement (dateprelevement, numCompteur, newIndex, ancienIndex) VALUES (?, ?, ?, ?)");
        insertPrelevement.addBindValue(date);
        insertPrelevement.addBindValue(compteur);
        insertPrelevement.addBindValue(nouvelVal);
        insertPrelevement.addBindValue(ancienVal);
        if (!insertPrelevement.exec()) {
            qDebug() << "[mlog] Erreur SQL lors de l'ajout prélèvement :" << insertPrelevement.lastError().text();
            QMessageBox::critical(this, "Erreur SQL", "Impossible d'insérer le prélèvement : " + insertPrelevement.lastError().text());
            return;
        }
        // Mise à jour du compteur uniquement après l'insertion du prélèvement
        QSqlQuery updateCompteur;
        updateCompteur.prepare("UPDATE Compteur SET ancienindex = ? WHERE numCompteur = ?");
        updateCompteur.addBindValue(nouvelVal);
        updateCompteur.addBindValue(compteur);
        if (!updateCompteur.exec()) {
            qDebug() << "[mlog] Erreur SQL lors de la mise à jour compteur :" << updateCompteur.lastError().text();
            QMessageBox::critical(this, "Erreur SQL", "Impossible de mettre à jour le compteur : " + updateCompteur.lastError().text());
            return;
        }
        // Ajout direct dans le modèle Qt pour affichage instantané
        insererLignePrelevementDansTable("", date, compteur, nouvelIndex, ancienIndex);
        qDebug() << "[mlog] Prélèvement ajouté et compteur mis à jour (ligne insérée dans le modèle).";
        QMessageBox::information(this, "Ajout", "Prélèvement ajouté et compteur mis à jour.");
        prelevNouvelIndexEdit->clear(); });
    // Connexion du signal textChanged déjà faite dans le constructeur, ne pas répéter ici pour éviter les crashs.
    tabWidget->addTab(prelevTab, "Prélèvements");

    // Onglet Facturation
    QWidget *factureTab = new QWidget;
    QVBoxLayout *factureLayout = new QVBoxLayout(factureTab);
    QHBoxLayout *factureTopLayout = new QHBoxLayout;
    this->facturationSearchBar = new QLineEdit;
    this->facturationSearchBar->setPlaceholderText("Filtrer par nom ou prénom client...");
    factureTopLayout->addWidget(this->facturationSearchBar);
    factureLayout->addLayout(factureTopLayout);
    QTableView *factureTable = new QTableView;

    factureTable->setObjectName("factureTable");

    QStandardItemModel *factureModel = new QStandardItemModel(0, 6, this);
    factureModel->setHorizontalHeaderLabels({"ID", "Date paiement", "Compteur", "Facture", "Client", "Mensualité"});
    factureTable->setModel(factureModel);
    factureTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    factureTable->setSelectionMode(QAbstractItemView::SingleSelection);
    factureTable->setAlternatingRowColors(true);
    factureTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    factureTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    factureLayout->addWidget(factureTable);
    // Connexion du signal textChanged une seule fois
    QObject::connect(this->facturationSearchBar, &QLineEdit::textChanged, this, [factureTable, this]() {
        QAbstractItemModel *rawModel = factureTable->model();
        QStandardItemModel *model = qobject_cast<QStandardItemModel *>(rawModel);
        if (!model) return;
        QString text = this->facturationSearchBar->text();
        for (int row = 0; row < model->rowCount(); ++row) {
            QString clientCell = model->item(row, 4)->text();
            bool matchNomPrenom = clientCell.contains(text, Qt::CaseInsensitive);
            factureTable->setRowHidden(row, !matchNomPrenom && !text.isEmpty());
        }
    });
    // Formulaire d'ajout sous le tableau Facturation
    // Méthode pour insérer une ligne dans le modèle de la table factures
    auto insererLigneFactureDansTable = [factureTable](const QString &id, const QString &datePaiement, const QString &numCompteur, const QString &idFacture, const QString &idClient, const QString &mensualite)
    {
        qDebug() << "[mlog] insererLigneFactureDansTable appelé";
        if (!factureTable)
        {
            qDebug() << "[mlog] ERREUR: factureTable est nul ! Impossible d'accéder au modèle.";
            return;
        }
        QAbstractItemModel *rawModel = factureTable->model();
        QStandardItemModel *model = qobject_cast<QStandardItemModel *>(rawModel);
        if (!model)
        {
            qDebug() << "[mlog] ERREUR: Le modèle n'est pas un QStandardItemModel ou est nul ! On recrée un modèle vierge pour éviter le crash.";
            model = new QStandardItemModel(0, 6, factureTable);
            model->setHorizontalHeaderLabels({"ID", "Date paiement", "Compteur", "Facture", "Client", "Mensualité"});
            factureTable->setModel(model);
        }
        int row = model->rowCount();
        model->insertRow(row);
        model->setItem(row, 0, new QStandardItem(id));
        model->setItem(row, 1, new QStandardItem(datePaiement));
        model->setItem(row, 2, new QStandardItem(numCompteur));
        model->setItem(row, 3, new QStandardItem(idFacture));
        model->setItem(row, 4, new QStandardItem(idClient));
        model->setItem(row, 5, new QStandardItem(mensualite));
        qDebug() << "[mlog] Ligne insérée dans le modèle factures :" << id << datePaiement << numCompteur << idFacture << idClient << mensualite;
    };
    QWidget *factureFormWidget = new QWidget;
    QFormLayout *factureFormLayout = new QFormLayout(factureFormWidget);
    // QDateEdit pour date_paiement
    QDateEdit *datePaiementEdit = new QDateEdit(QDate::currentDate(), factureFormWidget);
    datePaiementEdit->setCalendarPopup(true);
    // QComboBox pour numCompteur transféré
    QComboBox *numCompteurCombo = new QComboBox(factureFormWidget);
    QSqlQuery queryCompteurs("SELECT numCompteur FROM Compteur WHERE attributComp = 'Transféré'");
    while (queryCompteurs.next())
    {
        numCompteurCombo->addItem(queryCompteurs.value(0).toString());
    }
    // QComboBox pour idFacture
    QComboBox *idFactureCombo = new QComboBox(factureFormWidget);
    QSqlQuery queryFactures("SELECT idFacture FROM Facture");
    while (queryFactures.next())
    {
        idFactureCombo->addItem(queryFactures.value(0).toString());
    }
    // QComboBox pour idClient
    QComboBox *idClientCombo = new QComboBox(factureFormWidget);
    QSqlQuery queryClients("SELECT idClient, nom, prenom FROM Client");
    while (queryClients.next())
    {
        QString label = queryClients.value(0).toString() + " - " + queryClients.value(1).toString() + " " + queryClients.value(2).toString();
        idClientCombo->addItem(label, queryClients.value(0));
    }
        // Sélection par défaut du premier client si disponible
        if (idClientCombo->count() > 0) {
            idClientCombo->setCurrentIndex(0);
        }

    // Filtrage dynamique des compteurs et factures selon le client sélectionné
    auto updateCompteurEtFactureCombo = [=]() {
        QVariant idClient = idClientCombo->currentData();
        // Compteurs : uniquement ceux transférés ET ayant au moins une facture
        numCompteurCombo->clear();
        QSqlQuery queryCompteurs;
        queryCompteurs.prepare("SELECT c.numCompteur FROM Compteur c INNER JOIN Abonnement a ON c.numCompteur = a.numCompteur WHERE a.idClient = ? AND c.attributComp = 'Transféré' AND EXISTS (SELECT 1 FROM Facture f WHERE f.numCompteur = c.numCompteur)");
        queryCompteurs.addBindValue(idClient);
        if (queryCompteurs.exec()) {
            while (queryCompteurs.next()) {
                numCompteurCombo->addItem(queryCompteurs.value(0).toString());
            }
        }
        // Factures : initialement, on vide la liste
        idFactureCombo->clear();
        // Si un compteur est déjà sélectionné, on filtre directement
        QString numCompteurSel = numCompteurCombo->currentText();
        if (!numCompteurSel.isEmpty()) {
            QSqlQuery queryFactures;
            queryFactures.prepare("SELECT idFacture FROM Facture WHERE idClient = ? AND numCompteur = ?");
            queryFactures.addBindValue(idClient);
            queryFactures.addBindValue(numCompteurSel);
            if (queryFactures.exec()) {
                while (queryFactures.next()) {
                    idFactureCombo->addItem(queryFactures.value(0).toString());
                }
            }
        }
    };
    QObject::connect(idClientCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), factureFormWidget, updateCompteurEtFactureCombo);
    // Initialiser les combos au démarrage selon le client sélectionné
    if (idClientCombo->count() > 0) {
        updateCompteurEtFactureCombo();
    }

    // Connexion : quand le compteur change, on filtre les factures
    QObject::connect(numCompteurCombo, &QComboBox::currentTextChanged, factureFormWidget, [=](const QString &numCompteurSel){
        idFactureCombo->clear();
        QVariant idClient = idClientCombo->currentData();
        if (!numCompteurSel.isEmpty() && idClient.isValid()) {
            QSqlQuery queryFactures;
            queryFactures.prepare("SELECT idFacture FROM Facture WHERE idClient = ? AND numCompteur = ?");
            queryFactures.addBindValue(idClient);
            queryFactures.addBindValue(numCompteurSel);
            if (queryFactures.exec()) {
                while (queryFactures.next()) {
                    idFactureCombo->addItem(queryFactures.value(0).toString());
                }
            }
        }
    });
    // QLineEdit pour mensualite
    QLineEdit *mensualiteEdit = new QLineEdit(factureFormWidget);
    QPushButton *factureFormAddBtn = new QPushButton("Ajouter");
    factureFormLayout->addRow("Date paiement:", datePaiementEdit);
    factureFormLayout->addRow("Compteur:", numCompteurCombo);
    factureFormLayout->addRow("Facture:", idFactureCombo);
    factureFormLayout->addRow("Client:", idClientCombo);
    factureFormLayout->addRow("Mensualité:", mensualiteEdit);
    factureFormLayout->addRow(factureFormAddBtn);
    factureLayout->addWidget(factureFormWidget);

    // Mise à jour de l'ancien index selon le compteur sélectionné
    // (Optionnel) Ajout d'une logique de remplissage dynamique si besoin
    // Chargement réel des factures depuis la BDD
    auto chargerListeFactures = [&]() {
        qDebug() << "[mlog] chargerListeFactures()";
        QTableView *factureTable = this->findChild<QTableView *>("factureTable");
        if (!factureTable) {
            qDebug() << "[mlog] ERREUR: factureTable introuvable !";
            return;
        }
        QStandardItemModel *newFactureModel = new QStandardItemModel(0, 6, factureTable);
        newFactureModel->setHorizontalHeaderLabels({"ID", "Date paiement", "Compteur", "Facture", "Client", "Mensualité"});
        QSqlQuery query("SELECT f.id, f.date_paiement, f.numCompteur, f.idFacture, f.idClient, f.mensualite, c.nom, c.prenom FROM Facturation f LEFT JOIN Client c ON f.idClient = c.idClient");
        while (query.next()) {
            QList<QStandardItem *> items;
            for (int i = 0; i < 6; ++i) {
                QStandardItem *item = new QStandardItem(query.value(i).toString());
                // Seule la colonne Mensualité (5) est éditable
                if (i == 5)
                    item->setFlags(item->flags() | Qt::ItemIsEditable);
                else
                    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                items << item;
            }
            // Colonne client : id + prénom + nom
            QString clientDisplay = query.value(4).toString();
            if (!query.value(7).toString().isEmpty() || !query.value(6).toString().isEmpty()) {
                clientDisplay += " - " + query.value(7).toString() + " " + query.value(6).toString();
            }
            items[4]->setText(clientDisplay);
            newFactureModel->appendRow(items);
        }
        factureTable->setModel(newFactureModel);
        factureTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        factureTable->setSelectionMode(QAbstractItemView::SingleSelection);
        factureTable->setAlternatingRowColors(true);
        factureTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    factureTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
        QObject::connect(newFactureModel, &QStandardItemModel::itemChanged, this, [=](QStandardItem *item) {
            int row = item->row();
            int col = item->column();
            qDebug() << "[mlog] itemChanged Facturation : row=" << row << ", col=" << col << ", value=" << item->text();
            if (col == 5) {
                QString idFacturation = newFactureModel->item(row, 0)->text();
                QString nouvelleMensualite = item->text();
                if (nouvelleMensualite.trimmed().isEmpty()) {
                    QMessageBox::warning(this, "Erreur", "La mensualité ne doit pas être vide.");
                    return;
                }
                bool okMensualite = false;
                double valMensualite = nouvelleMensualite.toDouble(&okMensualite);
                if (!okMensualite || valMensualite < 0) {
                    QMessageBox::warning(this, "Erreur", "La mensualité doit être une valeur numérique positive.");
                    // Remettre la valeur précédente depuis la base
                    QSqlQuery query;
                    query.prepare("SELECT mensualite FROM Facturation WHERE id = ?");
                    query.addBindValue(idFacturation);
                    if (query.exec() && query.next()) {
                        item->setText(query.value(0).toString());
                    }
                    return;
                }
                qDebug() << "[mlog] Tentative de mise à jour mensualité en DB : id=" << idFacturation << ", nouvelleMensualite=" << nouvelleMensualite;
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE Facturation SET mensualite = ? WHERE id = ?");
                updateQuery.addBindValue(valMensualite);
                updateQuery.addBindValue(idFacturation);
                if (updateQuery.exec()) {
                    qDebug() << "[mlog] Mise à jour DB OK pour id=" << idFacturation;
                    QMessageBox::information(this, "Modification", "La mensualité a été modifiée dans la base de données.");
                } else {
                    qDebug() << "[mlog] Erreur SQL lors de la mise à jour mensualité :" << updateQuery.lastError().text();
                    QMessageBox::critical(this, "Erreur SQL", "Impossible de modifier la mensualité : " + updateQuery.lastError().text());
                }
            }
        });
        qDebug() << "[mlog] Fin chargerListeFactures()";
    };
    connect(factureFormAddBtn, &QPushButton::clicked, this, [=]()
            {
        qDebug() << "[mlog] Ajout facturation :" << datePaiementEdit->date() << numCompteurCombo->currentText() << idFactureCombo->currentText() << idClientCombo->currentData() << mensualiteEdit->text();
        QString datePaiement = datePaiementEdit->date().toString("yyyy-MM-dd");
        QString numCompteur = numCompteurCombo->currentText();
        QString idFacture = idFactureCombo->currentText();
        QVariant idClient = idClientCombo->currentData();
        QString mensualite = mensualiteEdit->text();
        if (datePaiement.isEmpty() || numCompteur.isEmpty() || idFacture.isEmpty() || !idClient.isValid() || mensualite.isEmpty()) {
            qDebug() << "[mlog] Champs manquants lors de l'ajout facturation";
            QMessageBox::warning(this, "Erreur", "Veuillez remplir tous les champs.");
            return;
        }
        QSqlQuery insertFacture;
        insertFacture.prepare("INSERT INTO Facturation (date_paiement, numCompteur, idFacture, idClient, mensualite) VALUES (?, ?, ?, ?, ?)");
        insertFacture.addBindValue(datePaiement);
        insertFacture.addBindValue(numCompteur);
        insertFacture.addBindValue(idFacture);
        insertFacture.addBindValue(idClient);
        insertFacture.addBindValue(mensualite.toDouble());
        if (!insertFacture.exec()) {
            qDebug() << "[mlog] Erreur SQL lors de l'ajout facturation :" << insertFacture.lastError().text();
            QMessageBox::critical(this, "Erreur SQL", "Impossible d'insérer la facturation : " + insertFacture.lastError().text());
            return;
        }
        // Ajout direct dans le modèle Qt pour affichage instantané
        insererLigneFactureDansTable("", datePaiement, numCompteur, idFacture, idClient.toString(), mensualite);
        qDebug() << "[mlog] Facturation ajoutée (ligne insérée dans le modèle).";
        QMessageBox::information(this, "Ajout", "Facturation ajoutée.");
        mensualiteEdit->clear(); });
    // Connexion du signal textChanged déjà faite dans le constructeur, ne pas répéter ici pour éviter les crashs.
    // Suppression du filtre de statut
    tabWidget->addTab(factureTab, "Facturation");

    // ICI : connexion du signal
    connect(tabWidget, &QTabWidget::currentChanged, this, [=](int index)
    {
        qDebug() << "[mlog] Changement d'onglet : index=" << index;
        if (index == 0) {
            qDebug() << "[mlog] Appel chargerListeCompteurs()";
            try {
                chargerListeCompteurs();
                qDebug() << "[mlog] chargerListeCompteurs() OK";
            } catch (...) {
                qDebug() << "[mlog] Exception dans chargerListeCompteurs()";
            }
        }
        else if (index == 1) {
            qDebug() << "[mlog] (avant appel) chargerListePrelevements()";
            try {
                chargerListePrelevements();
                qDebug() << "[mlog] chargerListePrelevements() OK";
            } catch (...) {
                qDebug() << "[mlog] Exception dans chargerListePrelevements()";
            }
            // Rafraîchir la liste des compteurs transférés dans le formulaire d'ajout de prélèvement
            updatePrelevCompteurCombo();
            if (prelevCompteurCombo->count() > 0)
                emit prelevCompteurCombo->currentTextChanged(prelevCompteurCombo->currentText());
        }
        else if (index == 2) {
            qDebug() << "[mlog] Appel chargerListeFactures()";
            try {
                chargerListeFactures();
                qDebug() << "[mlog] chargerListeFactures() OK";
            } catch (...) {
                qDebug() << "[mlog] Exception dans chargerListeFactures()";
            }
        }
    });

    // Premier chargement
    chargerListeCompteurs();
    chargerListePrelevements();
    chargerListeFactures();

    qDebug() << "[mlog] Fin constructeur CompteurWidget";
    mainLayout->addWidget(tabWidget);
}
