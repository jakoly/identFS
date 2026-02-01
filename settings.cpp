#include "settings.h"
#include "ui_settings.h"
#include <QSettings>

settings::settings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::settings)
{
    ui->setupUi(this);

    // Load settings on startup and set UI elements
    bool lightMode = loadSettings("lightMode", true, "Appearance").toBool();
    QString defaultPath = loadSettings("defaultPath", "C:/Users/Default", "Paths").toString();

    ui->checkBoxLightmode->setChecked(lightMode);
    ui->lineEditPath->setText(defaultPath);

    // Connect buttons
    connect(ui->okButton, &QPushButton::clicked, this, &settings::applySettings);
    connect(ui->cancelButton, &QPushButton::clicked, this, &settings::reject); // close dialog without saving
}

settings::~settings()
{
    delete ui;
}

// Called when OK button is clicked
void settings::applySettings() {
    // Save current UI values
    saveSettings("lightMode", ui->checkBoxLightmode->isChecked(), "Appearance");
    saveSettings("defaultPath", ui->lineEditPath->text(), "Paths");

    accept(); // close the dialog with "accepted" status
}

// Save a setting in a given group
void settings::saveSettings(const QString &key, const QVariant &value, const QString &group) {
    QSettings settings("MyCompany", "MyApp"); // persistent storage
    settings.beginGroup(group);
    settings.setValue(key, value);
    settings.endGroup();
}

// Load a setting, return defaultValue if not found
QVariant settings::loadSettings(const QString &key, const QVariant &defaultValue, const QString &group) {
    QSettings settings("MyCompany", "MyApp");
    settings.beginGroup(group);
    QVariant value = settings.value(key, defaultValue);
    settings.endGroup();
    return value;
}
