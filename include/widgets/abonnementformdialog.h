#ifndef ABONNEMENTFORMDIALOG_H
#define ABONNEMENTFORMDIALOG_H

#include <QDialog>
#include <QDate>
class QLineEdit;
class QComboBox;
class QDateEdit;

class AbonnementFormDialog : public QDialog {
    Q_OBJECT
public:
    explicit AbonnementFormDialog(QWidget* parent = nullptr);

    QString getCompteur() const;
    QDate getDateDebut() const;
    QString getStatut() const;

private slots:
    void onValiderClicked();

private:
    QLineEdit* compteurEdit;
    QDateEdit* dateDebutEdit;
    QComboBox* statutCombo;
};

#endif // ABONNEMENTFORMDIALOG_H
