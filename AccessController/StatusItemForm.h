#ifndef STATUSITEMFORM_H
#define STATUSITEMFORM_H

#include <QWidget>

namespace Ui {
class StatusItemForm;
}

class StatusItemForm : public QWidget
{
    Q_OBJECT


public:
    explicit StatusItemForm(QString enterTime,
                            QString code,
                            QWidget *parent = 0,
                            QString accessType = "ОПЛАТА",
                            QString enter_exit_text="",
                            QString elapsed = "");
    ~StatusItemForm();

private:
    Ui::StatusItemForm *ui;
};

#endif // STATUSITEMFORM_H
