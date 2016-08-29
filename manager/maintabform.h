#ifndef MAINTABFORM_H
#define MAINTABFORM_H

#include "library.h"
#include "app.h"
#include <QWidget>
#include <QList>
#include <QMap>

namespace Ui {
class MainTabForm;
}

class MainWindow;
class QPushButton;

class MainTabForm : public QWidget
{
    Q_OBJECT

public:
    explicit MainTabForm(MainWindow *parent);
    ~MainTabForm();

public slots:
    void updateState();
    void updateView();

private:
    void updateDirsState();
    void updateLibraryState();
    void updateAppsState();
    QString appsList(bool curExists, bool latestExists);
    QString nextAppToProcess();
    void deployApp(QString appName);
    void deployApps();

    enum FeatureStatus {
        Unknown,
        Error,
        Warning,
        OK
    };
    QPushButton* addRow(int row, FeatureStatus status, QString label, QString buttonLabel);

    void processNextAction();

private slots:
    void createWorkingDir();
    void createDownloadsDir();
    void createOutputDir();
    void detectMSVC();
    void deployLibrary();
    void installApps();
    void updateApps();
    void onLibrarySourceUpdate(LibrarySource source, const QList<Library>& libraries);
    void onAppAdded(App app, bool success);
    void onAppRemoved(App app);
    void onAppSourceUpdate(AppSource source, const QList<App>& newApps);
    void onLibraryDeployed(Library library, bool success);
    void onLibraryRemoved(Library library);

    void on_updateStateButton_clicked();

    void on_installAllButton_clicked();

private:
    Ui::MainTabForm *ui;
    MainWindow* parent;
    int curRow;
    QList<QWidget*> stateWidgets;

    enum ActionType {
        Idle,
        CreateWorkingDir,
        CreateDownloadsDir,
        CreateOutputDir,
        DetectMSVC,
        InstallLibrary,
        ProcessApps,
        OnlyInstallApps,
        OnlyUpdateApps
    };

    ActionType action;
    Library neededLib;
    Library curLib;

    struct AppInfo {
        App cur;
        App latest;
    };
    QMap<QString, AppInfo> apps;
};

#endif // MAINTABFORM_H
