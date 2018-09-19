#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <DatabaseManager.h>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class SettingsDialog;
class SlidingStackedWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    DatabaseManager &bDb;

    SettingsDialog *bSettings;
    SlidingStackedWidget *bStackedWidget;

    QTimer timer;
protected:
    void closeEvent(QCloseEvent *event) override;

private:
    void initActionsConnections();
    void readSettings();
    void writeSettings();

signals:
    void showStatusMessage(const QString &message);
};

#endif // MAINWINDOW_H
