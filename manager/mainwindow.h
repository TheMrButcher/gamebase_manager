#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "aboutwindow.h"
#include "settingsform.h"
#include "librariesform.h"
#include "appsform.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void update(const Settings& curSettings);

public slots:
    void update();
    void updateLibrarySources();
    void updateAppSources();

private slots:
    void on_aboutAction_triggered();

    void onLibrarySourceUpdateFinished(LibrarySource source, const QList<Library>& libraries);
    void onAppSourceUpdateFinished(AppSource source, const QList<App>& apps);

private:
    void updateLibrarySources(const Settings& curSettings);
    void updateAppSources(const Settings& curSettings);

    Ui::MainWindow *ui;
    AboutWindow* about;
    SettingsForm* settings;
    LibrariesForm* libraries;
    AppsForm* apps;
};

#endif // MAINWINDOW_H
