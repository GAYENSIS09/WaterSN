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
    ~DashboardWidget();

    void applyFilters();
    void updateAlerts();
    void updateKpi();

protected:
    void showEvent(QShowEvent* event) override;

    // ... autres membres ...

private:
    Controller* m_controller = nullptr;
    QStackedWidget* contentStack = nullptr;
    QLineEdit* searchLineEdit = nullptr;
    QComboBox* collectiviteCombo = nullptr;
    QComboBox* periodeCombo = nullptr;
    QComboBox* etatCompteurCombo = nullptr;
    QLabel* kpiAbonnes = nullptr;
    QLabel* kpiTotalFacture = nullptr;
    QLabel* kpiImpayes = nullptr;
    QLabel* kpiAlertes = nullptr;
    QListWidget* alertListWidget = nullptr;
    QWidget* kpiWidget = nullptr;
    QString currentNom;
    QString currentCollectivite;
    QString currentPeriode;
    QString currentEtat;
    QWidget* chartWidget = nullptr;
    // Ajout pour la page détail alertes
    QWidget* alertDetailPage = nullptr;
    QListWidget* alertDetailList = nullptr;
    // ... autres membres ...
};

#endif // DASHBOARDWIDGET_H