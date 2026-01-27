#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <windows.h>
#include <objbase.h>
#include <string>
#include "libs/sqlite/sqlite3.h"
#include <QListWidgetItem>
#include <QFont>
#include <QIcon>
#include <QSettings>
#include <QDebug>

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

    QString projectName;
    QString projectUUID;
    QString projectCreated;
    QString projectModificated;
    QString projectVaultPath;

public slots:
    void newProject();
    void openAddFilesWindow();
    void openAddFilesToProject();
    void saveDefaultSettings();
    void updateProjectList();
    void onProjectItemClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    std::string guidToString(const GUID& guid);
};
#endif // MAINWINDOW_H
