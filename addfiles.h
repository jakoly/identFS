#ifndef ADDFILES_H
#define ADDFILES_H

#include <QDialog>
#include <QFileDialog>
#include <QStringList>
#include "libs/sqlite/sqlite3.h"
#include "mainwindow.h"

namespace Ui {
class addFiles;
}

class addFiles : public QDialog
{
    Q_OBJECT

public:
    explicit addFiles(QWidget *parent = nullptr);
    ~addFiles();
    sqlite3* db;   // DB-Pointer
    float getFileSize(const std::string &filename);
    std::string getFileNameFromPath(const std::string& filePath);
    QStringList files;

private slots:
    void selectFiles();
    void onOkClicked();
    void onCancelClicked();

private:
    Ui::addFiles *ui;
    std::string guidToString(const GUID& guid);
    QString readUuidFromFile(const QString &filePath);
};

#endif // ADDFILES_H
