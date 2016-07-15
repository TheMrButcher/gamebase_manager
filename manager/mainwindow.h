#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "aboutwindow.h"
#include "settingsform.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_aboutAction_triggered();

private:
    Ui::MainWindow *ui;
    AboutWindow* about;
    SettingsForm* settings;
};

#endif // MAINWINDOW_H
