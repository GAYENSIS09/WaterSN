#ifndef PRELEVEMENTWIDGET_H
#define PRELEVEMENTWIDGET_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>
#include <QPushButton>
#include <QLineEdit>

class PrelevementWidget : public QWidget {
    Q_OBJECT
public:
    explicit PrelevementWidget(QWidget* parent = nullptr);

private:
    QTableView* tableView;
    QStandardItemModel* model;
    QPushButton* addButton;
    QLineEdit* filterEdit;
    // Membres du formulaire en bas
    QComboBox* compteurCombo;
    QLineEdit* ancienIndexEdit;
    QLineEdit* nouvelIndexEdit;
    QDateEdit* dateEdit;
    QPushButton* ajouterBtn;
    void setupUI();
    void loadPrelevements();
    void addPrelevement();
    void filterPrelevements(const QString& text);
};

#endif // PRELEVEMENTWIDGET_H
