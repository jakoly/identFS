#ifndef ADDFILES_H
#define ADDFILES_H

// Qt headers
#include <QDialog>
#include <QFileDialog>
#include <QStringList>

// Project/Local headers
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
    sqlite3* db;
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
    std::string hashText(const std::string &inputText);
    QString installPath;
};

#endif // ADDFILES_H
