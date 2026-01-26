#ifndef ADDFILESTOPROJECT_H
#define ADDFILESTOPROJECT_H

#include <QDialog>
#include "mainwindow.h"

namespace Ui {
class addFilesToProject;
}

class addFilesToProject : public QDialog
{
    Q_OBJECT

public:
    explicit addFilesToProject(QWidget *parent = nullptr);
    ~addFilesToProject();
    void someFunction(MainWindow* mw);

private:
    Ui::addFilesToProject *ui;
};

#endif // ADDFILESTOPROJECT_H
