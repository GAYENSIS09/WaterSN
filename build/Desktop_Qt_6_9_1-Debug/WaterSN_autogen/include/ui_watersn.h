/********************************************************************************
** Form generated from reading UI file 'watersn.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WATERSN_H
#define UI_WATERSN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_WaterSN
{
public:
    QWidget *centralwidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *WaterSN)
    {
        if (WaterSN->objectName().isEmpty())
            WaterSN->setObjectName("WaterSN");
        WaterSN->resize(800, 600);
        centralwidget = new QWidget(WaterSN);
        centralwidget->setObjectName("centralwidget");
        WaterSN->setCentralWidget(centralwidget);
        menubar = new QMenuBar(WaterSN);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 27));
        WaterSN->setMenuBar(menubar);
        statusbar = new QStatusBar(WaterSN);
        statusbar->setObjectName("statusbar");
        WaterSN->setStatusBar(statusbar);

        retranslateUi(WaterSN);

        QMetaObject::connectSlotsByName(WaterSN);
    } // setupUi

    void retranslateUi(QMainWindow *WaterSN)
    {
        WaterSN->setWindowTitle(QCoreApplication::translate("WaterSN", "WaterSN", nullptr));
    } // retranslateUi

};

namespace Ui {
    class WaterSN: public Ui_WaterSN {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WATERSN_H
