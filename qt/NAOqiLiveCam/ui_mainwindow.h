/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Fri Mar 27 10:41:39 2015
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QPushButton>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QPlainTextEdit *console;
    QPushButton *connectButton;
    QWidget *widget;
    QLabel *cameraView;
    QPushButton *disconnectButton;
    QLineEdit *naoIp;
    QLabel *label;
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(629, 390);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        console = new QPlainTextEdit(centralWidget);
        console->setObjectName(QString::fromUtf8("console"));
        console->setGeometry(QRect(3, 35, 223, 213));
        connectButton = new QPushButton(centralWidget);
        connectButton->setObjectName(QString::fromUtf8("connectButton"));
        connectButton->setGeometry(QRect(344, -4, 94, 32));
        widget = new QWidget(centralWidget);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(236, 35, 340, 260));
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(widget->sizePolicy().hasHeightForWidth());
        widget->setSizePolicy(sizePolicy);
        widget->setMinimumSize(QSize(162, 120));
        cameraView = new QLabel(widget);
        cameraView->setObjectName(QString::fromUtf8("cameraView"));
        cameraView->setGeometry(QRect(10, 10, 320, 240));
        cameraView->setAlignment(Qt::AlignCenter);
        disconnectButton = new QPushButton(centralWidget);
        disconnectButton->setObjectName(QString::fromUtf8("disconnectButton"));
        disconnectButton->setGeometry(QRect(230, -4, 112, 32));
        naoIp = new QLineEdit(centralWidget);
        naoIp->setObjectName(QString::fromUtf8("naoIp"));
        naoIp->setGeometry(QRect(110, 0, 116, 21));
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(3, 0, 99, 21));
        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 629, 22));
        MainWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(MainWindow);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        MainWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        MainWindow->setStatusBar(statusBar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        connectButton->setText(QApplication::translate("MainWindow", "Connect", 0, QApplication::UnicodeUTF8));
        cameraView->setText(QApplication::translate("MainWindow", "Camera VIew", 0, QApplication::UnicodeUTF8));
        disconnectButton->setText(QApplication::translate("MainWindow", "Disconnect", 0, QApplication::UnicodeUTF8));
        naoIp->setText(QApplication::translate("MainWindow", "127.0.0.1", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "NAO IP address", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
