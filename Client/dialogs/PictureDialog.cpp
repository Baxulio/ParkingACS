#include "PictureDialog.h"
#include "ui_PictureDialog.h"

#include <QPicture>

PictureDialog::PictureDialog(QByteArray arr, QString title, QWidget *parent):
    QDialog(parent),
    ui(new Ui::PictureDialog)
{
    ui->setupUi(this);

    setWindowTitle(title);

    QPixmap pix;
    if(pix.loadFromData(arr,"JPG")){
        pix.setDevicePixelRatio(4);
        ui->label->setPixmap(pix);
        ui->label->setScaledContents(true);
    }
    else ui->label->setText("Неудалось получить изображение!");
}

PictureDialog::~PictureDialog()
{
    delete ui;
}
