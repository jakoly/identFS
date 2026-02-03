#ifndef INTRODIALOG_H
#define INTRODIALOG_H

#include <QDialog>

namespace Ui {
class introdialog;
}

class introdialog : public QDialog
{
    Q_OBJECT

public:
    explicit introdialog(QWidget *parent = nullptr);
    ~introdialog();

private slots:
    void onNext();
    void onBack();
    void onFinish();


private:
    Ui::introdialog *ui;
    void updateButtons();
};

#endif // INTRODIALOG_H
