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
    explicit AbonnementFormDialog(const QStringList& compteursDisponibles, QWidget* parent = nullptr);

    QString getCompteur() const;
    QDate getDateDebut() const;
    QString getStatut() const;

private slots:
    void onValiderClicked();

private:
    QComboBox* compteurCombo;
    QDateEdit* dateDebutEdit;
    QComboBox* statutCombo;
    QStringList compteursDisponibles_;
};

#endif // ABONNEMENTFORMDIALOG_H
