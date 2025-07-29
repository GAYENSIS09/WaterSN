#ifndef FACTUREFORMDIALOG_H
#define FACTUREFORMDIALOG_H

#include <QDialog>
#include "model/model.h"

class FactureFormDialog : public QDialog {
    Q_OBJECT
public:
    explicit FactureFormDialog(QWidget* parent = nullptr);
    void setFacture(const Facture& facture);
    Facture getFacture() const;
};

#endif // FACTUREFORMDIALOG_H
