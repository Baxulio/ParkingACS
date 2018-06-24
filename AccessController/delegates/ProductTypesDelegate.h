#ifndef PRODUCTTYPESDELEGATE_H
#define PRODUCTTYPESDELEGATE_H

#include <QFrame>

namespace Ui {
class ProductTypesDelegate;
}

class ProductTypesDelegate : public QFrame
{
    Q_OBJECT

public:
    explicit ProductTypesDelegate(QWidget *parent = 0);
    ~ProductTypesDelegate();

private:
    Ui::ProductTypesDelegate *ui;
};

#endif // PRODUCTTYPESDELEGATE_H
