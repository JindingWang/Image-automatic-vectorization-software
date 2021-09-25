#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QValidator>
#include <QDebug>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(float* coordinate, QWidget *parent = nullptr);
    void on_OK_clicked();
    ~Dialog();

signals:
    void ExitWin();

private:
    Ui::Dialog *ui;
    float* coor = nullptr;
};

#endif // DIALOG_H
