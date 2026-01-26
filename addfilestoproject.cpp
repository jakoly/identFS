#include "addfilestoproject.h"
#include <QDebug>
#include "ui_addfilestoproject.h"

addFilesToProject::addFilesToProject(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::addFilesToProject)
{
    ui->setupUi(this);


}

addFilesToProject::~addFilesToProject()
{
    delete ui;
}

void addFilesToProject::someFunction(MainWindow* mw)
{
    QString name = mw->projectName;
    QString uuid = mw->projectUUID;

    qDebug() << name << uuid;
}

