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
    // Histogramme consommation par collectivité
    QBarSet *set = new QBarSet("Consommation");
    QStringList categories;
    QSqlQuery q("SELECT collectivite, SUM(montant) FROM Facture GROUP BY collectivite");
    while (q.next()) {
        categories << q.value(0).toString();
        *set << q.value(1).toDouble();
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
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(chartView);

    // Camembert consommation par période (exemple: année)
    QPieSeries *pieSeries = new QPieSeries();
    QSqlQuery q2("SELECT strftime('%Y', date), SUM(montant) FROM Facture GROUP BY strftime('%Y', date)");
    while (q2.next()) {
        pieSeries->append(q2.value(0).toString(), q2.value(1).toDouble());
    }
    QChart *pieChart = new QChart();
    pieChart->addSeries(pieSeries);
    pieChart->setTitle("Consommation par année");
    QChartView *pieView = new QChartView(pieChart);
    pieView->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(pieView);
}

void DashboardChartWidget::updateCharts()
{
    // À compléter si besoin de rafraîchir dynamiquement
}
