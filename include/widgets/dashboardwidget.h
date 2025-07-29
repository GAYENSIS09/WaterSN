#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>
#include <QLabel>
#include "controller/controller.h"

class DashboardWidget : public QWidget {
    Q_OBJECT
public:
    explicit DashboardWidget(Controller* controller, QWidget* parent = nullptr);
    void updateStats();
    void updateCharts();

private:
    Controller* m_controller;
    QChartView* m_chartView;
    QLabel* m_nbClientsLabel;
    QLabel* m_totalFactureLabel;
    QLabel* m_nbImpayeLabel;
};

#endif // DASHBOARDWIDGET_H
