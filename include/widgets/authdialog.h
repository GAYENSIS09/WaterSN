#ifndef AUTHDIALOG_H
#define AUTHDIALOG_H



#include <QDialog>

namespace Ui { class AuthDialog; }


class QLineEdit;
class QPushButton;
class QLabel;

class AuthDialog : public QDialog {
    Q_OBJECT
public:
    explicit AuthDialog(QWidget* parent = nullptr);
    ~AuthDialog();
    QString getEmail() const;
    QString getPassword() const;

signals:
    void forgotPasswordClicked();

private slots:
    void onLoginClicked();
    void onForgotPasswordClicked();

private:
    QLineEdit* emailEdit;
    QLineEdit* passwordEdit;
    QPushButton* loginButton;
    QPushButton* forgotButton;
    QLabel* errorLabel;
    void validateInputs();
    Ui::AuthDialog* ui;
};

#endif // AUTHDIALOG_H
