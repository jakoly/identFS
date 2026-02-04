#ifndef MAINWINDOW_H
#define MAINWINDOW_H

// System/Standard headers
#include <objbase.h>
#include <string>
#include <windows.h>

// Qt headers
#include <QDebug>
#include <QFont>
#include <QIcon>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QSettings>

// Project/Local headers
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
    sqlite3* db;

    QString projectName;
    QString projectUUID;
    QString projectCreated;
    QString projectModificated;
    QString projectVaultPath;
    QString installPath;
    QStringList tempFileUUIDs;
    bool firstTimeOpened;

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
    void repaireFiles();
    void copyWithRobocopy(const QString& currentPath, const QString& lastPath);
    void startWorkerThread();
    void pauseWorker();
    void stopWorker();
    void resumeWorker();
    void createSetting();
    void onCancelClicked();
    std::string guidToString(const GUID& guid);

private:
    Ui::MainWindow *ui;
    int updateCounter = 0;

};
#endif // MAINWINDOW_H
