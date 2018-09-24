#include "StatusItemForm.h"
#include "ui_StatusItemForm.h"

StatusItemForm::StatusItemForm(QString enterTime, QString code, QWidget *parent, QString accessType, QString enter_exit_text, QString elapsed):
    QWidget(parent),
    ui(new Ui::StatusItemForm)
{
    ui->setupUi(this);

    ui->enter_exit_label->setText(QString("<span style=\"font-style:italic; color:#646464;\">%1</span>").arg(enter_exit_text));
    ui->enter_time_label->setText(QString("<span style=\"font-style:italic; color:#646464;\">%1</span>").arg(enterTime));
    ui->elapsed_label->setText(QString("<span style=\"font-style:italic; color:#646464;\">%1</span>").arg(elapsed));

    ui->code_label->setText(QString("<span style=\"font-size:12pt; font-weight:600;\">%1</span>").arg(code));
    ui->access_type_label->setText(accessType);
}

StatusItemForm::~StatusItemForm()
{
    delete ui;
}
