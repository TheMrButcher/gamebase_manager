#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "aboutwindow.h"
#include "settingsform.h"
#include "librariesform.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void updateLibrarySources(const QList<LibrarySource>& sources);

public slots:
    void updateLibrarySources();

private slots:
    void on_aboutAction_triggered();

    void onUpdateFinished(LibrarySource source, const QList<Library>& libraries);

private:
    Ui::MainWindow *ui;
    AboutWindow* about;
    SettingsForm* settings;
    LibrariesForm* libraries;
};

#endif // MAINWINDOW_H
