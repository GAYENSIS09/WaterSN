/********************************************************************************
** Form generated from reading UI file 'authdialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_AUTHDIALOG_H
#define UI_AUTHDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>

QT_BEGIN_NAMESPACE

class Ui_AuthDialog
{
public:
    QVBoxLayout *verticalLayoutMain;
    QHBoxLayout *horizontalLayoutLogo;
    QSpacerItem *horizontalSpacerLogoLeft;
    QLabel *logoLabel;
    QSpacerItem *verticalSpacerTop;
    QHBoxLayout *horizontalLayoutCenter;
    QSpacerItem *horizontalSpacerLeft;
    QFrame *cardFrame;
    QVBoxLayout *verticalLayoutCard;
    QLabel *titleLabel;
    QSpacerItem *verticalSpacerTitle;
    QLabel *errorLabel;
    QSpacerItem *verticalSpacerError;
    QLineEdit *emailEdit;
    QLineEdit *passwordEdit;
    QPushButton *loginButton;
    QPushButton *forgotButton;
    QSpacerItem *horizontalSpacerRight;
    QSpacerItem *verticalSpacerBottom;

    void setupUi(QDialog *AuthDialog)
    {
        if (AuthDialog->objectName().isEmpty())
            AuthDialog->setObjectName("AuthDialog");
        AuthDialog->resize(420, 320);
        verticalLayoutMain = new QVBoxLayout(AuthDialog);
        verticalLayoutMain->setObjectName("verticalLayoutMain");
        horizontalLayoutLogo = new QHBoxLayout();
        horizontalLayoutLogo->setObjectName("horizontalLayoutLogo");
        horizontalSpacerLogoLeft = new QSpacerItem(20, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayoutLogo->addItem(horizontalSpacerLogoLeft);

        logoLabel = new QLabel(AuthDialog);
        logoLabel->setObjectName("logoLabel");
        logoLabel->setMinimumSize(QSize(64, 64));
        logoLabel->setMaximumSize(QSize(64, 64));
        logoLabel->setAlignment(Qt::AlignRight|Qt::AlignTop);
        logoLabel->setScaledContents(true);

        horizontalLayoutLogo->addWidget(logoLabel);


        verticalLayoutMain->addLayout(horizontalLayoutLogo);

        verticalSpacerTop = new QSpacerItem(20, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayoutMain->addItem(verticalSpacerTop);

        horizontalLayoutCenter = new QHBoxLayout();
        horizontalLayoutCenter->setObjectName("horizontalLayoutCenter");
        horizontalSpacerLeft = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayoutCenter->addItem(horizontalSpacerLeft);

        cardFrame = new QFrame(AuthDialog);
        cardFrame->setObjectName("cardFrame");
        cardFrame->setMinimumSize(QSize(340, 180));
        cardFrame->setFrameShape(QFrame::StyledPanel);
        cardFrame->setFrameShadow(QFrame::Raised);
        verticalLayoutCard = new QVBoxLayout(cardFrame);
        verticalLayoutCard->setObjectName("verticalLayoutCard");
        titleLabel = new QLabel(cardFrame);
        titleLabel->setObjectName("titleLabel");
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setMinimumHeight(36);

        verticalLayoutCard->addWidget(titleLabel);

        verticalSpacerTitle = new QSpacerItem(20, 8, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayoutCard->addItem(verticalSpacerTitle);

        errorLabel = new QLabel(cardFrame);
        errorLabel->setObjectName("errorLabel");
        errorLabel->setVisible(false);
        errorLabel->setAlignment(Qt::AlignCenter);

        verticalLayoutCard->addWidget(errorLabel);

        verticalSpacerError = new QSpacerItem(20, 4, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayoutCard->addItem(verticalSpacerError);

        emailEdit = new QLineEdit(cardFrame);
        emailEdit->setObjectName("emailEdit");

        verticalLayoutCard->addWidget(emailEdit);

        passwordEdit = new QLineEdit(cardFrame);
        passwordEdit->setObjectName("passwordEdit");
        passwordEdit->setEchoMode(QLineEdit::Password);

        verticalLayoutCard->addWidget(passwordEdit);

        loginButton = new QPushButton(cardFrame);
        loginButton->setObjectName("loginButton");

        verticalLayoutCard->addWidget(loginButton);

        forgotButton = new QPushButton(cardFrame);
        forgotButton->setObjectName("forgotButton");
        forgotButton->setFlat(true);

        verticalLayoutCard->addWidget(forgotButton);


        horizontalLayoutCenter->addWidget(cardFrame);

        horizontalSpacerRight = new QSpacerItem(40, 20, QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Minimum);

        horizontalLayoutCenter->addItem(horizontalSpacerRight);


        verticalLayoutMain->addLayout(horizontalLayoutCenter);

        verticalSpacerBottom = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayoutMain->addItem(verticalSpacerBottom);


        retranslateUi(AuthDialog);

        QMetaObject::connectSlotsByName(AuthDialog);
    } // setupUi

    void retranslateUi(QDialog *AuthDialog)
    {
        AuthDialog->setWindowTitle(QCoreApplication::translate("AuthDialog", "Connexion \303\240 WaterSN", nullptr));
        titleLabel->setText(QCoreApplication::translate("AuthDialog", "Login", nullptr));
        errorLabel->setText(QString());
        errorLabel->setStyleSheet(QCoreApplication::translate("AuthDialog", "color: red; font-size: 13px; font-weight: bold; padding: 2px 0 2px 0;", nullptr));
        emailEdit->setPlaceholderText(QCoreApplication::translate("AuthDialog", "Enter email", nullptr));
        passwordEdit->setPlaceholderText(QCoreApplication::translate("AuthDialog", "Password", nullptr));
        loginButton->setText(QCoreApplication::translate("AuthDialog", "Log In", nullptr));
        forgotButton->setText(QCoreApplication::translate("AuthDialog", "Mot de passe oubli\303\251 ?", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AuthDialog: public Ui_AuthDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_AUTHDIALOG_H
