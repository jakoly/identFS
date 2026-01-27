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
    explicit addFilesToProject(MainWindow* mw, QWidget *parent = nullptr);
    ~addFilesToProject();

private:
    Ui::addFilesToProject *ui;
    QString name;
    QString uuid;

private slots:
    void onCancelClicked();
};

#endif // ADDFILESTOPROJECT_H
