#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include <QLineEdit>
#include <QComboBox>
#include <QLabel>
#include "controller/controller.h"

#include "widgets/dashboardchartwidget.h"



class DashboardWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DashboardWidget(Controller* controller, QWidget *parent = nullptr);
    ~DashboardWidget() override;
    void applyFilters();

private:
    Controller* m_controller;
    QListWidget* navigationMenu;
    QStackedWidget* contentStack;
    QWidget* kpiWidget; // Section KPI
    QLabel* kpiAbonnes;
    QLabel* kpiTotalFacture;
    QLabel* kpiImpayes;
    QLabel* kpiAlertes;
    DashboardChartWidget* chartWidget; // Section graphique
    QListWidget* alertListWidget; // Section alertes
    QLineEdit* searchLineEdit;
    QComboBox* collectiviteCombo;
    QComboBox* periodeCombo;
    QComboBox* etatCompteurCombo;
    QString currentNom, currentCollectivite, currentPeriode, currentEtat;
    void updateAlerts();
    void updateKpi();
};

#endif // DASHBOARDWIDGET_H
