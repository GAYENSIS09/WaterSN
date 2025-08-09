#ifndef PRELEVEMENTFORMDIALOG_H
#define PRELEVEMENTFORMDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QLabel>
#include <QPushButton>

class PrelevementFormDialog : public QDialog {
    Q_OBJECT
public:
    explicit PrelevementFormDialog(QWidget* parent = nullptr);
    QString getCompteur() const;
    double getAncienIndex() const;
    double getNouveauIndex() const;
    QDate getDate() const;
    void setCompteurs(const QStringList& compteurs);
    void setAncienIndex(double index);
private:
    QComboBox* compteurCombo;
    QLineEdit* ancienIndexEdit;
    QLineEdit* nouveauIndexEdit;
    QDateEdit* dateEdit;
    QPushButton* okButton;
    QPushButton* cancelButton;
    void setupUI();
    void connectSignals();
};

#endif // PRELEVEMENTFORMDIALOG_H
