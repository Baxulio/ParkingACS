#ifndef STATUSFORM_H
#define STATUSFORM_H

#include <QWidget>

namespace Ui {
class StatusForm;
}

class StatusForm : public QWidget
{
    Q_OBJECT

public:
    explicit StatusForm(QWidget *parent = 0);
    ~StatusForm();

private:
    Ui::StatusForm *ui;
};

#endif // STATUSFORM_H
