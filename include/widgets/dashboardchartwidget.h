
#ifndef DASHBOARDCHARTWIDGET_H
#define DASHBOARDCHARTWIDGET_H

#include <QWidget>

class DashboardChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DashboardChartWidget(QWidget *parent = nullptr);
    void updateCharts();
};

#endif // DASHBOARDCHARTWIDGET_H
