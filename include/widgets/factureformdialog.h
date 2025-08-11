#ifndef FACTUREFORMDIALOG_H
#define FACTUREFORMDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include "model/model.h"


#include <QComboBox>

class FactureFormDialog : public QDialog {
    Q_OBJECT
public:
    explicit FactureFormDialog(const QStringList& compteursActifs, QWidget* parent = nullptr);
    void setFacture(const Facture& facture);
    Facture getFacture(int idClient) const;

private:
    QComboBox* numCompteurCombo;
    QLineEdit* soldeAnterieurEdit;
    QLineEdit* consommationEdit;
};

#endif // FACTUREFORMDIALOG_H
