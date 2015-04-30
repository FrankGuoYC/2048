#include <QMessageBox>
#include <QtCore>
#include <QtGui>

#include "whatis2048.h"
#include "ui_whatis2048.h"

whatis2048::whatis2048(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::whatis2048)
{
    ui->setupUi(this);
    QPixmap pic_bulb(":/images/resource/bulb_s.png");
    ui->label_bulb->setPixmap(pic_bulb);
}

whatis2048::~whatis2048()
{
    delete ui;
}

void whatis2048::on_pushButton_dKnow_clicked()
{
    QMessageBox::information(this,"(￣▽￣)~*","Cheer up!");
}
