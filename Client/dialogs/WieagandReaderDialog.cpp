#include "WieagandReaderDialog.h"
#include "ui_WieagandReaderDialog.h"
#include <QDebug>
WieagandReaderDialog::WieagandReaderDialog(QWidget *parent, const QString &title) :
    QDialog(parent),
    ui(new Ui::WieagandReaderDialog)
{
    ui->setupUi(this);
    ui->label->setText("<p align=\"center\"><span style=\" font-size:12pt;\">"+title+"</span></p>");
}

WieagandReaderDialog::~WieagandReaderDialog()
{
    delete ui;
}

quint32 WieagandReaderDialog::getCode()
{
//    QString temp = ui->lineEdit->text();
//    if((temp.contains(";",Qt::CaseInsensitive) && temp.contains(",",Qt::CaseInsensitive) && temp.contains("?",Qt::CaseInsensitive))
//            or (temp.contains("ж",Qt::CaseInsensitive) && temp.contains("б",Qt::CaseInsensitive))){
//        temp.remove(0,5);
//        temp.chop(1);
//        return temp.toUInt();
//    }
//    return 0;

    return ui->lineEdit->text().toUInt();
}
