#include "dialog.h"
#include "ui_dialog.h"

Dialog::Dialog(float * coordinate, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
    /*QRegExp rx1("^-?((0|1?[0-8]?[0-9]?)(([.][0-9]{1,10})?)|180(([.][0]{1,10})?))$");
    QRegExpValidator *pReg1 = new QRegExpValidator(rx1, this);
    QRegExp rx2("^-?((0|[1-8]?[0-9]?)(([.][0-9]{1,10})?)|90(([.][0]{1,10})?))$");
    QRegExpValidator *pReg2 = new QRegExpValidator(rx2, this);
    QRegExp rx3("^-?((0|1?[0-8]?[0-9]?)(([.][0-9]{1,10})?)|180(([.][0]{1,10})?))$");
    QRegExpValidator *pReg3 = new QRegExpValidator(rx3, this);
    QRegExp rx4("^-?((0|[1-8]?[0-9]?)(([.][0-9]{1,10})?)|90(([.][0]{1,10})?))$");
    QRegExpValidator *pReg4 = new QRegExpValidator(rx4, this);
    ui->lineEdit->setValidator(pReg1);
    ui->lineEdit_2->setValidator(pReg2);
    ui->lineEdit_3->setValidator(pReg3);
    ui->lineEdit_4->setValidator(pReg4);*/
    if (coordinate) coor = coordinate;
    connect(ui->pushButton, &QPushButton::clicked, this, &Dialog::on_OK_clicked);

}

void Dialog::on_OK_clicked()
{
    coor[0] = ui->lineEdit->text().toFloat();
    coor[1] = ui->lineEdit_2->text().toFloat();
    coor[2] = ui->lineEdit_3->text().toFloat();
    coor[3] = ui->lineEdit_4->text().toFloat();
    emit ExitWin();
}

Dialog::~Dialog()
{
    delete ui;
}
