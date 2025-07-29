#include "widgets/facturewidget.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QDate>
#include <QFile>
#include <QTextStream>
#include "controller/controller.h"

FactureWidget::FactureWidget(Controller* controller, QWidget* parent)
    : QWidget(parent), m_controller(controller) {
    // ...existing code...
}

// ...existing code for FactureWidget methods...
