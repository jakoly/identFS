/********************************************************************************
** Form generated from reading UI file 'addfiles.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ADDFILES_H
#define UI_ADDFILES_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QPushButton>

QT_BEGIN_NAMESPACE

class Ui_addFiles
{
public:
    QLabel *label;
    QLabel *label_2;
    QPushButton *pushButtonAddFiles;
    QListWidget *listWidgetFiles;
    QPushButton *pushButtonOk;
    QPushButton *pushButtonCancel;

    void setupUi(QDialog *addFiles)
    {
        if (addFiles->objectName().isEmpty())
            addFiles->setObjectName("addFiles");
        addFiles->resize(633, 451);
        label = new QLabel(addFiles);
        label->setObjectName("label");
        label->setGeometry(QRect(30, 20, 431, 51));
        QFont font;
        font.setPointSize(30);
        label->setFont(font);
        label_2 = new QLabel(addFiles);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(30, 70, 551, 61));
        label_2->setTextFormat(Qt::TextFormat::AutoText);
        label_2->setScaledContents(false);
        label_2->setWordWrap(true);
        label_2->setMargin(1);
        label_2->setOpenExternalLinks(true);
        pushButtonAddFiles = new QPushButton(addFiles);
        pushButtonAddFiles->setObjectName("pushButtonAddFiles");
        pushButtonAddFiles->setGeometry(QRect(60, 140, 211, 61));
        listWidgetFiles = new QListWidget(addFiles);
        listWidgetFiles->setObjectName("listWidgetFiles");
        listWidgetFiles->setGeometry(QRect(330, 140, 256, 221));
        pushButtonOk = new QPushButton(addFiles);
        pushButtonOk->setObjectName("pushButtonOk");
        pushButtonOk->setGeometry(QRect(450, 400, 75, 24));
        pushButtonCancel = new QPushButton(addFiles);
        pushButtonCancel->setObjectName("pushButtonCancel");
        pushButtonCancel->setGeometry(QRect(530, 400, 75, 24));

        retranslateUi(addFiles);

        QMetaObject::connectSlotsByName(addFiles);
    } // setupUi

    void retranslateUi(QDialog *addFiles)
    {
        addFiles->setWindowTitle(QCoreApplication::translate("addFiles", "Ident FS - Dateien hinzuf\303\274gen", nullptr));
        label->setText(QCoreApplication::translate("addFiles", "Hinzuf\303\274gen von Dateien", nullptr));
        label_2->setText(QCoreApplication::translate("addFiles", "<html><head/><body><p>Durch das Hinzuf\303\274gen von Dateien, k\303\266nnen sie diese zu ihren Projekten hinzuf\303\274gen. Die Dateien werden au\303\237erdem durch eineultra-komprimierte<span style=\" font-weight:700;\"> BackUp-Version </span>gesichert, ega, ob sie die Dateien l\303\266schen. Die BackUp-Versionen wird weiterhin f\303\274r den gew\303\244hlten Zeitraum aufbewahrt.</p></body></html>", nullptr));
        pushButtonAddFiles->setText(QCoreApplication::translate("addFiles", "Dateien ausw\303\244hlen", nullptr));
        pushButtonOk->setText(QCoreApplication::translate("addFiles", "OK", nullptr));
        pushButtonCancel->setText(QCoreApplication::translate("addFiles", "Cancel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class addFiles: public Ui_addFiles {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ADDFILES_H
