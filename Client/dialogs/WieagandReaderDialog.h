#ifndef WIEAGANDREADERDIALOG_H
#define WIEAGANDREADERDIALOG_H

#include <QDialog>

namespace Ui {
class WieagandReaderDialog;
}

class WieagandReaderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit WieagandReaderDialog(QWidget *parent = 0, const QString &title="Приложите браслет!");
    ~WieagandReaderDialog();

    quint32 getCode();
private:
    Ui::WieagandReaderDialog *ui;
};

#endif // WIEAGANDREADERDIALOG_H
