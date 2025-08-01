#include "widgets/clientdetailwidget.h"
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

// Implémentation style dashboard : QtCharts utilisé uniquement ici
#include <QtCharts/QChartView>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>

#include <QRandomGenerator>


ClientsWidget::ClientsWidget(QWidget* parent) : QWidget(parent) {
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

    // Table des abonnés (QTableView + QStandardItemModel)
    clientsTable = new QTableView;
    clientsTable->setMinimumHeight(340); // Hauteur du conteneur de la table
    clientsTable->setStyleSheet("background: #22332d; color: #e6e6e6; border-radius: 12px; font-size: 17px; border: none; margin-top: 18px; padding: 8px 16px;");
    clientsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    clientsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    clientsTable->setShowGrid(false);
    clientsTable->setAlternatingRowColors(true);
    clientsTable->verticalHeader()->setDefaultSectionSize(38);

    // Modèle de données fictif basé sur le schéma SQL
    QStandardItemModel* model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"ID", "Nom", "Prénom", "Adresse", "Téléphone"});
    // Données fictives
    QList<QList<QVariant>> fakeClients = {
        {1, "Sow", "Fatou", "123 rue de l'eau, Dakar", "771234567"},
        {2, "Diop", "Moussa", "45 avenue des Abonnés, Thiès", "781112233"},
        {3, "Ba", "Aminata", "Villa 7, St-Louis", "765432198"},
        {4, "Sy", "Ousmane", "Quartier Liberté, Kaolack", "770987654"},
        {5, "Fall", "Awa", "Immeuble 12, Ziguinchor", "772345678"}
    };
    for (int i = 0; i < fakeClients.size(); ++i) {
        const auto& row = fakeClients[i];
        QList<QStandardItem*> items;
        for (const auto& val : row) {
            auto* item = new QStandardItem(val.toString());
            item->setEditable(false);
            items << item;
        }
        model->appendRow(items);
    }
    clientsTable->setModel(model);
    clientsTable->horizontalHeader()->setVisible(true);
    clientsTable->horizontalHeader()->setStretchLastSection(true);
    clientsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // Ouvrir les détails du client sur double-clic ou touche Entrée
    connect(clientsTable, &QTableView::doubleClicked, this, [=](const QModelIndex& index){
        int row = index.row();
        if (row >= 0 && row < model->rowCount()) {
            QString nom = model->item(row, 1)->text();
            QString prenom = model->item(row, 2)->text();
            QString adresse = model->item(row, 3)->text();
            QString telephone = model->item(row, 4)->text();
            auto* detailWidget = new ClientDetailWidget;
            detailWidget->setClientInfo(nom, prenom, adresse, telephone);
            QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
            if (mainLayout) {
                while (mainLayout->count() > 1) {
                    QLayoutItem* item = mainLayout->takeAt(1);
                    if (item) {
                        QWidget* w = item->widget();
                        if (w) w->deleteLater();
                        delete item;
                    }
                }
                mainLayout->addWidget(detailWidget, 1);
                connect(detailWidget, &ClientDetailWidget::retourClicked, this, [=]() {
                    // Réafficher la liste des clients (scrollArea)
                    while (mainLayout->count() > 1) {
                        QLayoutItem* item = mainLayout->takeAt(1);
                        if (item) {
                            QWidget* w = item->widget();
                            if (w) w->deleteLater();
                            delete item;
                        }
                    }
                    mainLayout->addWidget(scrollArea, 1);
                });
            }
        }
    });
    clientsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    clientsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    clientsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    // Gestion touche Entrée
    clientsTable->installEventFilter(this);

    // Bouton flottant d'ajout
    addClientBtn = new QPushButton("/+");
    addClientBtn->setToolTip("Ajouter un abonné");
    addClientBtn->setFixedSize(54,54);
    addClientBtn->setStyleSheet("background: #ffd23f; color: #3d554b; font-size: 32px; font-weight: bold; border-radius: 27px; box-shadow: 0 2px 8px rgba(0,0,0,0.10); position: absolute;");
    addClientBtn->setCursor(Qt::PointingHandCursor);

    // Contenu scrollable (graphe + table + bouton)
    QWidget* scrollContent = new QWidget;
    QVBoxLayout* scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0,0,0,0);
    scrollLayout->setSpacing(0);
    scrollLayout->addWidget(chartView);
    scrollLayout->addWidget(clientsTable, 1);
    QHBoxLayout* btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    btnLayout->addWidget(addClientBtn);
    btnLayout->setContentsMargins(0,0,24,24);
    scrollLayout->addLayout(btnLayout);

    QScrollArea* scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidget(scrollContent);

    // Layout principal : barre de recherche fixe, contenu scrollable
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(filterWidget); // barre fixe
    mainLayout->addWidget(scrollArea, 1);
}


// ...existing code...

bool ClientsWidget::eventFilter(QObject* obj, QEvent* event) {
    if (obj == clientsTable && event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            QModelIndex index = clientsTable->currentIndex();
            if (index.isValid()) {
                int row = index.row();
                if (row >= 0 && row < clientsTable->model()->rowCount()) {
                    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(clientsTable->model());
                    if (model) {
                        QString nom = model->item(row, 1)->text();
                        QString prenom = model->item(row, 2)->text();
                        QString adresse = model->item(row, 3)->text();
                        QString telephone = model->item(row, 4)->text();
                        auto* detailWidget = new ClientDetailWidget;
                        detailWidget->setClientInfo(nom, prenom, adresse, telephone);
                        QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(this->layout());
                        if (mainLayout) {
                            while (mainLayout->count() > 1) {
                                QLayoutItem* item = mainLayout->takeAt(1);
                                if (item) {
                                    QWidget* w = item->widget();
                                    if (w) w->deleteLater();
                                    delete item;
                                }
                            }
                            mainLayout->addWidget(detailWidget, 1);
                            connect(detailWidget, &ClientDetailWidget::retourClicked, this, [=]() {
                                while (mainLayout->count() > 1) {
                                    QLayoutItem* item = mainLayout->takeAt(1);
                                    if (item) {
                                        QWidget* w = item->widget();
                                        if (w) w->deleteLater();
                                        delete item;
                                    }
                                }
                                mainLayout->addWidget(scrollArea, 1);
                            });
                        }
                    }
                }
                return true;
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}