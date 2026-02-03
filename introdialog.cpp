#include "introdialog.h"
#include "ui_introdialog.h"
#include <QSettings>

introdialog::introdialog(QWidget *parent)
    : QDialog(parent),
    ui(new Ui::introdialog)
{
    ui->setupUi(this);

    connect(ui->pushButtonNext, &QPushButton::clicked, this, &introdialog::onNext);
    connect(ui->pushButtonBack, &QPushButton::clicked, this, &introdialog::onBack);
    connect(ui->pushButtonFinish, &QPushButton::clicked, this, &introdialog::onFinish);

    updateButtons();
}

introdialog::~introdialog() {
    delete ui;
}

void introdialog::onNext() {
    ui->stackedWidget->setCurrentIndex(
        ui->stackedWidget->currentIndex() + 1);
    updateButtons();
}

void introdialog::onBack() {
    ui->stackedWidget->setCurrentIndex(
        ui->stackedWidget->currentIndex() - 1);
    updateButtons();
}

void introdialog::onFinish() {
    accept();
}

void introdialog::updateButtons() {
    int index = ui->stackedWidget->currentIndex();
    int max = ui->stackedWidget->count() - 1;

    ui->pushButtonBack->setVisible(index > 0);
    ui->pushButtonNext->setVisible(index < max);
    ui->pushButtonFinish->setVisible(index == max);
}
