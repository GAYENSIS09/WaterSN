#include "widgets/clientdetailwidget.h"
#include "widgets/clientformdialog.h"
#include <QMessageBox>
#include <QKeyEvent>
#include <QResizeEvent>
#include "widgets/clientswidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>

#include <QTableView>
#include <QPushButton>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QHeaderView>

// SQL
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "utils/databasemanager.h"

// Implémentation style dashboard : QtCharts utilisé uniquement ici
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>

#include <QRandomGenerator>


ClientsWidget::ClientsWidget(QWidget* parent) : QWidget(parent) {
    // S'assurer que la connexion à la base de données est établie
    DatabaseManager::connect();
    
    // Barre de recherche/filtrage (même design que dashboard)
    QWidget* filterWidget = new QWidget;
    filterWidget->setStyleSheet("background: #31463f; border-radius: 12px; padding: 12px 18px;");
    QHBoxLayout* filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setSpacing(18);
    searchLineEdit = new QLineEdit;
    searchLineEdit->setPlaceholderText("Nom abonné...");
    searchLineEdit->setMinimumWidth(160);
    searchLineEdit->setStyleSheet("background: #22332d; border: none; border-radius: 6px; padding: 6px 10px; font-size: 14px; color: #e6e6e6;");
    filterLayout->addWidget(searchLineEdit);
    consommationCombo = new QComboBox;
    consommationCombo->addItem("Toutes consommations");
    consommationCombo->addItem("0-10 m³");
    consommationCombo->addItem("10-50 m³");
    consommationCombo->addItem(">50 m³");
    QString comboStyle = "QComboBox { background: #22332d; color: #e6e6e6; border-radius: 6px; padding: 4px 12px; font-size: 14px; min-width: 80px; } QComboBox QAbstractItemView { background: #31463f; color: #e6e6e6; border-radius: 6px; }";
    consommationCombo->setStyleSheet(comboStyle);
    filterLayout->addWidget(consommationCombo);

    // Graphe d'évolution des clients (style dashboard)
    // --- Graphe évolution clients (données réelles) ---
    QLineSeries* series = new QLineSeries();
    series->setName("Clients");
    series->setColor(QColor("#42a5f5"));
    series->setPointsVisible(true);
    QStringList moisLabels = {"Janv", "Fév", "Mars", "Avr", "Mai", "Juin", "Juil", "Août", "Sept", "Oct", "Nov", "Déc"};
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Évolution des clients par mois");
    QFont chartTitleFont;
    chartTitleFont.setPointSize(18);
    chartTitleFont.setBold(true);
    chart->setTitleFont(chartTitleFont);
    chart->setTitleBrush(QBrush(QColor("#b7e0c0")));
    chart->setBackgroundBrush(QBrush(QColor("#31463f")));
    chart->setBackgroundVisible(true);
    chart->setPlotAreaBackgroundBrush(QBrush(QColor("#31463f")));
    chart->setPlotAreaBackgroundVisible(true);
    chart->legend()->setLabelColor(QColor("#e6e6e6"));
    chart->legend()->setFont(QFont("Segoe UI", 12, QFont::Bold));
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setAlignment(Qt::AlignBottom);
    QCategoryAxis* axisX = new QCategoryAxis;
    axisX->setTitleText("Mois");
    axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    axisX->setLabelsAngle(-45);
    QFont axisFont("Segoe UI", 13, QFont::Bold);
    axisX->setLabelsFont(axisFont);
    axisX->setLabelsColor(QColor("#e6e6e6"));
    axisX->setTitleFont(QFont("Segoe UI", 14, QFont::Bold));
    axisX->setTitleBrush(QBrush(QColor("#b7e0c0")));
    for (int i = 0; i < moisLabels.size(); ++i)
        axisX->append(moisLabels[i], i+1);
    axisX->setRange(1, 12);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    QValueAxis* axisY = new QValueAxis;
    axisY->setTitleText("Nombre de clients");
    axisY->setLabelFormat("%.0f");
    axisY->setRange(0, 50);
    axisY->setLabelsFont(axisFont);
    axisY->setLabelsColor(QColor("#e6e6e6"));
    axisY->setTitleFont(QFont("Segoe UI", 14, QFont::Bold));
    axisY->setTitleBrush(QBrush(QColor("#b7e0c0")));
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    QChartView* chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(420);
    chartView->setStyleSheet("background: #31463f; border-radius: 16px; padding: 8px; border: 1px solid #22332d;");

    // Fonction de mise à jour du graphe avec les vraies données
    auto updateClientsEvolutionChart = [series]() {
        series->clear();
        int currentYear = QDate::currentDate().year();
        for (int m = 1; m <= 12; ++m) {
            QSqlQuery q;
            q.prepare("SELECT COUNT(*) FROM Client WHERE MONTH(date_inscription) = ? AND YEAR(date_inscription) = ?");
            q.addBindValue(m);
            q.addBindValue(currentYear);
            int nb = 0;
            if (q.exec() && q.next()) nb = q.value(0).toInt();
            series->append(m, nb);
        }
    };
    updateClientsEvolutionChart();

    // Table des abonnés (QTableView + QStandardItemModel)
    clientsTable = new QTableView;
    clientsTable->setMinimumHeight(340); // Hauteur du conteneur de la table
    clientsTable->setStyleSheet("background: #22332d; color: #e6e6e6; border-radius: 12px; font-size: 17px; border: none; margin-top: 18px; padding: 8px 16px;");
    clientsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    clientsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    clientsTable->setShowGrid(false);
    clientsTable->setAlternatingRowColors(true);
    clientsTable->verticalHeader()->setDefaultSectionSize(38);

    // Modèle de données basé sur la base de données
    QStandardItemModel* model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"ID", "Nom", "Prénom", "Adresse", "Téléphone"});
    // Charger les clients depuis la base de données
    loadClientsFromDatabase(model);
    clientsTable->setModel(model);
    clientsTable->horizontalHeader()->setVisible(true);
    clientsTable->horizontalHeader()->setStretchLastSection(true);
    clientsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // --- Filtrage combiné nom + consommation moyenne (calcul dynamique) ---
    auto applyFilters = [this]() {
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(clientsTable->model());
        if (!model) return;
        QString text = searchLineEdit->text().trimmed().toLower();
        QString choix = consommationCombo->currentText();
        for (int row = 0; row < model->rowCount(); ++row) {
            bool matchNom = model->item(row, 1)->text().toLower().contains(text);
            // Calcul dynamique de la moyenne
            QString idClient = model->item(row, 0)->text();
            double moyenne = 0; int n = 0;
            QSqlQuery q("SELECT (newIndex - ancienIndex) FROM Prelevement WHERE idClient = ?");
            q.addBindValue(idClient);
            while (q.exec() && q.next()) {
                double c = q.value(0).toDouble();
                if (c > 0) { moyenne += c; n++; }
            }
            if (n > 0) moyenne /= n;
            else moyenne = 0;
            bool matchConso = true;
            if (choix == "0-10 m³") matchConso = (moyenne > 0 && moyenne <= 10);
            else if (choix == "10-50 m³") matchConso = (moyenne > 10 && moyenne <= 50);
            else if (choix == ">50 m³") matchConso = (moyenne > 50);
            // "Toutes consommations" => matchConso = true
            bool visible = (matchNom || text.isEmpty()) && matchConso;
            clientsTable->setRowHidden(row, !visible);
        }
    };
    connect(searchLineEdit, &QLineEdit::textChanged, this, [applyFilters](const QString&) { applyFilters(); });
    connect(consommationCombo, &QComboBox::currentTextChanged, this, [applyFilters](const QString&) { applyFilters(); });
    // Ouvrir les détails du client sur double-clic ou touche Entrée
    connect(clientsTable, &QTableView::doubleClicked, this, [this](const QModelIndex& index){
        int row = index.row();
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(clientsTable->model());
        if (row >= 0 && row < model->rowCount() && model) {
            QString nom = model->item(row, 1)->text();
            QString prenom = model->item(row, 2)->text();
            QString adresse = model->item(row, 3)->text();
            QString telephone = model->item(row, 4)->text();
            showClientDetails(nom, prenom, adresse, telephone);
        }
    });
    clientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    clientsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    clientsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    // Gestion touche Entrée
    clientsTable->installEventFilter(this);

    // Contenu scrollable (graphe + table)
    QWidget* scrollContent = new QWidget;
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0,0,0,0);
    scrollLayout->setSpacing(0);
    scrollLayout->addWidget(chartView);
    scrollLayout->addWidget(clientsTable, 1);

    // Bouton flottant d'ajout avec icône et animation
    addClientBtn = new QPushButton();
    
    // Utiliser une icône claire au lieu de texte "+"
    QIcon addIcon(":/icons/material/group.svg");
    addClientBtn->setIcon(addIcon);
    addClientBtn->setIconSize(QSize(28, 28));
    
    // Améliorer l'accessibilité avec un tooltip détaillé
    addClientBtn->setToolTip("Ajouter un nouvel abonné");
    addClientBtn->setAccessibleName("Ajouter un abonné");
    addClientBtn->setFixedSize(64, 64);
    
    // Style amélioré avec effets visuels
    addClientBtn->setStyleSheet(
        "QPushButton {"
        "   background: #ffd23f;"
        "   color: #3d554b;"
        "   border-radius: 32px;"
        "   border: none;"
        "}"
        "QPushButton:hover {"
        "   background: #ffda65;"
        "}"
        "QPushButton:pressed {"
        "   background: #ffc817;"
        "   margin-top: 2px;"
        "}"
    );
    
    addClientBtn->setCursor(Qt::PointingHandCursor);
    connect(addClientBtn, &QPushButton::clicked, this, &ClientsWidget::onAddClientClicked);

    scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidget(scrollContent);

    // Onglet 1 : graphe d'évolution
    QWidget* evolutionTab = new QWidget;
    QVBoxLayout* evolutionLayout = new QVBoxLayout(evolutionTab);
    evolutionLayout->addWidget(chartView);

    // Onglet 2 : liste des clients (style Compteur)
    QWidget* listeTab = new QWidget;
    QVBoxLayout* listeLayout = new QVBoxLayout(listeTab);
    listeLayout->addWidget(filterWidget);
    listeLayout->addWidget(scrollArea, 1);

    // Création du QTabWidget principal
    QTabWidget* tabWidget = new QTabWidget(this);
    tabWidget->addTab(evolutionTab, "Évolution");
    tabWidget->addTab(listeTab, "Liste des clients");

    // Rafraîchir la liste des clients à chaque sélection de l'onglet
    connect(tabWidget, &QTabWidget::currentChanged, this, [=](int index){
        if (index == 0) { // Onglet "Évolution"
            updateClientsEvolutionChart();
        } else if (index == 1) { // Onglet "Liste des clients"
            QStandardItemModel* model = qobject_cast<QStandardItemModel*>(clientsTable->model());
            if (model) {
                model->removeRows(0, model->rowCount());
                loadClientsFromDatabase(model);
            }
        }
    });

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(tabWidget);
    setLayout(mainLayout);

    // Position absolue du bouton flottant en bas à droite
    addClientBtn->setParent(this);
    addClientBtn->raise(); // Mettre le bouton au premier plan
    const int margin = 24; // Marge en pixels
    addClientBtn->move(width() - addClientBtn->width() - margin, 
                      height() - addClientBtn->height() - margin);
    // Repositionner le bouton lors du redimensionnement
    // Nous devons surcharger resizeEvent dans la classe ClientsWidget
    // Le code est dans le fichier .h et la méthode resizeEvent est implémentée plus bas
}

bool ClientsWidget::eventFilter(QObject* obj, QEvent* event) {
    if (obj == clientsTable && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            QModelIndex index = clientsTable->currentIndex();
            if (index.isValid()) {
                int row = index.row();
                QStandardItemModel* model = qobject_cast<QStandardItemModel*>(clientsTable->model());
                if (row >= 0 && row < clientsTable->model()->rowCount() && model) {
                    QString nom = model->item(row, 1)->text();
                    QString prenom = model->item(row, 2)->text();
                    QString adresse = model->item(row, 3)->text();
                    QString telephone = model->item(row, 4)->text();
                    showClientDetails(nom, prenom, adresse, telephone);
                    return true;
                }
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

// Charger les clients depuis la base de données
void ClientsWidget::loadClientsFromDatabase(QStandardItemModel* model) {
    QSqlQuery query("SELECT idClient, nom, prenom, adresse, telephone FROM Client");
    
    if (!query.exec()) {
        qCritical() << "Erreur lors de la récupération des clients:" << query.lastError().text();
        return;
    }
    
    while (query.next()) {
        QList<QStandardItem*> items;
        for (int i = 0; i < 5; ++i) {
            auto* item = new QStandardItem(query.value(i).toString());
            item->setEditable(false);
            items << item;
        }
        model->appendRow(items);
    }
}

// Ajouter un client à la base de données
int ClientsWidget::addClientToDatabase(const Client& client) {
    // Contrôle du numéro de téléphone sénégalais avec regex
    QRegularExpression regex("^7[0-9]{1}[ .-]?[0-9]{3}[ .-]?[0-9]{2}[ .-]?[0-9]{2}$");
    QRegularExpressionMatch match = regex.match(client.telephone);
    if (!match.hasMatch()) {
        QMessageBox::warning(this, "Numéro invalide", "Le numéro de téléphone n'est pas valide. Format attendu : 7X XXX XX XX");
        return -1;
    }

    QSqlQuery query;
    query.prepare("INSERT INTO Client (nom, prenom, adresse, telephone) VALUES (?, ?, ?, ?)");
    query.addBindValue(client.nom);
    query.addBindValue(client.prenom);
    query.addBindValue(client.adresse);
    query.addBindValue(client.telephone);

    if (!query.exec()) {
        qCritical() << "Erreur lors de l'ajout du client:" << query.lastError().text();
        return -1;
    }

    // Récupérer l'ID généré
    return query.lastInsertId().toInt();
}

void ClientsWidget::onAddClientClicked() {
    ClientFormDialog dialog(this);
    int result = dialog.exec();
    
    if (result == QDialog::Accepted) {
        Client newClient = dialog.getClient();
        
        // Ajouter le client à la base de données
        int clientId = addClientToDatabase(newClient);
        
        if (clientId > 0) {
            // Ajouter le client au modèle
            QStandardItemModel* model = qobject_cast<QStandardItemModel*>(clientsTable->model());
            if (model) {
                QList<QStandardItem*> items;
                items << new QStandardItem(QString::number(clientId));
                items << new QStandardItem(newClient.nom);
                items << new QStandardItem(newClient.prenom);
                items << new QStandardItem(newClient.adresse);
                items << new QStandardItem(newClient.telephone);
                
                // Définir les items comme non-éditables
                for (auto* item : items) {
                    item->setEditable(false);
                }
                
                model->appendRow(items);
                
                // Sélectionner la nouvelle ligne
                QModelIndex newIndex = model->index(model->rowCount() - 1, 0);
                clientsTable->setCurrentIndex(newIndex);
                clientsTable->scrollTo(newIndex);
                
                // Message de confirmation
                QMessageBox::information(this, 
                                        "Client ajouté", 
                                        "Le client " + newClient.prenom + " " + newClient.nom + " a été ajouté avec succès.",
                                        QMessageBox::Ok);
            }
        }
        else {
            QMessageBox::critical(this,
                                "Erreur",
                                "Impossible d'ajouter le client à la base de données.",
                                QMessageBox::Ok);
        }
    }
}

// Méthode pour afficher les détails d'un client
void ClientsWidget::showClientDetails(const QString& nom, const QString& prenom, const QString& adresse, const QString& telephone) {
    // Créer le widget de détail
    ClientDetailWidget* detailWidget = new ClientDetailWidget(this);
    QModelIndex index = clientsTable->currentIndex();
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(clientsTable->model());
    QString idClient;
    if (model && index.isValid()) {
        idClient = model->data(model->index(index.row(), 0)).toString();
    } else {
        idClient = "";
    }
    detailWidget->setClientInfo(idClient, nom, prenom, adresse, telephone);
    connect(detailWidget, SIGNAL(clientUpdated(QString,QString,QString,QString,QString)),
            this, SLOT(updateClientInModel(QString,QString,QString,QString,QString)));
    // Rafraîchir la liste des abonnements après ajout
    connect(detailWidget, &ClientDetailWidget::abonnementAjouteSignal, detailWidget, &ClientDetailWidget::loadAbonnementsFromDB);
    // Rafraîchir la liste des factures après ajout
    connect(detailWidget, &ClientDetailWidget::factureAjouteSignal, detailWidget, &ClientDetailWidget::loadFacturesFromDB);
    // Rafraîchir la liste de consommation après ajout
    // Ne pas connecter consommationAjouteSignal à loadConsommationFromDB car ce slot est privé

    // Remplacer la table par le widget de détail dans l'onglet 'Liste des clients'
    // On suppose que le parent de clientsTable est le layout de l'onglet listeTab
    if (clientsTable && clientsTable->parentWidget()) {
        QVBoxLayout* listeLayout = qobject_cast<QVBoxLayout*>(clientsTable->parentWidget()->layout());
        if (listeLayout) {
            int idx = listeLayout->indexOf(clientsTable);
            if (idx != -1) {
                listeLayout->removeWidget(clientsTable);
                clientsTable->setParent(nullptr);
                listeLayout->insertWidget(idx, detailWidget);
                // Connecter le bouton retour
                connect(detailWidget, &ClientDetailWidget::retourClicked, this, [=]() {
                    // Remettre la table à la place du détail
                    // Mettre à jour le modèle client avant de revenir à la liste
                    QModelIndex index = clientsTable->currentIndex();
                    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(clientsTable->model());
                    QString idClient;
                    if (model && index.isValid()) {
                        idClient = model->data(model->index(index.row(), 0)).toString();
                    } else {
                        idClient = "";
                    }
                    // Recharger les données du client depuis la base
                    QSqlQuery reloadQuery(QSqlDatabase::database());
                    reloadQuery.prepare("SELECT nom, prenom, adresse, telephone FROM Client WHERE idClient = :idClient");
                    reloadQuery.bindValue(":idClient", idClient.toInt());
                    if (reloadQuery.exec() && reloadQuery.next()) {
                        updateClientInModel(idClient,
                            reloadQuery.value(0).toString(),
                            reloadQuery.value(1).toString(),
                            reloadQuery.value(2).toString(),
                            reloadQuery.value(3).toString());
                    }
                    listeLayout->removeWidget(detailWidget);
                    detailWidget->deleteLater();
                    listeLayout->insertWidget(idx, clientsTable);
                });
            }
        }
    }
}

// Méthode pour revenir à la liste des clients
void ClientsWidget::showClientsList() {
    // Nettoyer le layout principal
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (mainLayout) {
        // Supprimer tous les widgets sauf la barre de filtres
        while (mainLayout->count() > 1) {
            QLayoutItem* item = mainLayout->takeAt(1);
            if (item) {
                QWidget* w = item->widget();
                if (w && w != scrollArea) { // Ne pas supprimer le scrollArea
                    w->deleteLater();
                }
                delete item;
            }
        }

        // Rafraîchir la table des clients depuis la base
        QStandardItemModel* model = qobject_cast<QStandardItemModel*>(clientsTable->model());
        if (model) {
            model->removeRows(0, model->rowCount());
            loadClientsFromDatabase(model);
        }

        // Réajouter le scrollArea au layout principal
        mainLayout->addWidget(scrollArea, 1);
    }

    // S'assurer que le bouton flottant est visible et en premier plan après le retour à la liste
    if (addClientBtn) {
        addClientBtn->show();
        addClientBtn->raise();
    }
}

// Gestion du redimensionnement pour repositionner le bouton flottant
void ClientsWidget::resizeEvent(QResizeEvent* event) {
    // Appeler l'implémentation de base
    QWidget::resizeEvent(event);
    
    // Repositionner le bouton flottant en bas à droite
    if (addClientBtn) {
        const int margin = 24; // Même marge que celle définie dans le constructeur
        addClientBtn->move(width() - addClientBtn->width() - margin, 
                          height() - addClientBtn->height() - margin);
    }
}

// Slot pour mettre à jour le modèle après modification d'un client
void ClientsWidget::updateClientInModel(const QString& idClient, const QString& nom, const QString& prenom, const QString& adresse, const QString& telephone) {
    // Contrôle du numéro de téléphone sénégalais avec regex
    QRegularExpression regex("^7[0-9]{1}[ .-]?[0-9]{3}[ .-]?[0-9]{2}[ .-]?[0-9]{2}$");
    QRegularExpressionMatch match = regex.match(telephone);
    if (!match.hasMatch()) {
        QMessageBox::warning(this, "Numéro invalide", "Le numéro de téléphone n'est pas valide. Format attendu : 7X XXX XX XX");
        return;
    }

    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(clientsTable->model());
    if (!model) return;
    for (int row = 0; row < model->rowCount(); ++row) {
        if (model->item(row, 0)->text() == idClient) {
            model->item(row, 1)->setText(nom);
            model->item(row, 2)->setText(prenom);
            model->item(row, 3)->setText(adresse);
            model->item(row, 4)->setText(telephone);
            break;
        }
    }
}