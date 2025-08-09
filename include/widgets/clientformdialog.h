#ifndef CLIENTFORMDIALOG_H
#define CLIENTFORMDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include "model/model.h"

class QLabel;

class ClientFormDialog : public QDialog {
    Q_OBJECT
public:
    explicit ClientFormDialog(QWidget* parent = nullptr);
    void setClient(const Client& client);
    Client getClient() const;
    
private slots:
    void onAddClicked();
    
private:
    QLineEdit* nomLineEdit;
    QLineEdit* prenomLineEdit;
    QLineEdit* adresseLineEdit;
    QLineEdit* telephoneLineEdit;
    QLabel* errorLabel;
    
    void showError(const QString& message);
};

#endif // CLIENTFORMDIALOG_H
