#ifndef FACTUREWIDGET_H
#define FACTUREWIDGET_H

#include <QWidget>
#include <QTableView>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QStandardItemModel>
#include "controller/controller.h"

class FactureWidget : public QWidget {
    Q_OBJECT
public:
    explicit FactureWidget(Controller* controller, QWidget* parent = nullptr);

private:
    Controller* m_controller;
    QTableView* m_tableView;
    QLineEdit* m_searchEdit;
    QComboBox* m_filterEtat;
    QPushButton* m_addButton;
    QPushButton* m_exportPdfButton;
    QPushButton* m_exportExcelButton;
    QLabel* m_titleLabel;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_actionLayout;
};

#endif // FACTUREWIDGET_H
