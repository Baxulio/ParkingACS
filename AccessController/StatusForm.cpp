#include "StatusForm.h"
#include "ui_StatusForm.h"

StatusForm::StatusForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::StatusForm)
{
    ui->setupUi(this);
}

StatusForm::~StatusForm()
{
    delete ui;
}
