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
#include <QListWidget>

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
    QString installPath;
    QStringList tempFileUUIDs;

    void turnAddProjectInvisible();

public slots:
    void newProject();
    void openAddFilesWindow();
    void updateProjectList();
    void updateFileList();
    void onProjectItemClicked(QListWidgetItem *item);
    void onFileClicked(QListWidgetItem *item);
    void onOkClicked();
    void loadProjectFiles(const QString &projectUuid);
    void onRecycleClicked();
    void onSettingsClicked();
    void repeatTask();
    std::string guidToString(const GUID& guid);

private:
    Ui::MainWindow *ui;

};
#endif // MAINWINDOW_H
