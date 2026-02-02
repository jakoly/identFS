#include "settings.h"
#include "ui_settings.h"
#include <fstream>
#include <QRegularExpression>
#include <QMessageBox>


settings::settings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::settings)
{
    ui->setupUi(this);

    connect(ui->okButton, &QPushButton::clicked, this, &settings::onOkClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &settings::onCancelClicked);

    // Datei wieder einlesen
    QString filePath = QCoreApplication::applicationDirPath() + "/settings/stdPath.txt";
    std::ifstream in(filePath.toStdString());
    if (in.is_open()) {
        std::string tempString;
        std::getline(in, tempString);
        in.close();
        ui->lineEditStdPath->setPlaceholderText("aktueller Laufwerkbuchstabe: " + QString::fromStdString(tempString).trimmed());
    } else {
        qDebug() << "Fehler: Konnte Datei zum Lesen nicht öffnen:" << filePath;
    }
}

settings::~settings()
{
    delete ui;
}

void settings::loadStdPath() {
    // Absoluter Pfad zur Datei
    QString filePath = QCoreApplication::applicationDirPath() + "/settings/stdPath.txt";
    std::ifstream in(filePath.toStdString());
    if (in.is_open()) {
        std::string tempString;
        std::getline(in, tempString);
        in.close();
        ui->lineEditStdPath->setPlaceholderText("aktueller Laufwerkbuchstabe: " + QString::fromStdString(tempString).trimmed());
    } else {
        qDebug() << "Fehler: Konnte Datei zum Lesen nicht öffnen:" << filePath;
    }
}

void settings::onOkClicked() {
    QString StdPath = ui->lineEditStdPath->text();

    if (StdPath.isEmpty()) {
        QMessageBox msgBox;
        msgBox.setText("Bitte gib etwas ein!");
        msgBox.exec();
        ui->lineEditStdPath->clear();
        loadStdPath();
        return;
    }

    // Absoluter Pfad zur Datei
    QString filePath = QCoreApplication::applicationDirPath() + "/settings/stdPath.txt";

    // Datei schreiben
    std::ofstream out(filePath.toStdString());
    if (out.is_open()) {
        out << StdPath.toStdString();
        out.close();
    } else {
        qDebug() << "Fehler: Konnte Datei zum Schreiben nicht öffnen:" << filePath;
        return;
    }

    ui->lineEditStdPath->clear();

    // Datei wieder einlesen
    loadStdPath();
}


void settings::onCancelClicked() {
    close();
}
