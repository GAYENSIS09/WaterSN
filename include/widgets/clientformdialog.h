#ifndef CLIENTFORMDIALOG_H
#define CLIENTFORMDIALOG_H

#include <QDialog>
#include "model/model.h"

class ClientFormDialog : public QDialog {
    Q_OBJECT
public:
    explicit ClientFormDialog(QWidget* parent = nullptr);
    void setClient(const Client& client);
    Client getClient() const;
};

#endif // CLIENTFORMDIALOG_H
