#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "library.h"
#include "app.h"

namespace Ui {
class MainWindow;
}

struct Settings;
class AboutWindow;
class MainTabForm;
class SettingsForm;
class LibrariesForm;
class AppsForm;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void updateAll(const Settings& curSettings);

    SettingsForm* settingsForm() const;
    LibrariesForm* librariesForm() const;
    AppsForm* appsForm() const;

public slots:
    void updateAll();
    void updateLibrarySources();
    void updateAppSources();
    void gotUpdates(bool any);

private slots:
    void on_aboutAction_triggered();

    void onLibrarySourceUpdateFinished(LibrarySource source, const QList<Library>& libraries);
    void onAppSourceUpdateFinished(AppSource source, const QList<App>& apps);
    void startAdditionalBackgroundTasks();

private:
    void updateLibrarySources(const Settings& curSettings);
    void updateAppSources(const Settings& curSettings);

    Ui::MainWindow *ui;
    AboutWindow* about;
    MainTabForm* mainTab;
    SettingsForm* settings;
    LibrariesForm* libraries;
    AppsForm* apps;
    bool askAboutUpdates;
};

#endif // MAINWINDOW_H
