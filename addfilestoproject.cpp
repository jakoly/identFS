#include "addfilestoproject.h"
#include <QDebug>
#include "ui_addfilestoproject.h"

addFilesToProject::addFilesToProject(MainWindow* mw, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::addFilesToProject)
{
    ui->setupUi(this);

    connect(ui->pushButtonCancel, &QPushButton::clicked, this, &addFilesToProject::onCancelClicked);

    ui->label->setText(QString("Projekt: %1").arg(mw->projectName));
    ui->labelCreated->setText(QString("Projekt erstellt:    %1").arg(mw->projectCreated));
    ui->labelLastChange->setText(QString("Projekt geändert:     %1").arg(mw->projectModificated));
}

addFilesToProject::~addFilesToProject()
{
    delete ui;
}

void addFilesToProject::onCancelClicked() {
    close();
}
