#include "widgets/dashboardwidget.h"
#include <QVBoxLayout>
#include <QtCharts/QChart>

DashboardWidget::DashboardWidget(Controller* controller, QWidget* parent)
    : QWidget(parent), m_controller(controller) {
    m_chartView = new QChartView(this);
    m_nbClientsLabel = new QLabel("Nombre d'abonnés : 0", this);
    m_totalFactureLabel = new QLabel("Total facturé : 0", this);
    m_nbImpayeLabel = new QLabel("Nombre d'impayés : 0", this);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_nbClientsLabel);
    layout->addWidget(m_totalFactureLabel);
    layout->addWidget(m_nbImpayeLabel);
    layout->addWidget(m_chartView);
    setLayout(layout);

    updateStats();
    updateCharts();
}

void DashboardWidget::updateStats() {
    m_nbClientsLabel->setText(QString("Nombre d'abonnés : %1").arg(m_controller->clients.size()));
    double total = 0;
    int impayes = 0;
    for (const auto& f : m_controller->facturations) {
        total += f.mensualite;
        if (f.date_paiement.isNull()) impayes++;
    }
    m_totalFactureLabel->setText(QString("Total facturé : %1").arg(total));
    m_nbImpayeLabel->setText(QString("Nombre d'impayés : %1").arg(impayes));
}

void DashboardWidget::updateCharts() {
    QBarSeries* series = new QBarSeries();
    QBarSet* set = new QBarSet("Consommation");
    QStringList categories;
    for (const auto& client : m_controller->clients) {
        double totalCons = 0;
        for (const auto& f : m_controller->factures) {
            if (f.idClient == client.idClient) totalCons += f.consommation;
        }
        *set << totalCons;
        categories << client.nom;
    }
    series->append(set);
    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Consommation par abonné");
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    QValueAxis* axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);
    m_chartView->setChart(chart);
}
