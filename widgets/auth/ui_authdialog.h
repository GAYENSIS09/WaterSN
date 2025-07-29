// Fichier généré automatiquement par uic pour AuthDialog
#ifndef UI_AUTHDIALOG_H
#define UI_AUTHDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AuthDialog {
public:
    QWidget *backgroundWidget;
    QFrame *cardFrame;
    QLabel *logoLabel;
    QLabel *titleLabel;
    QLineEdit *emailEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;

    void setupUi(QDialog *AuthDialog) {
        if (AuthDialog->objectName().isEmpty())
            AuthDialog->setObjectName(QString::fromUtf8("AuthDialog"));
        AuthDialog->resize(420, 320);
        backgroundWidget = new QWidget(AuthDialog);
        backgroundWidget->setObjectName(QString::fromUtf8("backgroundWidget"));
        backgroundWidget->setGeometry(QRect(0, 0, 420, 320));
        cardFrame = new QFrame(backgroundWidget);
        cardFrame->setObjectName(QString::fromUtf8("cardFrame"));
        cardFrame->setGeometry(QRect(110, 60, 200, 200));
        cardFrame->setFrameShape(QFrame::StyledPanel);
        cardFrame->setFrameShadow(QFrame::Raised);
        logoLabel = new QLabel(cardFrame);
        logoLabel->setObjectName(QString::fromUtf8("logoLabel"));
        logoLabel->setGeometry(QRect(80, 10, 40, 40));
        titleLabel = new QLabel(cardFrame);
        titleLabel->setObjectName(QString::fromUtf8("titleLabel"));
        titleLabel->setGeometry(QRect(20, 50, 160, 30));
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setText("Login");
        emailEdit = new QLineEdit(cardFrame);
        emailEdit->setObjectName(QString::fromUtf8("emailEdit"));
        emailEdit->setGeometry(QRect(20, 90, 160, 28));
        emailEdit->setPlaceholderText("Enter email");
        passwordEdit = new QLineEdit(cardFrame);
        passwordEdit->setObjectName(QString::fromUtf8("passwordEdit"));
        passwordEdit->setGeometry(QRect(20, 125, 160, 28));
        passwordEdit->setEchoMode(QLineEdit::Password);
        passwordEdit->setPlaceholderText("Password");
        loginButton = new QPushButton(cardFrame);
        loginButton->setObjectName(QString::fromUtf8("loginButton"));
        loginButton->setGeometry(QRect(20, 165, 160, 32));
        loginButton->setText("Log In");
    }
};

namespace Ui {
    class AuthDialog: public Ui_AuthDialog {};
}

QT_END_NAMESPACE

#endif // UI_AUTHDIALOG_H
