/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QWidget *layoutWidget;
    QGridLayout *Project_Input;
    QPushButton *pushButtonCreateProject;
    QLabel *label_2;
    QLineEdit *lineEditProjectName;
    QListWidget *projectList;
    QPushButton *pushButtonAddFiles;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1920, 1080);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Ignored, QSizePolicy::Policy::Ignored);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        layoutWidget = new QWidget(centralwidget);
        layoutWidget->setObjectName("layoutWidget");
        layoutWidget->setGeometry(QRect(540, 170, 281, 91));
        Project_Input = new QGridLayout(layoutWidget);
        Project_Input->setObjectName("Project_Input");
        Project_Input->setSizeConstraint(QLayout::SizeConstraint::SetDefaultConstraint);
        Project_Input->setContentsMargins(0, 0, 0, 0);
        pushButtonCreateProject = new QPushButton(layoutWidget);
        pushButtonCreateProject->setObjectName("pushButtonCreateProject");
        QFont font;
        font.setPointSize(15);
        pushButtonCreateProject->setFont(font);

        Project_Input->addWidget(pushButtonCreateProject, 1, 1, 1, 1);

        label_2 = new QLabel(layoutWidget);
        label_2->setObjectName("label_2");
        QFont font1;
        font1.setPointSize(20);
        label_2->setFont(font1);

        Project_Input->addWidget(label_2, 0, 0, 1, 1);

        lineEditProjectName = new QLineEdit(layoutWidget);
        lineEditProjectName->setObjectName("lineEditProjectName");
        lineEditProjectName->setFont(font);

        Project_Input->addWidget(lineEditProjectName, 1, 0, 1, 1);

        projectList = new QListWidget(centralwidget);
        projectList->setObjectName("projectList");
        projectList->setGeometry(QRect(320, 320, 256, 192));
        pushButtonAddFiles = new QPushButton(centralwidget);
        pushButtonAddFiles->setObjectName("pushButtonAddFiles");
        pushButtonAddFiles->setGeometry(QRect(130, 170, 261, 91));
        pushButtonAddFiles->setFont(font);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1920, 22));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Ident FS", nullptr));
        pushButtonCreateProject->setText(QCoreApplication::translate("MainWindow", "Eingeben", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Projektname", nullptr));
        lineEditProjectName->setInputMask(QString());
        pushButtonAddFiles->setText(QCoreApplication::translate("MainWindow", "Dateien hinzuf\303\274gen", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
