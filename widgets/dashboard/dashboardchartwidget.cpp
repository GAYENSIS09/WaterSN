#include "widgets/dashboardchartwidget.h"
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QSqlQuery>
#include <QVBoxLayout>


DashboardChartWidget::DashboardChartWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(18);
    layout->setContentsMargins(12, 12, 12, 12);

    // --- Histogramme consommation par collectivité ---
    QBarSet *set = new QBarSet("Consommation");
    QStringList categories;
    QSqlQuery q("SELECT collectivite, SUM(montant) FROM Facture GROUP BY collectivite");
    bool hasData = false;
    while (q.next()) {
        categories << q.value(0).toString();
        *set << q.value(1).toDouble();
        hasData = true;
    }
    // Si pas de données, simuler
    if (!hasData) {
        categories << "Collectivité A" << "Collectivité B" << "Collectivité C";
        *set << 1200000 << 900000 << 1400000;
    }
    QBarSeries *series = new QBarSeries();
    series->append(set);
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Consommation par collectivité");
    QBarCategoryAxis *axis = new QBarCategoryAxis();
    axis->append(categories);
    chart->createDefaultAxes();
    chart->setAxisX(axis, series);
    chart->legend()->setVisible(false);
    chart->setBackgroundBrush(QBrush(QColor("#fffbe8")));
    chart->setBackgroundRoundness(18);
    chart->setTitleFont(QFont("Montserrat", 13, QFont::Bold));
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setStyleSheet("background: #fffbe8; border-radius: 18px; border: 1.5px solid #e0e0e0; box-shadow: 0px 8px 24px rgba(0,0,0,0.08);");
    layout->addWidget(chartView);

    // --- Camembert consommation par période (année) ---
    QPieSeries *pieSeries = new QPieSeries();
    QSqlQuery q2("SELECT strftime('%Y', date), SUM(montant) FROM Facture GROUP BY strftime('%Y', date)");
    bool hasPie = false;
    while (q2.next()) {
        pieSeries->append(q2.value(0).toString(), q2.value(1).toDouble());
        hasPie = true;
    }
    if (!hasPie) {
        pieSeries->append("2024", 1800000);
        pieSeries->append("2025", 1600000);
    }
    for (auto slice : pieSeries->slices()) {
        slice->setLabelVisible(true);
        slice->setLabelFont(QFont("Montserrat", 10, QFont::Bold));
    }
    QChart *pieChart = new QChart();
    pieChart->addSeries(pieSeries);
    pieChart->setTitle("Consommation par année");
    pieChart->setTitleFont(QFont("Montserrat", 13, QFont::Bold));
    pieChart->setBackgroundBrush(QBrush(QColor("#fffbe8")));
    pieChart->setBackgroundRoundness(18);
    QChartView *pieView = new QChartView(pieChart);
    pieView->setRenderHint(QPainter::Antialiasing);
    pieView->setStyleSheet("background: #fffbe8; border-radius: 18px; border: 1.5px solid #e0e0e0; box-shadow: 0px 8px 24px rgba(0,0,0,0.08);");
    layout->addWidget(pieView);
}

void DashboardChartWidget::updateCharts()
{
    // À compléter si besoin de rafraîchir dynamiquement
}
