#include "ProductTypesDelegate.h"
#include "ui_ProductTypesDelegate.h"

ProductTypesDelegate::ProductTypesDelegate(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::ProductTypesDelegate)
{
    ui->setupUi(this);
}

ProductTypesDelegate::~ProductTypesDelegate()
{
    delete ui;
}
