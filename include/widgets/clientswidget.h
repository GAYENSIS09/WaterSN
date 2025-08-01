
#pragma once
#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QTableView>
#include <QPushButton>
#include <QScrollArea>

class ClientsWidget : public QWidget {
    Q_OBJECT
public:
    explicit ClientsWidget(QWidget* parent = nullptr);
protected:
    bool eventFilter(QObject* obj, QEvent* event) override;

private:
    QLineEdit* searchLineEdit = nullptr;
    QComboBox* collectiviteCombo = nullptr;
    QComboBox* consommationCombo = nullptr;
    QTableView* clientsTable = nullptr;
    QPushButton* addClientBtn = nullptr;
    QScrollArea* scrollArea = nullptr;
};