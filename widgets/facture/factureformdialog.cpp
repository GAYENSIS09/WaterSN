#include "widgets/factureformdialog.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>

FactureFormDialog::FactureFormDialog(QWidget* parent) : QDialog(parent) {
    // ... UI et logique formulaire ...
}

void FactureFormDialog::setFacture(const Facture& facture) {
    // ...
}

Facture FactureFormDialog::getFacture() const {
    // ...
    return Facture(0, "", 0, 0, 0);
}
