#include <QLineEdit>
#include <QComboBox>
#include "widgets/dashboardwidget.h"
#include <QLabel>
#include <QIcon>
#include <QPixmap>
#include <QFrame>
#include <QPalette>
#include <QSqlQuery>
#include <QSqlError>
#include "widgets/dashboardchartwidget.h"

DashboardWidget::DashboardWidget(Controller* controller, QWidget *parent)
    : QWidget(parent), m_controller(controller)
{
    // Menu vertical moderne avec logo et nom en haut
    QFrame* menuFrame = new QFrame;
    menuFrame->setFixedWidth(72);
    menuFrame->setStyleSheet(
        "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #232946, stop:1 #3e4a89);"
        "border: none;"
        "border-radius: 0px;"
    );
    QVBoxLayout* menuLayout = new QVBoxLayout(menuFrame);
    menuLayout->setContentsMargins(0, 0, 0, 0);
    menuLayout->setSpacing(0);

    // Logo et nom de l'app en haut du menu
    QWidget* logoWidget = new QWidget;
    QVBoxLayout* logoLayout = new QVBoxLayout(logoWidget);
    logoLayout->setAlignment(Qt::AlignHCenter);
    QLabel* logo = new QLabel;
    logo->setPixmap(QPixmap(":/logo_watersn.png").scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    logo->setStyleSheet("margin-top: 18px; margin-bottom: 8px;");
    logoLayout->addWidget(logo);
    logoWidget->setLayout(logoLayout);
    menuLayout->addWidget(logoWidget);

    // Menu navigation
    navigationMenu = new QListWidget;
    navigationMenu->setStyleSheet(
        "QListWidget { background: transparent; border: none; color: #b8c1ec; font-size: 13px; }"
        "QListWidget::item { height: 56px; padding-left: 0px; border-radius: 12px; margin: 6px 0px; text-align: center; }"
        "QListWidget::item:selected { background: #fff; color: #232946; font-weight: bold; }"
        "QListWidget::item:hover { background: #3e4a89; color: #fff; }"
        "QListWidget::item:selected:!active { background: #fff; color: #232946; }"
    );
    navigationMenu->setIconSize(QSize(28,28));
    QListWidgetItem* dashItem = new QListWidgetItem(QIcon(":/icons/dashboard.png"), "");
    dashItem->setToolTip("Dashboard");
    navigationMenu->addItem(dashItem);
    QListWidgetItem* clientsItem = new QListWidgetItem(QIcon(":/icons/clients.png"), "");
    clientsItem->setToolTip("Clients");
    navigationMenu->addItem(clientsItem);
    QListWidgetItem* compteursItem = new QListWidgetItem(QIcon(":/icons/compteurs.png"), "");
    compteursItem->setToolTip("Compteurs");
    navigationMenu->addItem(compteursItem);
    QListWidgetItem* paramsItem = new QListWidgetItem(QIcon(":/icons/settings.png"), "");
    paramsItem->setToolTip("Paramètres");
    navigationMenu->addItem(paramsItem);
    navigationMenu->setFixedWidth(72);
    menuLayout->addWidget(navigationMenu, 1);
    menuLayout->addSpacing(10);


    // Filtres de recherche (ligne au-dessus des KPI)
    QWidget* filterWidget = new QWidget;
    QHBoxLayout* filterLayout = new QHBoxLayout(filterWidget);
    filterLayout->setContentsMargins(30, 10, 30, 0);
    filterLayout->setSpacing(18);

    searchLineEdit = new QLineEdit;
    searchLineEdit->setPlaceholderText("Nom abonné...");
    searchLineEdit->setMinimumWidth(160);
    filterLayout->addWidget(searchLineEdit);

    collectiviteCombo = new QComboBox;
    collectiviteCombo->addItem("Tous");
    collectiviteCombo->addItem("Collectivité 1");
    collectiviteCombo->addItem("Collectivité 2");
    filterLayout->addWidget(collectiviteCombo);

    periodeCombo = new QComboBox;
    periodeCombo->addItem("Tous");
    periodeCombo->addItem("2024");
    periodeCombo->addItem("2025");
    filterLayout->addWidget(periodeCombo);

    etatCompteurCombo = new QComboBox;
    etatCompteurCombo->addItem("Tous");
    etatCompteurCombo->addItem("Actif");
    etatCompteurCombo->addItem("Inactif");
    filterLayout->addWidget(etatCompteurCombo);

    // Section KPI (en haut du dashboard)
    kpiWidget = new QWidget;
    QHBoxLayout* kpiLayout = new QHBoxLayout(kpiWidget);
    kpiLayout->setContentsMargins(30, 20, 30, 20);
    kpiLayout->setSpacing(30);

    auto makeKpi = [](const QString& label, const QString& value, const QString& color) {
        QWidget* box = new QWidget;
        box->setStyleSheet(QString("background:%1; border-radius:12px; padding:16px;").arg(color));
        QVBoxLayout* v = new QVBoxLayout(box);
        QLabel* l = new QLabel(label);
        l->setStyleSheet("color:#fff; font-size:14px;");
        QLabel* vLabel = new QLabel(value);
        vLabel->setStyleSheet("color:#fff; font-size:24px; font-weight:bold;");
        v->addWidget(l);
        v->addWidget(vLabel);
        v->setAlignment(Qt::AlignCenter);
        return std::make_pair(box, vLabel);
    };

    auto abonneKpi = makeKpi("Abonnés", "0", "#1976d2");
    auto totalKpi = makeKpi("Total facturé", "0", "#43a047");
    auto impayeKpi = makeKpi("Impayés", "0", "#e53935");
    auto alerteKpi = makeKpi("Alertes", "0", "#fbc02d");
    kpiAbonnes = abonneKpi.second;
    kpiTotalFacture = totalKpi.second;
    kpiImpayes = impayeKpi.second;
    kpiAlertes = alerteKpi.second;
    kpiLayout->addWidget(abonneKpi.first);
    kpiLayout->addWidget(totalKpi.first);
    kpiLayout->addWidget(impayeKpi.first);
    kpiLayout->addWidget(alerteKpi.first);

    // Widget pour la liste des alertes
    alertListWidget = new QListWidget;
    alertListWidget->setStyleSheet(
        "background: #fff; border-radius: 12px; font-size: 14px; color: #232946; margin: 10px 30px 10px 30px;"
        "border: 1.5px solid #e0e0e0;"
        "padding: 8px 0px;"
        "QListWidget::item { border-radius: 8px; margin: 2px 0; padding: 8px 12px; }"
        "QListWidget::item:selected { background: #e3e8ff; color: #232946; }"
        "QListWidget::item:hover { background: #f5f7ff; color: #232946; }"
    );
    alertListWidget->setMinimumHeight(120);
    alertListWidget->setMaximumHeight(180);
    alertListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    alertListWidget->setFocusPolicy(Qt::NoFocus);

    // Zone centrale
    contentStack = new QStackedWidget;
    contentStack->addWidget(new QLabel("Bienvenue sur le Dashboard !"));
    contentStack->addWidget(new QLabel("Section Clients"));
    contentStack->addWidget(new QLabel("Section Compteurs"));
    contentStack->addWidget(new QLabel("Section Paramètres"));


    QVBoxLayout* rightLayout = new QVBoxLayout;
    rightLayout->addWidget(filterWidget);
    rightLayout->addWidget(kpiWidget);
    rightLayout->addWidget(alertListWidget);
    rightLayout->addWidget(contentStack, 1);

    // Widget graphique (chart)
    chartWidget = new DashboardChartWidget;
    rightLayout->addWidget(chartWidget);

    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(menuFrame);
    mainLayout->addLayout(rightLayout, 1);
    setLayout(mainLayout);

    connect(navigationMenu, &QListWidget::currentRowChanged, contentStack, &QStackedWidget::setCurrentIndex);
    navigationMenu->setCurrentRow(0);
    updateKpi();

    // Connexion des filtres à l'application dynamique
    connect(searchLineEdit, &QLineEdit::textChanged, this, &DashboardWidget::applyFilters);
    connect(collectiviteCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DashboardWidget::applyFilters);
    connect(periodeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DashboardWidget::applyFilters);
    connect(etatCompteurCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DashboardWidget::applyFilters);
}
void DashboardWidget::applyFilters()
{
    // Récupérer les valeurs des filtres et les stocker dans des membres
    currentNom = searchLineEdit->text();
    currentCollectivite = collectiviteCombo->currentText();
    currentPeriode = periodeCombo->currentText();
    currentEtat = etatCompteurCombo->currentText();

    updateKpi();
    updateAlerts();
    if (chartWidget) chartWidget->updateCharts();
}
void DashboardWidget::updateAlerts()
{
    // Construction dynamique des filtres SQL pour les alertes
    QString abonneWhere = "1=1";
    if (!currentNom.isEmpty())
        abonneWhere += " AND nom LIKE '%" + currentNom.replace("'", "''") + "%'";
    if (!currentCollectivite.isEmpty() && currentCollectivite != "Tous")
        abonneWhere += " AND collectivite='" + currentCollectivite.replace("'", "''") + "'";
    if (!currentEtat.isEmpty() && currentEtat != "Tous")
        abonneWhere += " AND etat_compteur='" + currentEtat.replace("'", "''") + "'";

    // On peut aussi filtrer par période si pertinent (exemple: date_alerte)
    QString periodeWhere = "1=1";
    if (!currentPeriode.isEmpty() && currentPeriode != "Tous")
        periodeWhere += " AND al.periode='" + currentPeriode.replace("'", "''") + "'";

    // Affichage de la liste des alertes filtrées (exemple simplifié)
    QString alerteQuery = "SELECT al.id, al.type, al.message, al.date_alerte FROM Alerte al JOIN Abonne a ON al.abonne_id=a.id WHERE " + abonneWhere + " AND " + periodeWhere + " ORDER BY al.date_alerte DESC LIMIT 10";
    QSqlQuery q(alerteQuery);
    QStringList alertes;
    while (q.next()) {
        QString type = q.value(1).toString();
        QString msg = q.value(2).toString();
        QString date = q.value(3).toString();
        alertes << (date + " - [" + type + "] " + msg);
    }
    // Affichage dans le widget d'alerte du dashboard
    if (alertListWidget) {
        alertListWidget->clear();
        if (alertes.isEmpty())
            alertListWidget->addItem("Aucune alerte récente.");
        else
            alertListWidget->addItems(alertes);
    }
}
void DashboardWidget::updateKpi()
{
    // Construction dynamique des filtres SQL
    QString abonneWhere = "1=1";
    if (!currentNom.isEmpty())
        abonneWhere += " AND nom LIKE '%" + currentNom.replace("'", "''") + "%'";
    if (!currentCollectivite.isEmpty() && currentCollectivite != "Tous")
        abonneWhere += " AND collectivite='" + currentCollectivite.replace("'", "''") + "'";
    if (!currentEtat.isEmpty() && currentEtat != "Tous")
        abonneWhere += " AND etat_compteur='" + currentEtat.replace("'", "''") + "'";

    // Nombre d'abonnés filtré
    QSqlQuery q1("SELECT COUNT(*) FROM Abonne WHERE " + abonneWhere);
    if (q1.next())
        kpiAbonnes->setText(q1.value(0).toString());
    else
        kpiAbonnes->setText("-");

    // Total facturé filtré (jointure avec Abonne pour appliquer les mêmes filtres)
    QString factureWhere = "1=1";
    if (!currentPeriode.isEmpty() && currentPeriode != "Tous")
        factureWhere += " AND periode='" + currentPeriode.replace("'", "''") + "'";
    // On applique aussi les filtres abonné
    QString totalFactureQuery = "SELECT SUM(f.montant) FROM Facture f JOIN Abonne a ON f.abonne_id=a.id WHERE " + abonneWhere + " AND " + factureWhere;
    QSqlQuery q2(totalFactureQuery);
    if (q2.next())
        kpiTotalFacture->setText(q2.value(0).isNull() ? "0" : q2.value(0).toString());
    else
        kpiTotalFacture->setText("-");

    // Nombre d'impayés filtré
    QString impayeQuery = "SELECT COUNT(*) FROM Facture f JOIN Abonne a ON f.abonne_id=a.id WHERE f.paye=0 AND " + abonneWhere + " AND " + factureWhere;
    QSqlQuery q3(impayeQuery);
    if (q3.next())
        kpiImpayes->setText(q3.value(0).toString());
    else
        kpiImpayes->setText("-");

    // Nombre d'alertes filtré (exemple: consommation anormale)
    QString alerteQuery = "SELECT COUNT(*) FROM Alerte al JOIN Abonne a ON al.abonne_id=a.id WHERE al.etat='anormale' AND " + abonneWhere;
    QSqlQuery q4(alerteQuery);
    if (q4.next())
        kpiAlertes->setText(q4.value(0).toString());
    else
        kpiAlertes->setText("-");
}
