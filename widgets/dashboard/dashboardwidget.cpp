#include <QDebug>
#include <QLineEdit>
#include <QComboBox>
#include "widgets/dashboardwidget.h"
#include "widgets/compteurwidget.h"
#include <QLabel>
#include "widgets/clientswidget.h"
#include <QIcon>
#include <QPixmap>
#include <QFrame>
#include <QPalette>
#include <QSqlQuery>
#include <QSqlError>
#include "widgets/dashboardchartwidget.h"
#include <QPushButton>
#include <QGraphicsDropShadowEffect>
#include <QListWidget>
#include <QButtonGroup>
#include <QtSvg/QSvgRenderer>
#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLegend>
#include <QtCharts/QCategoryAxis>
#include <QRandomGenerator>

DashboardWidget::DashboardWidget(Controller* controller, QWidget *parent)
    // --- STYLE AVANCÉ POUR QComboBox (filtres) ---
    // (doit être appliqué après la création des QComboBox)
    : QWidget(parent), m_controller(controller), contentStack(nullptr)
{
    qDebug() << "[DashboardWidget] Construction en cours...";
    Q_ASSERT_X(m_controller != nullptr, "DashboardWidget", "Le pointeur Controller est nul !");
    qDebug() << "[DashboardWidget] Controller OK";

    // Correction crash : la géométrie sera appliquée dans showEvent
    this->setMinimumSize(900, 600);
    qDebug() << "[DashboardWidget] Correction: taille minimale 900x600 (showEvent gère le resize)";
   // (Header supprimé)

    // --- ZONE CENTRALE (QStackedWidget pour navigation multi-pages) ---
    qDebug() << "[DashboardWidget] Initialisation contentStack (QStackedWidget navigation)";
    contentStack = new QStackedWidget(this);
    Q_ASSERT(contentStack != nullptr);
    // Page 0 : Dashboard (page principale)
    QWidget* dashboardPage = new QWidget;
    // Création d'un widget de contenu pour le scroll
    QWidget* dashboardContent = new QWidget;
    QVBoxLayout* dashboardLayout = new QVBoxLayout(dashboardContent);
    dashboardLayout->setContentsMargins(0,0,0,0);
    dashboardLayout->setSpacing(0);
    // Création du widget de filtres (doit être fait avant l'ajout au layout)
    qDebug() << "[DashboardWidget] Création filterWidget";
    QWidget* filterWidget = new QWidget;
    qDebug() << "[DashboardWidget] filterWidget créé, sizeHint:" << filterWidget->sizeHint();
    Q_ASSERT(filterWidget != nullptr);
    filterWidget->setStyleSheet("background: #31463f; border-radius: 12px; padding: 12px 18px;");
    QHBoxLayout* filterLayout = new QHBoxLayout(filterWidget);
    Q_ASSERT(filterLayout != nullptr);
    qDebug() << "[DashboardWidget] Layout filterWidget prêt";
    filterLayout->setSpacing(18);
    searchLineEdit = new QLineEdit;
    Q_ASSERT(searchLineEdit != nullptr);
    searchLineEdit->setPlaceholderText("Nom abonné...");
    searchLineEdit->setMinimumWidth(160);
    searchLineEdit->setStyleSheet("background: #22332d; border: none; border-radius: 6px; padding: 6px 10px; font-size: 14px; color: #e6e6e6;");
    filterLayout->addWidget(searchLineEdit);
    collectiviteCombo = new QComboBox;
    Q_ASSERT(collectiviteCombo != nullptr);
    collectiviteCombo->addItem("Tous"); collectiviteCombo->addItem("Collectivité 1"); collectiviteCombo->addItem("Collectivité 2");
    filterLayout->addWidget(collectiviteCombo);
    periodeCombo = new QComboBox;
    Q_ASSERT(periodeCombo != nullptr);
    periodeCombo->addItem("Tous"); periodeCombo->addItem("2024"); periodeCombo->addItem("2025");
    filterLayout->addWidget(periodeCombo);
    etatCompteurCombo = new QComboBox;
    Q_ASSERT(etatCompteurCombo != nullptr);
    etatCompteurCombo->addItem("Tous"); etatCompteurCombo->addItem("Actif"); etatCompteurCombo->addItem("Inactif");
    filterLayout->addWidget(etatCompteurCombo);
    // Appliquer le style après la création des combos
    QString comboStyle = "QComboBox { background: #22332d; color: #e6e6e6; border-radius: 6px; padding: 4px 12px; font-size: 14px; min-width: 80px; } "
                         "QComboBox QAbstractItemView { background: #31463f; color: #e6e6e6; border-radius: 6px; }";
    collectiviteCombo->setStyleSheet(comboStyle);
    periodeCombo->setStyleSheet(comboStyle);
    etatCompteurCombo->setStyleSheet(comboStyle);
    qDebug() << "[DashboardWidget] filterWidget prêt à être ajouté au layout";
    // Encapsuler le contenu dashboard dans un QScrollArea
    QScrollArea* dashboardScroll = new QScrollArea(dashboardPage);
    dashboardScroll->setWidgetResizable(true);
    dashboardScroll->setFrameShape(QFrame::NoFrame);
    dashboardScroll->setWidget(dashboardContent);
    QVBoxLayout* dashboardPageLayout = new QVBoxLayout(dashboardPage);
    dashboardPageLayout->setContentsMargins(0,0,0,0);
    dashboardPageLayout->setSpacing(0);
    dashboardPageLayout->addWidget(filterWidget); // Barre de recherche fixe
    // --- Création et initialisation de kpiWidget AVANT son ajout au layout ---
    qDebug() << "[DashboardWidget] Création kpiWidget";
    kpiWidget = new QWidget;
    Q_ASSERT(kpiWidget != nullptr);
    QHBoxLayout* kpiLayout = new QHBoxLayout(kpiWidget);
    Q_ASSERT(kpiLayout != nullptr);
    kpiLayout->setContentsMargins(18, 12, 18, 12);
    kpiLayout->setSpacing(18);

    auto makeKpi = [](const QString& label, const QString& value, const QString& color) {
        QWidget* box = new QWidget;
        box->setStyleSheet("background:#31463f; border-radius:16px; padding:10px 8px; border: 1.5px solid #22332d; box-shadow: 0 2px 8px rgba(0,0,0,0.10);");
        QVBoxLayout* v = new QVBoxLayout(box);
        v->setSpacing(2);
        QLabel* l = new QLabel(label);
        l->setStyleSheet("color:#b7e0c0; font-size:13px; font-family: 'Segoe UI', 'Arial', sans-serif; letter-spacing: 1px; margin-bottom:2px;");
        QLabel* vLabel = new QLabel(value);
        vLabel->setStyleSheet("color:#fff; font-size:32px; font-weight:bold; font-family: 'Segoe UI', 'Arial', sans-serif; letter-spacing: 1px; margin-top:2px;");
        v->addWidget(l, 0, Qt::AlignHCenter);
        v->addWidget(vLabel, 0, Qt::AlignHCenter);
        v->setAlignment(Qt::AlignCenter);
        return std::make_pair(box, vLabel);
    };

    auto abonneKpi = makeKpi("Abonnés", "0", "#ffd23f");
    auto totalKpi = makeKpi("Total facturé", "0", "#ffe066");
    auto impayeKpi = makeKpi("Impayés", "0", "#e53935");
    auto alerteKpi = makeKpi("Alertes", "0", "#ffb300"); // orange pour contraste
    kpiAbonnes = abonneKpi.second;
    kpiTotalFacture = totalKpi.second;
    kpiImpayes = impayeKpi.second;
    kpiAlertes = alerteKpi.second;
    kpiLayout->addWidget(abonneKpi.first);
    kpiLayout->addWidget(totalKpi.first);
    kpiLayout->addWidget(impayeKpi.first);
    kpiLayout->addWidget(alerteKpi.first);

    // Style harmonieux pour la barre KPI
    kpiWidget->setStyleSheet("background: #31463f; border: 1.5px solid #22332d; border-radius: 16px; margin: 0 0 12px 0;");
    kpiWidget->setMinimumHeight(60);
    dashboardPageLayout->addWidget(kpiWidget);    // KPI fixe
    qDebug() << "[DashboardWidget] kpiWidget ajouté au dashboardPageLayout, sizeHint:" << kpiWidget->sizeHint();
    dashboardPageLayout->addWidget(dashboardScroll, 1); // Le scroll ne concerne que le reste    // Les autres pages seront ajoutées ensuite
        contentStack->addWidget(dashboardPage); // index 0
    // Page 1 : Clients (nouveau widget dédié)
    ClientsWidget* clientsPage = new ClientsWidget;
    contentStack->addWidget(clientsPage); // index 1
    // Page 2 : Compteurs
    QWidget* compteursPage = new QWidget;
    QVBoxLayout* compteursLayout = new QVBoxLayout(compteursPage);
    compteursLayout->addWidget(new CompteurWidget(m_controller, compteursPage));
    contentStack->addWidget(compteursPage); // index 2
    // Page 3 : Paramètres
    QWidget* settingsPage = new QWidget;
    QVBoxLayout* settingsLayout = new QVBoxLayout(settingsPage);
    settingsLayout->addWidget(new QLabel("Page Paramètres (à implémenter)"));
    contentStack->addWidget(settingsPage); // index 3
    // Page 4 : Détail Alertes
    alertDetailPage = new QWidget;
    QVBoxLayout* alertDetailLayout = new QVBoxLayout(alertDetailPage);
    QLabel* alertDetailTitle = new QLabel("Détail des alertes");
    alertDetailTitle->setStyleSheet("color:#ffd23f; font-size:22px; font-weight:bold; margin:18px 0 12px 0;");
    alertDetailLayout->addWidget(alertDetailTitle, 0, Qt::AlignHCenter);
    alertDetailList = new QListWidget;
    alertDetailList->setStyleSheet("background: #31463f; border-radius: 16px; font-size: 15px; color: #e6e6e6; border: none; padding: 18px 18px; margin: 0px;");
    alertDetailList->setSelectionMode(QAbstractItemView::NoSelection);
    alertDetailList->setFocusPolicy(Qt::NoFocus);
    alertDetailLayout->addWidget(alertDetailList);
    QPushButton* retourBtn = new QPushButton("Retour au dashboard");
    retourBtn->setStyleSheet("background:#ffd23f; color:#3d554b; font-weight:bold; border-radius:10px; padding:8px 18px; margin:18px auto 0 auto;");
    retourBtn->setCursor(Qt::PointingHandCursor);
    alertDetailLayout->addWidget(retourBtn, 0, Qt::AlignHCenter);
    contentStack->addWidget(alertDetailPage); // index 4
    // Par défaut, afficher la page dashboard
    contentStack->setCurrentIndex(0);
    qDebug() << "[DashboardWidget] contentStack initialisé avec 5 pages.";

    // --- MENU LATERAL HARMONISÉ (même fond que la page d'authentification) ---
    QFrame* menuFrame = new QFrame;
    qDebug() << "[DashboardWidget] menuFrame créé, sizeHint:" << menuFrame->sizeHint();
    Q_ASSERT(menuFrame != nullptr);
    menuFrame->setFixedWidth(90);
    menuFrame->setStyleSheet("background: #39544c; border: none; border-radius: 16px;");
    QVBoxLayout* menuLayout = new QVBoxLayout(menuFrame);
    menuLayout->setContentsMargins(0, 0, 0, 0);
    menuLayout->setSpacing(0);

    qDebug() << "[DashboardWidget] Début logo";
    QLabel* logo = new QLabel;
    qDebug() << "[DashboardWidget] logo QLabel créé, sizeHint:" << logo->sizeHint();
    Q_ASSERT(logo != nullptr);
    QPixmap pixLogo("logo_watersn.png");
    if (!pixLogo.isNull()) {
        QPixmap roundedLogo(pixLogo.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        QPixmap mask(80, 80);
        mask.fill(Qt::transparent);
        QPainter painter(&mask);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(Qt::white);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(0, 0, 80, 80);
        roundedLogo.setMask(mask.createMaskFromColor(Qt::transparent));
        logo->setPixmap(roundedLogo);
        qDebug() << "[DashboardWidget] Logo pixmap appliqué";
    } else {
        logo->setText("🌊");
        qDebug() << "[DashboardWidget] Logo fallback emoji appliqué";
    }
    logo->setAlignment(Qt::AlignHCenter);
    logo->setStyleSheet("margin-top: 36px; margin-bottom: 36px; background: transparent; border-radius: 40px;");
    menuLayout->addWidget(logo);
    qDebug() << "[DashboardWidget] Logo ajouté au menuLayout";

    qDebug() << "[DashboardWidget] Début boutons navigation";

    // Boutons de navigation avec icônes (flat, blanc, effet actif)
    QList<QPushButton*> navButtons;
    QStringList navNames = {"Dashboard", "Clients", "Compteurs", "Paramètres"};
    // Icônes Material Design SVG (téléchargeables via le terminal)
    QStringList navIcons = {
        ":/icons/material/dashboard.svg",
        ":/icons/material/group.svg", // SVG de test
        ":/icons/material/speedometer.svg",
        ":/icons/material/settings.svg"
    };

    for (int i = 0; i < navNames.size(); ++i) {
        qDebug() << "[DashboardWidget] Création bouton navigation" << navNames[i] << "avec icône" << navIcons[i];
        QPushButton* btn = new QPushButton;
        qDebug() << "[DashboardWidget] btn créé, ptr =" << btn;
        qDebug() << "[DashboardWidget] Avant setIcon";
        btn->setIcon(QIcon(navIcons[i]));
        qDebug() << "[DashboardWidget] Après setIcon";
        qDebug() << "[DashboardWidget] Avant setIconSize";
        btn->setIconSize(QSize(30,30));
        qDebug() << "[DashboardWidget] Après setIconSize";
        qDebug() << "[DashboardWidget] Avant setToolTip";
        btn->setToolTip(navNames[i]);
        qDebug() << "[DashboardWidget] Après setToolTip";
        qDebug() << "[DashboardWidget] Avant setCheckable";
        btn->setCheckable(true);
        qDebug() << "[DashboardWidget] Après setCheckable";
        qDebug() << "[DashboardWidget] Avant setStyleSheet";
        btn->setStyleSheet("QPushButton { background: transparent; border: none; margin: 14px 0; color: #fff; border-radius: 12px; } QPushButton:checked { background: #ffd23f; color: #3d554b; border-radius: 12px; } QPushButton:hover { background: #ffe066; color: #3d554b; border-radius: 12px; }");
        qDebug() << "[DashboardWidget] Après setStyleSheet";
        qDebug() << "[DashboardWidget] Avant menuLayout->addWidget";
        menuLayout->addWidget(btn);
        qDebug() << "[DashboardWidget] Après menuLayout->addWidget";
        navButtons << btn;
        qDebug() << "[DashboardWidget] Bouton navigation ajouté au menuLayout";
    }
    qDebug() << "[DashboardWidget] Fin boutons navigation";
    qDebug() << "[DashboardWidget] navButtons.size() =" << navButtons.size();
    for (int i = 0; i < navButtons.size(); ++i) {
        qDebug() << "[DashboardWidget] navButtons[" << i << "] ptr =" << navButtons[i];
    }
    QButtonGroup* navGroup = new QButtonGroup(this);
    qDebug() << "[DashboardWidget] QButtonGroup créé, ptr =" << navGroup;
    Q_ASSERT(navGroup != nullptr);
    navGroup->setExclusive(true);
    for (int i = 0; i < navButtons.size(); ++i) {
        qDebug() << "[DashboardWidget] Ajout navButton au navGroup :" << navButtons[i];
        navGroup->addButton(navButtons[i], i);
    }
    if (!navButtons.isEmpty()) {
        qDebug() << "[DashboardWidget] navButtons[0] setChecked(true) - AVANT";
        try {
            navButtons[0]->setChecked(true);
            qDebug() << "[DashboardWidget] navButtons[0] setChecked(true) - APRES";
        } catch (...) {
            qCritical() << "[DashboardWidget] EXCEPTION attrapée lors de setChecked(true) !";
        }
    } else {
        qWarning() << "[DashboardWidget] navButtons est vide, impossible de setChecked sur le premier bouton !";
    }
    // Connexion navigation : chaque bouton du menu latéral affiche la page correspondante dans contentStack
    connect(navGroup, QOverload<int>::of(&QButtonGroup::idClicked), contentStack, &QStackedWidget::setCurrentIndex);
    menuLayout->addStretch();



    // Card centrale harmonisée (fond blanc, arrondi, ombre) - placée dans la page dashboard
    qDebug() << "[DashboardWidget] Création cardFrame (dashboardPage)";
    QFrame* cardFrame = new QFrame;
    qDebug() << "[DashboardWidget] cardFrame créé, sizeHint:" << cardFrame->sizeHint();
    Q_ASSERT(cardFrame != nullptr);
    cardFrame->setObjectName("cardFrame");
    cardFrame->setStyleSheet("background: #39544c; border-radius: 18px; border: none; box-shadow: 0 4px 24px rgba(0,0,0,0.10);");
    cardFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect;
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 6);
    shadow->setColor(QColor(0,0,0,40));
    cardFrame->setGraphicsEffect(shadow);

    QVBoxLayout* cardLayout = new QVBoxLayout(cardFrame);
    qDebug() << "[DashboardWidget] cardLayout créé, cardFrame size:" << cardFrame->size();
    Q_ASSERT(cardLayout != nullptr);
    qDebug() << "[DashboardWidget] Layout cardFrame prêt";
    cardLayout->setContentsMargins(32, 32, 32, 32);
    cardLayout->setSpacing(24);
    // Ajout des widgets dashboard dans la page dashboard (créés plus haut)

    
    dashboardLayout->addWidget(cardFrame, 1);


    // Section KPI (4 cards colorées)
    QWidget* kpiRow = new QWidget;
    // ...existing code...
    // Section KPI (4 cards colorées)
    // (Suppression des redéfinitions, ne déclarer chaque variable/struct qu'une seule fois)
    // ...existing code...

    // Section alertes
    qDebug() << "[DashboardWidget] Création alertListWidget";
    alertListWidget = new QListWidget;
    qDebug() << "[DashboardWidget] alertListWidget créé, sizeHint:" << alertListWidget->sizeHint();
    Q_ASSERT(alertListWidget != nullptr);
    alertListWidget->setStyleSheet("background: #31463f; border-radius: 16px; font-size: 15px; color: #e6e6e6; border: none; padding: 18px 18px; margin: 0px;");
    alertListWidget->setMinimumHeight(160);
    alertListWidget->setMaximumHeight(240);
    alertListWidget->setSelectionMode(QAbstractItemView::NoSelection);
    alertListWidget->setFocusPolicy(Qt::NoFocus);
    // Conteneur d'alerte aligné sur la largeur des graphiques, fond #39544c autour
    QWidget* alertContainer = new QWidget;
    alertContainer->setStyleSheet("background: #39544c; border-radius: 18px; border: none; margin: 0; padding: 0;");
    QVBoxLayout* alertContainerVLayout = new QVBoxLayout(alertContainer);
    alertContainerVLayout->setContentsMargins(0,0,0,0);
    alertContainerVLayout->setSpacing(0);
    // Ligne titre centré + bouton œil à droite (layout corrigé)
    QHBoxLayout* alertHeaderLayout = new QHBoxLayout;
    alertHeaderLayout->setContentsMargins(0, 0, 0, 0);
    alertHeaderLayout->setSpacing(0);
    QLabel* alertTitle = new QLabel("Alertes récentes");
    alertTitle->setAlignment(Qt::AlignCenter);
    alertTitle->setStyleSheet("color: #b7e0c0; font-size: 18px; font-weight: bold; font-family: 'Segoe UI', 'Arial', sans-serif; letter-spacing: 1px; padding: 12px 0;");
    alertHeaderLayout->addWidget(alertTitle, 1, Qt::AlignVCenter);
    QPushButton* voirToutBtn = new QPushButton;
    voirToutBtn->setIcon(QIcon(":/icons/material/visibility.svg"));
    voirToutBtn->setIconSize(QSize(26,26));
    voirToutBtn->setToolTip("Voir toutes les alertes");
    voirToutBtn->setCursor(Qt::PointingHandCursor);
    voirToutBtn->setFixedSize(38,38);
    voirToutBtn->setStyleSheet("QPushButton { background: transparent; border: none; margin: 0 8px; } QPushButton:hover { background: #22332d; border-radius: 8px; }");
    alertHeaderLayout->addWidget(voirToutBtn, 0, Qt::AlignRight|Qt::AlignVCenter);
    alertContainerVLayout->addLayout(alertHeaderLayout);
    alertContainerVLayout->addWidget(alertListWidget, 1);
    cardLayout->addWidget(alertContainer);
    qDebug() << "[DashboardWidget] alertListWidget ajouté à cardLayout";

    // Connexion du bouton 'voir tout' (œil) à l'ouverture de la page détail alertes
    connect(voirToutBtn, &QPushButton::clicked, this, [this]() {
        qDebug() << "[DashboardWidget] Bouton 'voir tout' cliqué, ouverture page détail alertes.";
        // Copier les alertes dans la liste détail
        alertDetailList->clear();
        for (int i = 0; i < alertListWidget->count(); ++i) {
            QListWidgetItem* src = alertListWidget->item(i);
            QListWidgetItem* item = new QListWidgetItem(src->icon(), src->text());
            item->setToolTip(src->toolTip());
            item->setForeground(src->foreground());
            alertDetailList->addItem(item);
        }
        contentStack->setCurrentIndex(4); // page détail alertes
    });
    // Bouton retour du détail alertes
    connect(retourBtn, &QPushButton::clicked, this, [this]() {
        contentStack->setCurrentIndex(0);
    });



    // PIE CHART - Consommation par collectivité (données fictives)
    QPieSeries* pieSeries = new QPieSeries();
    qDebug() << "[DashboardWidget] pieSeries créé";
    Q_ASSERT(pieSeries != nullptr);
    // Couleurs pastel variées pour chaque collectivité
    QList<QColor> pieColors = {QColor("#42a5f5"), QColor("#66bb6a"), QColor("#ffa726"), QColor("#ab47bc")};
    pieSeries->append("Collectivité 1", 40);
    pieSeries->append("Collectivité 2", 30);
    pieSeries->append("Collectivité 3", 20);
    pieSeries->append("Collectivité 4", 10);
    for (int i = 0; i < pieSeries->slices().size(); ++i) {
        auto slice = pieSeries->slices().at(i);
        slice->setLabelVisible(true);
        slice->setColor(pieColors[i % pieColors.size()]);
        slice->setLabelPosition(QPieSlice::LabelOutside);
        QFont labelFont = slice->labelFont();
        labelFont.setPointSize(13);
        labelFont.setBold(true);
        slice->setLabelFont(labelFont);
        slice->setLabelColor(QColor("#e6e6e6"));
        slice->setPen(QPen(QColor("#22332d"), 2));
    }
    QChart* pieChart = new QChart();
    qDebug() << "[DashboardWidget] pieChart créé";
    Q_ASSERT(pieChart != nullptr);
    pieChart->addSeries(pieSeries);
    pieChart->setTitle("Consommation par collectivité");
    QFont chartTitleFont = pieChart->titleFont();
    chartTitleFont.setPointSize(18);
    chartTitleFont.setBold(true);
    pieChart->setTitleFont(chartTitleFont);
    pieChart->setTitleBrush(QBrush(QColor("#b7e0c0")));
    pieChart->setBackgroundBrush(QBrush(QColor("#31463f")));
    pieChart->setBackgroundVisible(true);
    pieChart->setPlotAreaBackgroundBrush(QBrush(QColor("#31463f")));
    pieChart->setPlotAreaBackgroundVisible(true);
    pieChart->legend()->setLabelColor(QColor("#e6e6e6"));
    pieChart->legend()->setFont(QFont("Segoe UI", 12, QFont::Bold));
    pieChart->setAnimationOptions(QChart::SeriesAnimations);
    pieChart->legend()->setAlignment(Qt::AlignRight);
    QChartView* pieChartView = new QChartView(pieChart);
    qDebug() << "[DashboardWidget] pieChartView créé, sizeHint:" << pieChartView->sizeHint();
    Q_ASSERT(pieChartView != nullptr);
    pieChartView->setRenderHint(QPainter::Antialiasing);
    pieChartView->setMinimumHeight(420);
    pieChartView->setStyleSheet("background: #31463f; border-radius: 16px; padding: 8px; border: 1px solid #22332d;");

    // LINE CHART - Consommation par année (données fictives)
    QLineSeries* series2024 = new QLineSeries();
    qDebug() << "[DashboardWidget] series2024 créé";
    Q_ASSERT(series2024 != nullptr);
    series2024->setName("2024");
    QLineSeries* series2025 = new QLineSeries();
    qDebug() << "[DashboardWidget] series2025 créé";
    Q_ASSERT(series2025 != nullptr);
    series2025->setName("2025");
    // Mois fictifs (1 à 12)
    for (int m = 1; m <= 12; ++m) {
        series2024->append(m, 10 + QRandomGenerator::global()->bounded(30));
        series2025->append(m, 15 + QRandomGenerator::global()->bounded(25));
    }
    QChart* lineChart = new QChart();
    qDebug() << "[DashboardWidget] lineChart créé";
    Q_ASSERT(lineChart != nullptr);
    lineChart->addSeries(series2024);
    lineChart->addSeries(series2025);
    lineChart->setTitle("Consommation par année");
    lineChart->setTitleFont(chartTitleFont);
    lineChart->setTitleBrush(QBrush(QColor("#b7e0c0")));
    lineChart->setBackgroundBrush(QBrush(QColor("#31463f")));
    lineChart->setBackgroundVisible(true);
    lineChart->setPlotAreaBackgroundBrush(QBrush(QColor("#31463f")));
    lineChart->setPlotAreaBackgroundVisible(true);
    lineChart->legend()->setLabelColor(QColor("#e6e6e6"));
    // Les axes doivent être stylés après leur création effective

    // (Suppression des redéfinitions inutiles de moisLabels, axisX et axisY)
    QStringList moisLabels = {"Janv", "Fév", "Mars", "Avr", "Mai", "Juin", "Juil", "Août", "Sept", "Oct", "Nov", "Déc"};
    QCategoryAxis* axisX = new QCategoryAxis;
    axisX->setTitleText("Mois");
    axisX->setLabelsPosition(QCategoryAxis::AxisLabelsPositionOnValue);
    axisX->setLabelsAngle(-45);
    QFont axisFont("Segoe UI", 13, QFont::Bold);
    axisX->setLabelsFont(axisFont);
    axisX->setLabelsColor(QColor("#e6e6e6"));
    axisX->setTitleFont(QFont("Segoe UI", 14, QFont::Bold));
    axisX->setTitleBrush(QBrush(QColor("#b7e0c0")));
    for (int i = 0; i < moisLabels.size(); ++i) {
        axisX->append(moisLabels[i], i+1);
    }
    axisX->setRange(1, 12);
    lineChart->addAxis(axisX, Qt::AlignBottom);
    series2024->attachAxis(axisX);
    series2025->attachAxis(axisX);
    QValueAxis* axisY = new QValueAxis;
    axisY->setTitleText("Consommation");
    axisY->setLabelFormat("%.0f");
    axisY->setRange(0, 50);
    axisY->setLabelsFont(axisFont);
    axisY->setLabelsColor(QColor("#e6e6e6"));
    axisY->setTitleFont(QFont("Segoe UI", 14, QFont::Bold));
    axisY->setTitleBrush(QBrush(QColor("#b7e0c0")));
    lineChart->addAxis(axisY, Qt::AlignLeft);
    series2024->attachAxis(axisY);
    series2025->attachAxis(axisY);
    lineChart->legend()->setVisible(true);
    lineChart->legend()->setAlignment(Qt::AlignBottom);
    QChartView* lineChartView = new QChartView(lineChart);
    qDebug() << "[DashboardWidget] lineChartView créé, sizeHint:" << lineChartView->sizeHint();
    if (lineChartView->width() <= 0 || lineChartView->height() <= 0) {
        qCritical() << "[DashboardWidget] ERREUR: lineChartView a une taille nulle ou négative! width=" << lineChartView->width() << ", height=" << lineChartView->height();
    }
    Q_ASSERT(lineChartView != nullptr);
    lineChartView->setRenderHint(QPainter::Antialiasing);
    lineChartView->setMinimumHeight(420);
    lineChartView->setStyleSheet("background: #31463f; border-radius: 16px; padding: 8px; border: 1px solid #22332d;");

    // Ajout au layout principal de la card
    cardLayout->addWidget(pieChartView);
    cardLayout->addWidget(lineChartView);

    // Layout principal centré
    // ...existing code... (suppression de la redéclaration de mainLayout)

    // Connexions filtres (robustesse)
    connect(searchLineEdit, &QLineEdit::textChanged, this, &DashboardWidget::applyFilters);
    connect(collectiviteCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DashboardWidget::applyFilters);
    connect(periodeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DashboardWidget::applyFilters);
    connect(etatCompteurCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DashboardWidget::applyFilters);

    // (Suppression de l'appel prématuré à updateKpi() et updateAlerts())




    qDebug() << "[DashboardWidget] Création rightLayout";
    QVBoxLayout* rightLayout = new QVBoxLayout;
    Q_ASSERT(rightLayout != nullptr);
    rightLayout->setContentsMargins(0,0,0,0);
    rightLayout->setSpacing(0);
    // On place UNIQUEMENT le contentStack dans la partie droite
    rightLayout->addWidget(contentStack, 1);

    qDebug() << "[DashboardWidget] Création mainLayout";
    QHBoxLayout* mainLayout = new QHBoxLayout(this);
    Q_ASSERT(mainLayout != nullptr);
    mainLayout->setContentsMargins(0,0,0,0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(menuFrame);
    mainLayout->addLayout(rightLayout, 1);
    setLayout(mainLayout);
    // Appliquer le fond général du dashboard comme la page d'authentification
    this->setStyleSheet("background: #39544c;");
    qDebug() << "[DashboardWidget] Layout principal appliqué";


    // Suppression de la connexion à navigationMenu (plus utilisé)
    qDebug() << "[DashboardWidget] Fin du constructeur DashboardWidget";
    // Protection : n'appeler updateKpi/updateAlerts que si les widgets sont bien créés
    if (kpiAbonnes && kpiTotalFacture && kpiImpayes && kpiAlertes && alertListWidget) {
        updateKpi();
        updateAlerts();
    } else {
        qWarning() << "[DashboardWidget] Widgets KPI ou alertes non initialisés, updateKpi/updateAlerts non appelés.";
    }

    // Connexion des filtres à l'application dynamique
    connect(searchLineEdit, &QLineEdit::textChanged, this, &DashboardWidget::applyFilters);
    connect(collectiviteCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DashboardWidget::applyFilters);
    connect(periodeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DashboardWidget::applyFilters);
    connect(etatCompteurCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DashboardWidget::applyFilters);
    qDebug() << "[DashboardWidget] Fin CONSTRUCTEUR (dernier log avant return)";
}

void DashboardWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    QWidget* topLevel = window();
    if (topLevel && topLevel->isWindow()) {
        QSize winSize = topLevel->size();
        if (winSize.width() > 200 && winSize.height() > 200) {
            this->resize(winSize);
            this->setMinimumSize(winSize.width() * 0.7, winSize.height() * 0.7);
            qDebug() << "[DashboardWidget] showEvent: resize selon window, winSize=" << winSize;
        }
    }
}

   
// Définition correcte du destructeur (hors de toute accolade superflue)
DashboardWidget::~DashboardWidget() {
    qDebug() << "[DashboardWidget] Destructeur appelé";
}

void DashboardWidget::applyFilters()
{
    qDebug() << "[DashboardWidget] applyFilters()";
    // Récupérer les valeurs des filtres et les stocker dans des membres
    if (searchLineEdit) currentNom = searchLineEdit->text();
    if (collectiviteCombo) currentCollectivite = collectiviteCombo->currentText();
    if (periodeCombo) currentPeriode = periodeCombo->currentText();
    if (etatCompteurCombo) currentEtat = etatCompteurCombo->currentText();

    updateKpi();
    updateAlerts();
    // if (chartWidget) chartWidget->updateCharts(); // Suppression : QWidget n'a pas updateCharts()
}
void DashboardWidget::updateAlerts()
{
    qDebug() << "[DashboardWidget] updateAlerts()";
    // Liste statique d'alertes pour test visuel
    struct TestAlerte {
        QString type;
        QString message;
        QString date;
    };
    QList<TestAlerte> alertesTest = {
        {"consommation anormale", "Consommation anormale détectée", "2025-07-31"},
        {"impayé", "Facture impayée depuis 2 mois", "2025-07-30"},
        {"fuite", "Suspicion de fuite sur compteur", "2025-07-29"},
        {"coupure", "Coupure d'eau programmée demain", "2025-07-28"},
        {"info", "Nouvelle fonctionnalité disponible", "2025-07-27"}
    };
    if (alertListWidget) {
        alertListWidget->clear();
        for (const auto& alerte : alertesTest) {
            QString type = alerte.type.toLower();
            QString msg = alerte.message;
            QString date = alerte.date;
            QIcon icon;
            QColor textColor = QColor("#e6e6e6");
            if (type.contains("consommation")) {
                icon = QIcon(":/icons/material/warning.svg");
                textColor = QColor("#ffb300"); // orange
            } else if (type.contains("impay")) {
                icon = QIcon(":/icons/material/money_off.svg");
                textColor = QColor("#e53935"); // rouge
            } else if (type.contains("fuite")) {
                icon = QIcon(":/icons/material/water_drop.svg");
                textColor = QColor("#42a5f5"); // bleu
            } else if (type.contains("coupure")) {
                icon = QIcon(":/icons/material/block.svg");
                textColor = QColor("#757575"); // gris foncé
            } else {
                icon = QIcon(":/icons/material/info.svg");
                textColor = QColor("#66bb6a"); // vert
            }
            QListWidgetItem* item = new QListWidgetItem(icon, msg + " (" + date + ")");
            item->setToolTip(type);
            item->setForeground(textColor);
            alertListWidget->addItem(item);
        }
        if (alertesTest.isEmpty())
            alertListWidget->addItem("Aucune alerte récente.");
    } else {
        qDebug() << "[DashboardWidget] alertListWidget est nul !";
    }
    qDebug() << "[DashboardWidget] updateKpi()";
}
void DashboardWidget::updateKpi()
{
    qDebug() << "[DashboardWidget] updateKpi() - DEBUT";
    // Construction dynamique des filtres SQL
    QString clientWhere = "1=1";
    if (!currentNom.isEmpty())
        clientWhere += " AND nom LIKE '%" + currentNom.replace("'", "''") + "%'";
    // Pas de champ collectivite ni etat_compteur dans Client

    // Nombre de clients filtré
    qDebug() << "[DashboardWidget] Avant QSqlQuery q1 (COUNT Client)";
    QSqlQuery q1("SELECT COUNT(*) FROM Client WHERE " + clientWhere);
    qDebug() << "[DashboardWidget] Après QSqlQuery q1, isActive=" << q1.isActive();
    if (!q1.isActive()) qDebug() << "[DashboardWidget] Erreur SQL kpiAbonnes:" << q1.lastError().text();
    if (kpiAbonnes) {
        qDebug() << "[DashboardWidget] kpiAbonnes existe, avant next()";
        if (q1.next()) {
            qDebug() << "[DashboardWidget] q1.next() ok, valeur=" << q1.value(0);
            kpiAbonnes->setText(q1.value(0).toString());
        } else {
            qDebug() << "[DashboardWidget] q1.next() false";
            kpiAbonnes->setText("-");
        }
    } else {
        qDebug() << "[DashboardWidget] kpiAbonnes est nul !";
    }

    // Total facturé filtré (jointure avec Client pour appliquer les mêmes filtres)
    QString factureWhere = "1=1";
    if (!currentPeriode.isEmpty() && currentPeriode != "Tous")
        factureWhere += " AND YEAR(f.date_facture)='" + currentPeriode.replace("'", "''") + "'";
    // On applique aussi les filtres client
    QString totalFactureQuery = "SELECT SUM(f.soldeanterieur + f.consommation) FROM Facture f JOIN Client c ON f.idClient=c.idClient WHERE " + clientWhere + " AND " + factureWhere;
    qDebug() << "[DashboardWidget] Avant QSqlQuery q2 (SUM Facture):" << totalFactureQuery;
    QSqlQuery q2(totalFactureQuery);
    qDebug() << "[DashboardWidget] Après QSqlQuery q2, isActive=" << q2.isActive();
    if (!q2.isActive()) qDebug() << "[DashboardWidget] Erreur SQL kpiTotalFacture:" << q2.lastError().text();
    qDebug() << "[DashboardWidget] Avant test kpiTotalFacture";
    if (kpiTotalFacture) {
        qDebug() << "[DashboardWidget] kpiTotalFacture existe, avant next()";
        if (q2.next()) {
            qDebug() << "[DashboardWidget] q2.next() ok, valeur=" << q2.value(0);
            kpiTotalFacture->setText(q2.value(0).isNull() ? "0" : q2.value(0).toString());
            qDebug() << "[DashboardWidget] kpiTotalFacture->setText exécuté";
        } else {
            qDebug() << "[DashboardWidget] q2.next() false";
            kpiTotalFacture->setText("-");
            qDebug() << "[DashboardWidget] kpiTotalFacture->setText '-' exécuté";
        }
    } else {
        qDebug() << "[DashboardWidget] kpiTotalFacture est nul !";
    }
    qDebug() << "[DashboardWidget] Après bloc kpiTotalFacture";

    // Nombre d'impayés filtré (pas de champ paye, on suppose impayé = soldeanterieur > 0)
    QString impayeQuery = "SELECT COUNT(*) FROM Facture f JOIN Client c ON f.idClient=c.idClient WHERE (f.soldeanterieur > 0) AND " + clientWhere + " AND " + factureWhere;
    qDebug() << "[DashboardWidget] Avant QSqlQuery q3 (COUNT impayés):" << impayeQuery;
    QSqlQuery q3(impayeQuery);
    qDebug() << "[DashboardWidget] Après QSqlQuery q3, isActive=" << q3.isActive();
    if (!q3.isActive()) qDebug() << "[DashboardWidget] Erreur SQL kpiImpayes:" << q3.lastError().text();
    if (kpiImpayes) {
        qDebug() << "[DashboardWidget] kpiImpayes existe, avant next()";
        if (q3.next()) {
            qDebug() << "[DashboardWidget] q3.next() ok, valeur=" << q3.value(0);
            kpiImpayes->setText(q3.value(0).toString());
        } else {
            qDebug() << "[DashboardWidget] q3.next() false";
            kpiImpayes->setText("-");
        }
    } else {
        qDebug() << "[DashboardWidget] kpiImpayes est nul !";
    }

    // Nombre d'alertes filtré (exemple: type = consommation anormale)
    QString alerteQuery = "SELECT COUNT(*) FROM Alerte al JOIN Client c ON al.idClient=c.idClient WHERE al.type='consommation anormale' AND " + clientWhere;
    qDebug() << "[DashboardWidget] Avant QSqlQuery q4 (COUNT alertes):" << alerteQuery;
    QSqlQuery q4(alerteQuery);
    qDebug() << "[DashboardWidget] Après QSqlQuery q4, isActive=" << q4.isActive();
    if (!q4.isActive()) qDebug() << "[DashboardWidget] Erreur SQL kpiAlertes:" << q4.lastError().text();
    if (kpiAlertes) {
        qDebug() << "[DashboardWidget] kpiAlertes existe, avant next()";
        if (q4.next()) {
            qDebug() << "[DashboardWidget] q4.next() ok, valeur=" << q4.value(0);
            kpiAlertes->setText(q4.value(0).toString());
        } else {
            qDebug() << "[DashboardWidget] q4.next() false";
            kpiAlertes->setText("-");
        }
    } else {
        qDebug() << "[DashboardWidget] kpiAlertes est nul !";
    }
    qDebug() << "[DashboardWidget] updateKpi() - FIN";
}
