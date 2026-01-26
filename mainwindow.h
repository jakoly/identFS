#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <windows.h>
#include <objbase.h>
#include <string>
#include "libs/sqlite/sqlite3.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    sqlite3* db;   // DB-Pointer

public slots:
    void newProject();
    void openAddFilesWindow();
    void saveDefaultSettings();

private:
    Ui::MainWindow *ui;
    std::string guidToString(const GUID& guid);
};
#endif // MAINWINDOW_H
