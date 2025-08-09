#ifndef FACTURATIONFORMDIALOG_H
#define FACTURATIONFORMDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QDateEdit>
#include <QLabel>
#include <QPushButton>

class FacturationFormDialog : public QDialog {
    Q_OBJECT
public:
    explicit FacturationFormDialog(QWidget* parent = nullptr);
    QDate getDatePaiement() const;
    QString getNumCompteur() const;
    int getIdFacture() const;
    int getIdClient() const;
    double getMensualite() const;
    void setCompteurs(const QStringList& compteurs);
    void setFactures(const QList<QPair<int, QString>>& factures); // id, description
    void setClients(const QList<QPair<int, QString>>& clients); // id, nom
private:
    QDateEdit* dateEdit;
    QComboBox* compteurCombo;
    QComboBox* factureCombo;
    QComboBox* clientCombo;
    QLineEdit* mensualiteEdit;
    QPushButton* okButton;
    QPushButton* cancelButton;
    void setupUI();
    void connectSignals();
};

#endif // FACTURATIONFORMDIALOG_H
