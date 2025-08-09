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

CompteurWidget::CompteurWidget(Controller* controller, QWidget* parent)
    : QWidget(parent) {
    qDebug() << "[mlog] Début constructeur CompteurWidget";
    qDebug() << "[mlog] Adresse this=" << this;
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QTabWidget* tabWidget = new QTabWidget(this);


    // Onglet Compteur avec filtrage par état
    QWidget* compteurTab = new QWidget;
    QVBoxLayout* ajoutLayout = new QVBoxLayout(compteurTab);
    // ... formulaire d'ajout ...
    // Filtre d'état déplacé sous le formulaire
    QHBoxLayout* filterLayout = new QHBoxLayout;
    QLabel* filterLabel = new QLabel("Filtrer par état :");
    QComboBox* filterCombo = new QComboBox;
    filterCombo->addItem("Tous");
    filterCombo->addItem("Disponible");
    filterCombo->addItem("Archivé");
    filterCombo->addItem("Transféré");
    QLineEdit* searchCompteurEdit = new QLineEdit;
    searchCompteurEdit->setPlaceholderText("Rechercher numéro compteur...");
    searchCompteurEdit->setMinimumWidth(160);
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(filterCombo);
    filterLayout->addWidget(searchCompteurEdit);
    ajoutLayout->addLayout(filterLayout);

    // Tableau des compteurs
    QTableView* compteursTable = new QTableView;
    QStandardItemModel* compteursModel = new QStandardItemModel(0, 5, this);
    compteursModel->setHorizontalHeaderLabels({"Numéro", "Attribut", "Ancien index", "Série", "Actions"});
    compteursTable->setModel(compteursModel);
    compteursTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    compteursTable->setAlternatingRowColors(true);
    // Largeur fixe pour la colonne Actions
    compteursTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    compteursTable->setColumnWidth(4, 90); // Largeur adaptée pour 3 icônes
    ajoutLayout->addWidget(compteursTable);

    // Chargement réel des compteurs depuis la BDD
    auto chargerListeCompteurs = [&]() {
        qDebug() << "[mlog] chargerListeCompteurs()";
        qDebug() << "[mlog] Adresse compteursTable=" << compteursTable;
        // Recréer le modèle et le delegate à chaque rechargement
        QStandardItemModel* newModel = new QStandardItemModel(0, 5, compteursTable);
        qDebug() << "[mlog] Adresse newModel=" << newModel;
        newModel->setHorizontalHeaderLabels({"Numéro", "Attribut", "Ancien index", "Série", "Actions"});
        QSqlQuery query("SELECT numCompteur, attributComp, ancienindex, serie FROM Compteur");
        int row = 0;
        while (query.next()) {
            qDebug() << "[mlog] Insertion row=" << row;
            newModel->insertRow(row);
            int modelCols = newModel->columnCount();
            // Colonnes 0 à 3 : données SQL
            for (int col = 0; col < 4 && col < modelCols; ++col) {
                QStandardItem* item = new QStandardItem;
                item->setText(query.value(col).toString());
                // Seule la colonne 3 (Série) est éditable
                if (col == 3) {
                    item->setFlags(item->flags() | Qt::ItemIsEditable);
                } else {
                    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                }
                newModel->setItem(row, col, item);
                qDebug() << "[mlog] setItem row=" << row << "col=" << col << ":" << item->text();
            }
            // Colonne 4 : Actions (lecture seule, vide, sans Assigner)
            if (modelCols > 4) {
                QStandardItem* actionItem = new QStandardItem;
                actionItem->setFlags(actionItem->flags() & ~Qt::ItemIsEditable);
                newModel->setItem(row, 4, actionItem);
                qDebug() << "[mlog] setItem row=" << row << "col=4 (Actions)";
            }
            row++;
        }
        // Libérer l'ancien modèle pour éviter la fuite/crash
        QAbstractItemModel* oldModel = compteursTable->model();
        qDebug() << "[mlog] Avant setModel, oldModel=" << oldModel << ", newModel=" << newModel;
        compteursTable->setModel(newModel);
        qDebug() << "[mlog] Après setModel";
        if (oldModel && oldModel != newModel) {
            qDebug() << "[mlog] Suppression oldModel";
            delete oldModel;
        }
        compteursTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        compteursTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
        compteursTable->setColumnWidth(4, 120);
        MyActionsDelegate* actionsDelegate = new MyActionsDelegate(compteursTable);
        qDebug() << "[mlog] Delegate créé :" << actionsDelegate;
        compteursTable->setItemDelegateForColumn(4, actionsDelegate);
        qDebug() << "[mlog] Delegate affecté à la colonne 4";
        // Reconnecter les signaux
        QObject::connect(actionsDelegate, &MyActionsDelegate::emitArchiver, this, [=](const QModelIndex& index){
            qDebug() << "[mlog] Signal emitArchiver reçu pour row=" << index.row();
            QString numCompteur = newModel->item(index.row(),0)->text();
            int ret = QMessageBox::question(this, "Archiver", "Voulez-vous vraiment archiver le compteur : " + numCompteur + " ?", QMessageBox::Yes | QMessageBox::No);
            if (ret == QMessageBox::Yes) {
                // Mise à jour SQL
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE Compteur SET attributComp = 'Archivé' WHERE numCompteur = ?");
                updateQuery.addBindValue(numCompteur);
                if (!updateQuery.exec()) {
                    qDebug() << "[mlog] Erreur SQL lors de l'archivage :" << updateQuery.lastError().text();
                    QMessageBox::critical(this, "Erreur SQL", "Impossible d'archiver le compteur : " + updateQuery.lastError().text());
                    return;
                }
                // Mise à jour dans le modèle Qt
                newModel->setItem(index.row(), 1, new QStandardItem("Archivé"));
                qDebug() << "[mlog] Compteur archivé et modèle mis à jour.";
                QMessageBox::information(this, "Archivé", "Le compteur a été archivé.");
            }
        });
        // Connexion du signal itemChanged sur le modèle affiché (newModel)
        QObject::connect(newModel, &QStandardItemModel::itemChanged, this, [=](QStandardItem* item) {
            int row = item->row();
            int col = item->column();
            qDebug() << "[mlog] itemChanged déclenché : row=" << row << ", col=" << col << ", value=" << item->text();
            // Seule la colonne Série (colonne 3) est modifiable
            if (col == 3) {
                QString numCompteur = newModel->item(row, 0)->text();
                QString nouvelleSerie = item->text();
                qDebug() << "[mlog] Tentative de mise à jour série en DB : numCompteur=" << numCompteur << ", nouvelleSerie=" << nouvelleSerie;
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE Compteur SET serie = ? WHERE numCompteur = ?");
                updateQuery.addBindValue(nouvelleSerie);
                updateQuery.addBindValue(numCompteur);
                if (updateQuery.exec()) {
                    qDebug() << "[mlog] Mise à jour DB OK pour numCompteur=" << numCompteur;
                    QMessageBox::information(this, "Modification", "La série a été modifiée dans la base de données.");
                } else {
                    qDebug() << "[mlog] Erreur SQL lors de la mise à jour série :" << updateQuery.lastError().text();
                    QMessageBox::critical(this, "Erreur SQL", "Impossible de modifier la série : " + updateQuery.lastError().text());
                }
            }
        });
        QObject::connect(actionsDelegate, &MyActionsDelegate::emitTransferer, this, [=](const QModelIndex& index){
            qDebug() << "[mlog] Signal emitTransferer reçu pour row=" << index.row();
            QString numCompteur = newModel->item(index.row(),0)->text();
            // Création du formulaire de sélection de client
            QDialog dialog(this);
            dialog.setWindowTitle("Transférer compteur");
            QVBoxLayout* layout = new QVBoxLayout(&dialog);
            QLabel* label = new QLabel("Sélectionnez le client destinataire :", &dialog);
            QComboBox* clientCombo = new QComboBox(&dialog);
            QSqlQuery queryClients("SELECT idClient, nom, prenom FROM Client");
            while (queryClients.next()) {
                QString labelText = queryClients.value(0).toString() + " - " + queryClients.value(1).toString() + " " + queryClients.value(2).toString();
                clientCombo->addItem(labelText, queryClients.value(0));
            }
            layout->addWidget(label);
            layout->addWidget(clientCombo);
            QPushButton* okBtn = new QPushButton("OK", &dialog);
            layout->addWidget(okBtn);
            QObject::connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
            if (dialog.exec() == QDialog::Accepted && clientCombo->currentIndex() >= 0) {
                QVariant idClient = clientCombo->currentData();
                // Vérifier si l'abonnement existe déjà
                QSqlQuery checkQuery;
                checkQuery.prepare("SELECT id FROM Abonnement WHERE numCompteur = ? AND idClient = ?");
                checkQuery.addBindValue(numCompteur);
                checkQuery.addBindValue(idClient);
                if (checkQuery.exec() && checkQuery.next()) {
                    // Abonnement déjà existant
                    QMessageBox::information(this, "Transfert", "Ce compteur est déjà relié à ce client (abonnement existant).");
                } else {
                    // Créer un nouvel abonnement
                    QSqlQuery insertQuery;
                    insertQuery.prepare("INSERT INTO Abonnement (idClient, numCompteur, date_abonnement) VALUES (?, ?, ?)");
                    insertQuery.addBindValue(idClient);
                    insertQuery.addBindValue(numCompteur);
                    insertQuery.addBindValue(QDate::currentDate());
                    if (!insertQuery.exec()) {
                        qDebug() << "[mlog] Erreur SQL lors de la création de l'abonnement :" << insertQuery.lastError().text();
                        QMessageBox::critical(this, "Erreur SQL", "Impossible de créer l'abonnement : " + insertQuery.lastError().text());
                        return;
                    }
                    QMessageBox::information(this, "Transfert", "Abonnement créé entre le compteur et le client.");
                }
                // Mise à jour attributComp
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE Compteur SET attributComp = 'Transféré' WHERE numCompteur = ?");
                updateQuery.addBindValue(numCompteur);
                if (!updateQuery.exec()) {
                    qDebug() << "[mlog] Erreur SQL lors du transfert :" << updateQuery.lastError().text();
                    QMessageBox::critical(this, "Erreur SQL", "Impossible de transférer le compteur : " + updateQuery.lastError().text());
                    return;
                }
                newModel->setItem(index.row(), 1, new QStandardItem("Transféré"));
                qDebug() << "[mlog] Compteur transféré et modèle mis à jour.";
            }
        });
        // Filtrage dynamique sur le modèle actif (compteurs)
        QObject::connect(searchCompteurEdit, &QLineEdit::textChanged, this, [=]() {
            QString text = searchCompteurEdit->text();
            QString etat = filterCombo->currentText();
            for (int row = 0; row < newModel->rowCount(); ++row) {
                bool matchNum = newModel->item(row, 0)->text().contains(text, Qt::CaseInsensitive);
                bool matchEtat = (etat == "Tous") || (newModel->item(row, 1)->text() == etat);
                compteursTable->setRowHidden(row, !(matchNum && matchEtat));
            }
        });
        QObject::connect(filterCombo, &QComboBox::currentTextChanged, this, [=]() {
            QString text = searchCompteurEdit->text();
            QString etat = filterCombo->currentText();
            for (int row = 0; row < newModel->rowCount(); ++row) {
                bool matchNum = newModel->item(row, 0)->text().contains(text, Qt::CaseInsensitive);
                bool matchEtat = (etat == "Tous") || (newModel->item(row, 1)->text() == etat);
                compteursTable->setRowHidden(row, !(matchNum && matchEtat));
            }
        });
       
        qDebug() << "[mlog] Fin chargerListeCompteurs()";
    };
    chargerListeCompteurs();
    // Formulaire d'ajout de compteur
    QWidget* formWidget = new QWidget;
    QFormLayout* formLayout = new QFormLayout(formWidget);
    QLineEdit* numCompteurEdit = new QLineEdit;
    QComboBox* attributCompCombo = new QComboBox;
    attributCompCombo->addItems({"Disponible", "Archivé", "Transféré"});
    // Ancien index retiré du formulaire, valeur par défaut 0
    QLineEdit* serieEdit = new QLineEdit;
    formLayout->addRow("Numéro de compteur:", numCompteurEdit);
    formLayout->addRow("Attribut:", attributCompCombo);
    // Ancien index non affiché, sera 0 par défaut
    formLayout->addRow("Série:", serieEdit);
    QPushButton* addBtn = new QPushButton("Ajouter");
    formLayout->addRow(addBtn);
    ajoutLayout->addWidget(formWidget);
    
    // Méthode pour insérer une ligne dans le modèle de la table compteurs
    auto insererLigneCompteurDansTable = [compteursTable](const QString& numCompteur, const QString& attributComp, double ancienIndex, const QString& serie) {
        qDebug() << "[mlog] insererLigneCompteurDansTable appelé";
        qDebug() << "[mlog] Paramètres reçus : numCompteur=" << numCompteur << ", attributComp=" << attributComp << ", ancienIndex=" << ancienIndex << ", serie=" << serie;
        if (!compteursTable) {
            qDebug() << "[mlog] ERREUR: compteursTable est nul ! Impossible d'accéder au modèle.";
            return;
        }
        QAbstractItemModel* rawModel = compteursTable->model();
        qDebug() << "[mlog] compteursTable->model() =" << rawModel << ", type =" << (rawModel ? rawModel->metaObject()->className() : "nullptr");
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(rawModel);
 if (!model) {
     qDebug() << "[mlog] ERREUR: Le modèle n'est pas un QStandardItemModel ou est nul ! On recrée un modèle vierge pour éviter le crash.";
     model = new QStandardItemModel(0, 5, compteursTable);
     model->setHorizontalHeaderLabels({"Numéro", "Attribut", "Ancien index", "Série", "Actions"});
     compteursTable->setModel(model);
 }
        if (!model) {
            qDebug() << "[mlog] ERREUR: Le modèle n'est pas un QStandardItemModel ou est nul ! On recrée un modèle vierge pour éviter le crash.";
            // Correction immédiate : on recrée un modèle vide si le modèle est corrompu
            model = new QStandardItemModel(0, 5, compteursTable);
            model->setHorizontalHeaderLabels({"Numéro", "Attribut", "Ancien index", "Série", "Actions"});
            compteursTable->setModel(model);
        }
        // Vérifier si le compteur existe déjà dans le modèle
        for (int i = 0; i < model->rowCount(); ++i) {
            if (model->item(i, 0) && model->item(i, 0)->text() == numCompteur) {
                qDebug() << "[mlog] Doublon détecté dans le modèle, insertion ignorée.";
                return;
            }
        }
        int row = model->rowCount();
        qDebug() << "[mlog] Insertion nouvelle ligne row=" << row << ", numCompteur=" << numCompteur;
        model->insertRow(row);
        {
            QStandardItem* item0 = new QStandardItem(numCompteur);
            item0->setFlags(item0->flags() & ~Qt::ItemIsEditable);
            model->setItem(row, 0, item0);
            QStandardItem* item1 = new QStandardItem(attributComp);
            item1->setFlags(item1->flags() & ~Qt::ItemIsEditable);
            model->setItem(row, 1, item1);
            QStandardItem* item2 = new QStandardItem(QString::number(ancienIndex));
            item2->setFlags(item2->flags() | Qt::ItemIsEditable);
            model->setItem(row, 2, item2);
            QStandardItem* item3 = new QStandardItem(serie);
            item3->setFlags(item3->flags() | Qt::ItemIsEditable);
            model->setItem(row, 3, item3);
            QStandardItem* actionItem = new QStandardItem;
            actionItem->setFlags(actionItem->flags() & ~Qt::ItemIsEditable);
            model->setItem(row, 4, actionItem);
        }
        qDebug() << "[mlog] Ligne insérée dans le modèle :" << numCompteur << attributComp << ancienIndex << serie;
    };

    connect(addBtn, &QPushButton::clicked, this, [=]() {
        // --- Début du slot d'ajout de compteur ---
        qDebug() << "[mlog] Ajout compteur :" << numCompteurEdit->text() << attributCompCombo->currentText() << serieEdit->text();
        qDebug() << "[mlog] Adresse compteursTable (addBtn)=" << compteursTable;
        QString numCompteur = numCompteurEdit->text();
        QString attributComp = attributCompCombo->currentText();
        QString serie = serieEdit->text();
        double ancienIndex = 0; // valeur par défaut

        // Vérification des champs du formulaire
        if (numCompteur.isEmpty() || attributComp.isEmpty() || serie.isEmpty()) {
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
        if (!insertCompteur.exec()) {
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
    QWidget* prelevTab = new QWidget;
    QVBoxLayout* prelevLayout = new QVBoxLayout(prelevTab);
    QHBoxLayout* prelevTopLayout = new QHBoxLayout;
    QLineEdit* prelevFilterEdit = new QLineEdit;
    prelevFilterEdit->setPlaceholderText("Filtrer par compteur...");
    prelevTopLayout->addWidget(prelevFilterEdit);
    prelevLayout->addLayout(prelevTopLayout);
    QTableView* prelevTable = new QTableView;
    QStandardItemModel* prelevModel = new QStandardItemModel(0, 5, this);
    prelevModel->setHorizontalHeaderLabels({"ID", "Date", "Compteur", "Nouvel Index", "Ancien Index"});
    prelevTable->setModel(prelevModel);
    prelevTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    prelevTable->setSelectionMode(QAbstractItemView::SingleSelection);
    prelevTable->setAlternatingRowColors(true);
    prelevTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    prelevTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    prelevLayout->addWidget(prelevTable);

    // Chargement réel des prélèvements depuis la BDD
    auto chargerListePrelevements = [&]() {
        qDebug() << "[mlog] chargerListePrelevements()";
        QStandardItemModel* newPrelevModel = new QStandardItemModel(0, 5, prelevTable);
        newPrelevModel->setHorizontalHeaderLabels({"ID", "Date", "Compteur", "Nouvel Index", "Ancien Index"});
        QSqlQuery query("SELECT idpreleve, dateprelevement, numCompteur, newIndex, ancienIndex FROM Prelevement");
        while (query.next()) {
            QList<QStandardItem*> items;
            for (int i = 0; i < 5; ++i) {
                QStandardItem* item = new QStandardItem(query.value(i).toString());
                // Seules les colonnes Nouvel Index (3) et Ancien Index (4) sont éditables
                if (i == 3 || i == 4) {
                    item->setFlags(item->flags() | Qt::ItemIsEditable);
                } else {
                    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                }
                items << item;
            }
            newPrelevModel->appendRow(items);
        }
        prelevTable->setModel(newPrelevModel);
        // Filtrage dynamique sur le modèle actif
        QObject::connect(prelevFilterEdit, &QLineEdit::textChanged, this, [=]() {
            QString text = prelevFilterEdit->text();
            for (int row = 0; row < newPrelevModel->rowCount(); ++row) {
                bool match = newPrelevModel->item(row, 2)->text().contains(text, Qt::CaseInsensitive);
                prelevTable->setRowHidden(row, !match && !text.isEmpty());
            }
        });
        prelevTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        prelevTable->setSelectionMode(QAbstractItemView::SingleSelection);
        prelevTable->setAlternatingRowColors(true);
        prelevTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        // Autoriser l'édition par double-clic et touche Entrée
        prelevTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
        // Connexion du signal itemChanged sur le modèle affiché (newPrelevModel)
        QObject::connect(newPrelevModel, &QStandardItemModel::itemChanged, this, [=](QStandardItem* item) {
            int row = item->row();
            int col = item->column();
            qDebug() << "[mlog] itemChanged Prélèvements : row=" << row << ", col=" << col << ", value=" << item->text();
            // Seules les colonnes Nouvel Index (3) et Ancien Index (4) sont modifiables
            if (col == 3 || col == 4) {
                QString idPreleve = newPrelevModel->item(row, 0)->text();
                QString nouvelIndex = newPrelevModel->item(row, 3)->text();
                QString ancienIndex = newPrelevModel->item(row, 4)->text();
                bool okAncien = false, okNouvel = false;
                double valAncien = ancienIndex.toDouble(&okAncien);
                double valNouvel = nouvelIndex.toDouble(&okNouvel);
                if (!okAncien || !okNouvel) {
                    QMessageBox::warning(this, "Erreur", "Les index doivent être des valeurs numériques.");
                    return;
                }
                if (valAncien >= valNouvel) {
                    QMessageBox::warning(this, "Erreur", "Le nouvel index doit être strictement supérieur à l'ancien index.");
                    // Annuler la modification dans le modèle (remettre l'ancienne valeur)
                    QSqlQuery query;
                    query.prepare("SELECT newIndex, ancienIndex FROM Prelevement WHERE idpreleve = ?");
                    query.addBindValue(idPreleve);
                    if (query.exec() && query.next()) {
                        newPrelevModel->item(row, 3)->setText(query.value(0).toString());
                        newPrelevModel->item(row, 4)->setText(query.value(1).toString());
                    }
                    return;
                }
                qDebug() << "[mlog] Mise à jour en DB : idPreleve=" << idPreleve << ", nouvelIndex=" << nouvelIndex << ", ancienIndex=" << ancienIndex;
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE Prelevement SET newIndex = ?, ancienIndex = ? WHERE idpreleve = ?");
                updateQuery.addBindValue(nouvelIndex);
                updateQuery.addBindValue(ancienIndex);
                updateQuery.addBindValue(idPreleve);
                if (updateQuery.exec()) {
                    qDebug() << "[mlog] Mise à jour DB OK pour idpreleve=" << idPreleve;
                    QMessageBox::information(this, "Modification", "Les index ont été modifiés dans la base de données.");
                } else {
                    qDebug() << "[mlog] Erreur SQL lors de la mise à jour index :" << updateQuery.lastError().text();
                    QMessageBox::critical(this, "Erreur SQL", "Impossible de modifier les index : " + updateQuery.lastError().text());
                }
            }
        });
        qDebug() << "[mlog] Fin chargerListePrelevements()";
    };
    chargerListePrelevements();

    // Formulaire d'ajout sous le tableau Prélèvements
    // Méthode pour insérer une ligne dans le modèle de la table prélèvements
    auto insererLignePrelevementDansTable = [prelevTable](const QString& id, const QString& date, const QString& compteur, const QString& nouvelIndex, const QString& ancienIndex) {
        qDebug() << "[mlog] insererLignePrelevementDansTable appelé";
        if (!prelevTable) {
            qDebug() << "[mlog] ERREUR: prelevTable est nul ! Impossible d'accéder au modèle.";
            return;
        }
        QAbstractItemModel* rawModel = prelevTable->model();
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(rawModel);
        if (!model) {
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
    QWidget* prelevFormWidget = new QWidget;
    QFormLayout* prelevFormLayout = new QFormLayout(prelevFormWidget);
    // QDateEdit pour la date
    QDateEdit* prelevDateEdit = new QDateEdit(QDate::currentDate(), prelevFormWidget);
    prelevDateEdit->setCalendarPopup(true);
    // QComboBox pour compteur transféré
    QComboBox* prelevCompteurCombo = new QComboBox(prelevFormWidget);
    QSqlQuery queryCompteursPrelev("SELECT numCompteur FROM Compteur WHERE attributComp = 'Transféré'");
    while (queryCompteursPrelev.next()) {
        prelevCompteurCombo->addItem(queryCompteursPrelev.value(0).toString());
    }
    // Ancien index (lecture seule)
    QLineEdit* prelevAncienIndexEdit = new QLineEdit(prelevFormWidget);
    prelevAncienIndexEdit->setReadOnly(true);
    // Nouvel index
    QLineEdit* prelevNouvelIndexEdit = new QLineEdit(prelevFormWidget);
    QPushButton* prelevFormAddBtn = new QPushButton("Ajouter");
    prelevFormLayout->addRow("Date:", prelevDateEdit);
    prelevFormLayout->addRow("Compteur:", prelevCompteurCombo);
    prelevFormLayout->addRow("Ancien index:", prelevAncienIndexEdit);
    prelevFormLayout->addRow("Nouvel index:", prelevNouvelIndexEdit);
    prelevFormLayout->addRow(prelevFormAddBtn);
    prelevLayout->addWidget(prelevFormWidget);

    // Mise à jour de l'ancien index selon le compteur sélectionné
    QObject::connect(prelevCompteurCombo, &QComboBox::currentTextChanged, prelevFormWidget, [=](const QString& numCompteur){
        QSqlQuery queryIndex;
        queryIndex.prepare("SELECT ancienindex FROM Compteur WHERE numCompteur = ?");
        queryIndex.addBindValue(numCompteur);
        if (queryIndex.exec() && queryIndex.next()) {
            double ancienIndex = queryIndex.value(0).toDouble();
            prelevAncienIndexEdit->setText(QString::number(ancienIndex));
        } else {
            prelevAncienIndexEdit->setText("0");
        }
    });
    // Initialiser l'ancien index au démarrage
    if (prelevCompteurCombo->count() > 0)
        emit prelevCompteurCombo->currentTextChanged(prelevCompteurCombo->currentText());
    
    connect(prelevFormAddBtn, &QPushButton::clicked, this, [=]() {
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
        prelevNouvelIndexEdit->clear();
    });
    connect(prelevFilterEdit, &QLineEdit::textChanged, this, [=](const QString& text) {
        for (int row = 0; row < prelevModel->rowCount(); ++row) {
            bool match = prelevModel->item(row, 2)->text().contains(text, Qt::CaseInsensitive);
            prelevTable->setRowHidden(row, !match && !text.isEmpty());
        }
    });
    tabWidget->addTab(prelevTab, "Prélèvements");











    // Onglet Facturation
    QWidget* factureTab = new QWidget;
    QVBoxLayout* factureLayout = new QVBoxLayout(factureTab);
    QHBoxLayout* factureTopLayout = new QHBoxLayout;
    QLineEdit* factureFilterEdit = new QLineEdit;
    factureFilterEdit->setPlaceholderText("Filtrer par client...");
    QComboBox* factureStatusCombo = new QComboBox;
    factureStatusCombo->addItem("Tous");
    factureStatusCombo->addItem("Payée");
    factureStatusCombo->addItem("En attente");
    factureTopLayout->addWidget(factureFilterEdit);
    factureTopLayout->addWidget(new QLabel("Statut : "));
    factureTopLayout->addWidget(factureStatusCombo);
    factureLayout->addLayout(factureTopLayout);
    QTableView* factureTable = new QTableView;
    QStandardItemModel* factureModel = new QStandardItemModel(0, 6, this);
    factureModel->setHorizontalHeaderLabels({"ID", "Date paiement", "Compteur", "Facture", "Client", "Mensualité"});
    factureTable->setModel(factureModel);
    factureTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    factureTable->setSelectionMode(QAbstractItemView::SingleSelection);
    factureTable->setAlternatingRowColors(true);
    factureTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    factureTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    factureLayout->addWidget(factureTable);
    // Formulaire d'ajout sous le tableau Facturation
    // Méthode pour insérer une ligne dans le modèle de la table factures
    auto insererLigneFactureDansTable = [factureTable](const QString& id, const QString& datePaiement, const QString& numCompteur, const QString& idFacture, const QString& idClient, const QString& mensualite) {
        qDebug() << "[mlog] insererLigneFactureDansTable appelé";
        if (!factureTable) {
            qDebug() << "[mlog] ERREUR: factureTable est nul ! Impossible d'accéder au modèle.";
            return;
        }
        QAbstractItemModel* rawModel = factureTable->model();
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(rawModel);
        if (!model) {
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
    QWidget* factureFormWidget = new QWidget;
    QFormLayout* factureFormLayout = new QFormLayout(factureFormWidget);
    // QDateEdit pour date_paiement
    QDateEdit* datePaiementEdit = new QDateEdit(QDate::currentDate(), factureFormWidget);
    datePaiementEdit->setCalendarPopup(true);
    // QComboBox pour numCompteur transféré
    QComboBox* numCompteurCombo = new QComboBox(factureFormWidget);
    QSqlQuery queryCompteurs("SELECT numCompteur FROM Compteur WHERE attributComp = 'Transféré'");
    while (queryCompteurs.next()) {
        numCompteurCombo->addItem(queryCompteurs.value(0).toString());
    }
    // QComboBox pour idFacture
    QComboBox* idFactureCombo = new QComboBox(factureFormWidget);
    QSqlQuery queryFactures("SELECT idFacture FROM Facture");
    while (queryFactures.next()) {
        idFactureCombo->addItem(queryFactures.value(0).toString());
    }
    // QComboBox pour idClient
    QComboBox* idClientCombo = new QComboBox(factureFormWidget);
    QSqlQuery queryClients("SELECT idClient, nom, prenom FROM Client");
    while (queryClients.next()) {
        QString label = queryClients.value(0).toString() + " - " + queryClients.value(1).toString() + " " + queryClients.value(2).toString();
        idClientCombo->addItem(label, queryClients.value(0));
    }
    // QLineEdit pour mensualite
    QLineEdit* mensualiteEdit = new QLineEdit(factureFormWidget);
    QPushButton* factureFormAddBtn = new QPushButton("Ajouter");
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
        QStandardItemModel* newFactureModel = new QStandardItemModel(0, 6, factureTable);
        newFactureModel->setHorizontalHeaderLabels({"ID", "Date paiement", "Compteur", "Facture", "Client", "Mensualité"});
        QSqlQuery query("SELECT id, date_paiement, numCompteur, idFacture, idClient, mensualite FROM Facturation");
        while (query.next()) {
            QList<QStandardItem*> items;
            for (int i = 0; i < 6; ++i) {
                QStandardItem* item = new QStandardItem(query.value(i).toString());
                // Seule la colonne Mensualité (5) est éditable
                if (i == 5) {
                    item->setFlags(item->flags() | Qt::ItemIsEditable);
                } else {
                    item->setFlags(item->flags() & ~Qt::ItemIsEditable);
                }
                items << item;
            }
            newFactureModel->appendRow(items);
        }
        factureTable->setModel(newFactureModel);
        // Filtrage dynamique sur le modèle actif
        QObject::connect(factureFilterEdit, &QLineEdit::textChanged, this, [=]() {
            QString text = factureFilterEdit->text();
            QString status = factureStatusCombo->currentText();
            for (int row = 0; row < newFactureModel->rowCount(); ++row) {
                bool matchNum = newFactureModel->item(row, 2)->text().contains(text, Qt::CaseInsensitive);
                bool matchStatus = (status == "Tous") || (newFactureModel->item(row, 4)->text() == status);
                factureTable->setRowHidden(row, !(matchNum && matchStatus));
            }
        });
        QObject::connect(factureStatusCombo, &QComboBox::currentTextChanged, this, [=]() {
            QString text = factureFilterEdit->text();
            QString status = factureStatusCombo->currentText();
            for (int row = 0; row < newFactureModel->rowCount(); ++row) {
                bool matchNum = newFactureModel->item(row, 2)->text().contains(text, Qt::CaseInsensitive);
                bool matchStatus = (status == "Tous") || (newFactureModel->item(row, 4)->text() == status);
                factureTable->setRowHidden(row, !(matchNum && matchStatus));
            }
        });
        factureTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        factureTable->setSelectionMode(QAbstractItemView::SingleSelection);
        factureTable->setAlternatingRowColors(true);
        factureTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        factureTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
        // Connexion du signal itemChanged sur le modèle affiché (newFactureModel)
        QObject::connect(newFactureModel, &QStandardItemModel::itemChanged, this, [=](QStandardItem* item) {
            int row = item->row();
            int col = item->column();
            qDebug() << "[mlog] itemChanged Facturation : row=" << row << ", col=" << col << ", value=" << item->text();
            // Seule la colonne Mensualité (5) est modifiable
            if (col == 5) {
                QString idFacturation = newFactureModel->item(row, 0)->text();
                QString nouvelleMensualite = item->text();
                qDebug() << "[mlog] Tentative de mise à jour mensualité en DB : id=" << idFacturation << ", nouvelleMensualite=" << nouvelleMensualite;
                QSqlQuery updateQuery;
                updateQuery.prepare("UPDATE Facturation SET mensualite = ? WHERE id = ?");
                updateQuery.addBindValue(nouvelleMensualite);
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
    chargerListeFactures();
    connect(factureFormAddBtn, &QPushButton::clicked, this, [=]() {
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
        mensualiteEdit->clear();
    });
    connect(factureFilterEdit, &QLineEdit::textChanged, this, [=](const QString& text) {
        for (int row = 0; row < factureModel->rowCount(); ++row) {
            bool match = factureModel->item(row, 2)->text().contains(text, Qt::CaseInsensitive);
            factureTable->setRowHidden(row, !match && !text.isEmpty());
        }
    });
    connect(factureStatusCombo, &QComboBox::currentTextChanged, this, [=](const QString& status) {
        for (int row = 0; row < factureModel->rowCount(); ++row) {
            bool match = (status == "Tous") || (factureModel->item(row, 4)->text() == status);
            factureTable->setRowHidden(row, !match);
        }
    });
    tabWidget->addTab(factureTab, "Facturation");

qDebug() << "[mlog] Fin constructeur CompteurWidget";
mainLayout->addWidget(tabWidget);

}
