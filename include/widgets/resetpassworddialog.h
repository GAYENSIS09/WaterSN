#ifndef RESETPASSWORDDIALOG_H
#define RESETPASSWORDDIALOG_H

#include <QDialog>

class QLineEdit;
class QPushButton;
class QLabel;
class QProgressDialog;

class ResetPasswordDialog : public QDialog {
    Q_OBJECT
public:
    explicit ResetPasswordDialog(QWidget* parent = nullptr);

signals:
    void passwordResetSuccess();

private slots:
    void onSendClicked();
    void onResendClicked();
    void onEmailCheckFinished(bool found);
    void onSendMailFinished(bool success);

private:
    QLineEdit* emailEdit;
    QPushButton* sendButton;
    QPushButton* resendButton;
    QLabel* infoLabel;
    QLabel* errorLabel;
    QProgressDialog* progressDialog;
    void validateInput();
    void setLoading(bool loading);
    void resetUI();
};

#endif // RESETPASSWORDDIALOG_H
