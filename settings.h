#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

namespace Ui {
class settings;
}

class settings : public QDialog
{
    Q_OBJECT

public:
    explicit settings(QWidget *parent = nullptr);
    ~settings();

    void saveSettings (const QString &key, const QVariant &value, const QString &group);
    void applySettings();
    QVariant loadSettings(const QString &key, const QVariant &defaultValue, const QString &group);

private:
    Ui::settings *ui;
};

#endif // SETTINGS_H
