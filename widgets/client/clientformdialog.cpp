#include "widgets/clientformdialog.h"
#include <QFormLayout>
#include <QLineEdit>
#include <QDialogButtonBox>

ClientFormDialog::ClientFormDialog(QWidget* parent) : QDialog(parent) {
    // ... UI et logique formulaire ...
}

void ClientFormDialog::setClient(const Client& client) {
    // ...
}

Client ClientFormDialog::getClient() const {
    // ...
    return Client(0, "", "", "", "");
}
