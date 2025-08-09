#include "widgets/clientdetailwidget.h"
#include "widgets/clientformdialog.h"
#include <QMessageBox>
#include <QKeyEvent>
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

// Effets et animations
#include <QGraphicsEffect>
#include <QPropertyAnimation>
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
    collectiviteCombo = new QComboBox;
    collectiviteCombo->addItem("Tous"); collectiviteCombo->addItem("Collectivité 1"); collectiviteCombo->addItem("Collectivité 2");
    consommationCombo = new QComboBox;
    consommationCombo->addItem("Toutes consommations"); consommationCombo->addItem("0-10 m³"); consommationCombo->addItem("10-50 m³"); consommationCombo->addItem(">50 m³");
    QString comboStyle = "QComboBox { background: #22332d; color: #e6e6e6; border-radius: 6px; padding: 4px 12px; font-size: 14px; min-width: 80px; } QComboBox QAbstractItemView { background: #31463f; color: #e6e6e6; border-radius: 6px; }";
    collectiviteCombo->setStyleSheet(comboStyle);
    consommationCombo->setStyleSheet(comboStyle);
    filterLayout->addWidget(collectiviteCombo);
    filterLayout->addWidget(consommationCombo);

    // Graphe d'évolution des clients (style dashboard)
    QLineSeries* series = new QLineSeries();
    series->setName("Clients");
    series->setColor(QColor("#42a5f5"));
    series->setPointsVisible(true);
    QStringList moisLabels = {"Janv", "Fév", "Mars", "Avr", "Mai", "Juin", "Juil", "Août", "Sept", "Oct", "Nov", "Déc"};
    for (int m = 1; m <= 12; ++m)
        series->append(m, 10 + QRandomGenerator::global()->bounded(30));
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Évolution des clients par mois");
    QFont chartTitleFont;
    chartTitleFont.setPointSize(18);
    chartTitleFont.setBold(true);
    chart->setTitleFont(chartTitleFont);
    chart->setBackgroundBrush(QColor("#f5f5f5"));
    chart->legend()->hide();

    // Personnalisation axes
    QCategoryAxis* axisX = new QCategoryAxis();
    chart->addAxis(axisX, Qt::AlignBottom);
    for (int m = 0; m < moisLabels.size(); ++m) {
        axisX->append(moisLabels[m], m+1);
    }
    axisX->setRange(0, 13);
    axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    series->attachAxis(axisX);

    QValueAxis* axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    axisY->setRange(0, 50);
    axisY->setTickCount(6);
    series->attachAxis(axisY);

    QChartView* chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumHeight(220);

    // Tableau des clients
    clientsTable = new QTableView;
    QStandardItemModel* model = new QStandardItemModel(0, 5, this);
    model->setHorizontalHeaderLabels({"ID", "Nom", "Prénom", "Adresse", "Téléphone"});
    clientsTable->setModel(model);
    loadClientsFromDatabase(model);
    clientsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    clientsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    clientsTable->setColumnWidth(0, 80);
    clientsTable->setStyleSheet("QTableView { background-color: #ffffff; alternate-background-color: #f5f5f5; } QTableView::item:selected { background-color: #b3e5fc; color: #000000; }");
    clientsTable->setAlternatingRowColors(true);
    connect(clientsTable, &QTableView::doubleClicked, [this](const QModelIndex &index){
        if (index.isValid()) {
            QAbstractItemModel* model = clientsTable->model();
            int row = index.row();
            
            // Récupérer les données du client sélectionné
            QModelIndex idIndex = model->index(row, 0);
            QModelIndex nomIndex = model->index(row, 1);
            QModelIndex prenomIndex = model->index(row, 2);
            QModelIndex adresseIndex = model->index(row, 3);
            QModelIndex telephoneIndex = model->index(row, 4);
            
            QString nom = model->data(nomIndex).toString();
            QString prenom = model->data(prenomIndex).toString();
            QString adresse = model->data(adresseIndex).toString();
            QString telephone = model->data(telephoneIndex).toString();
            
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

    scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidget(scrollContent);

    // Layout principal : barre de recherche fixe, contenu scrollable
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(filterWidget); // barre fixe
    mainLayout->addWidget(scrollArea, 1);

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

    // Position absolue du bouton flottant en bas à droite avec meilleur positionnement
    addClientBtn->setParent(this);
    addClientBtn->raise(); // Mettre le bouton au premier plan
    
    // Position initiale avec espace de marge autour
    const int margin = 24; // Marge en pixels
    addClientBtn->move(width() - addClientBtn->width() - margin, 
                      height() - addClientBtn->height() - margin);
    
    // Repositionnement dynamique lors du redimensionnement
    connect(this, &QWidget::resized, [this, margin]() {
        addClientBtn->move(width() - addClientBtn->width() - margin, 
                          height() - addClientBtn->height() - margin);
    });
}

bool ClientsWidget::eventFilter(QObject* obj, QEvent* event) {
    if (obj == clientsTable && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            QModelIndex index = clientsTable->currentIndex();
            if (index.isValid()) {
                QAbstractItemModel* model = clientsTable->model();
                int row = index.row();
                
                // Récupérer les données du client sélectionné
                QModelIndex nomIndex = model->index(row, 1);
                QModelIndex prenomIndex = model->index(row, 2);
                QModelIndex adresseIndex = model->index(row, 3);
                QModelIndex telephoneIndex = model->index(row, 4);
                
                QString nom = model->data(nomIndex).toString();
                QString prenom = model->data(prenomIndex).toString();
                QString adresse = model->data(adresseIndex).toString();
                QString telephone = model->data(telephoneIndex).toString();
                
                showClientDetails(nom, prenom, adresse, telephone);
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void ClientsWidget::loadClientsFromDatabase(QStandardItemModel* model) {
    QSqlQuery query("SELECT * FROM client");
    int row = 0;
    while (query.next()) {
        int id = query.value("id").toInt();
        QString nom = query.value("nom").toString();
        QString prenom = query.value("prenom").toString();
        QString adresse = query.value("adresse").toString();
        QString telephone = query.value("telephone").toString();
        
        model->insertRow(row);
        model->setData(model->index(row, 0), id);
        model->setData(model->index(row, 1), nom);
        model->setData(model->index(row, 2), prenom);
        model->setData(model->index(row, 3), adresse);
        model->setData(model->index(row, 4), telephone);
        
        row++;
    }
}

void ClientsWidget::onAddClientClicked() {
    ClientFormDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Client newClient = dialog.getClient();
        int id = addClientToDatabase(newClient);
        if (id > 0) {
            QStandardItemModel* model = qobject_cast<QStandardItemModel*>(clientsTable->model());
            int row = model->rowCount();
            model->insertRow(row);
            model->setData(model->index(row, 0), id);
            model->setData(model->index(row, 1), newClient.nom);
            model->setData(model->index(row, 2), newClient.prenom);
            model->setData(model->index(row, 3), newClient.adresse);
            model->setData(model->index(row, 4), newClient.telephone);
            
            QMessageBox::information(this, "Succès", "Client ajouté avec succès !");
        } else {
            QMessageBox::critical(this, "Erreur", "Erreur lors de l'ajout du client.");
        }
    }
}

int ClientsWidget::addClientToDatabase(const Client& client) {
    QSqlQuery query;
    query.prepare("INSERT INTO client (nom, prenom, adresse, telephone) VALUES (?, ?, ?, ?)");
    query.addBindValue(client.nom);
    query.addBindValue(client.prenom);
    query.addBindValue(client.adresse);
    query.addBindValue(client.telephone);
    
    if (query.exec()) {
        return query.lastInsertId().toInt();
    } else {
        qDebug() << "Erreur lors de l'ajout du client:" << query.lastError().text();
        return -1;
    }
}

void ClientsWidget::showClientDetails(const QString& nom, const QString& prenom, const QString& adresse, const QString& telephone) {
    ClientDetailWidget* detailWidget = new ClientDetailWidget(nom, prenom, adresse, telephone);
    detailWidget->setAttribute(Qt::WA_DeleteOnClose);
    detailWidget->show();
}

void ClientsWidget::showClientsList() {
    // Rafraîchir la liste des clients depuis la base de données
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(clientsTable->model());
    model->removeRows(0, model->rowCount());
    loadClientsFromDatabase(model);
}
