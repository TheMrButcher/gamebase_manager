#ifndef APPSFORM_H
#define APPSFORM_H

#include "appstablemodel.h"
#include <QWidget>

namespace Ui {
class AppsForm;
}

class MainWindow;
class QItemSelection;
class NewAppDialog;
class AppConfigurationDialog;
class AppCompressionDialog;
class AppDeploySuccessDialog;

class AppsForm : public QWidget
{
    Q_OBJECT

public:
    explicit AppsForm(MainWindow *parent = 0);
    ~AppsForm();

    void clearAppsTable();
    void append(const QList<App>& apps);
    void reconfigurateAll();
    void addApp(App app);

signals:
    void addedApp(const App& app, bool success);
    void removedApp(const App& app);

private slots:
    void onAppAdded(App app);
    void onAppUpdate(App app);
    void onAppRename(App oldApp, App newApp);
    void onTempAppAdded(App app);
    void onAppDeployed(App app, QString path, bool success);

    void onAppsSelectionChanged(const QItemSelection &, const QItemSelection &);

    void on_createButton_clicked();

    void on_configureButton_clicked();

    void on_removeButton_clicked();

    void on_compressButton_clicked();

    void on_addButton_clicked();

    void on_deployButton_clicked();

    void on_openSolutionButton_clicked();

    void on_openDirButton_clicked();

private:
    int selectedRow() const;
    void removeApp(App app);
    void updateButtons();

    Ui::AppsForm *ui;
    AppsTableModel* appsModel;
    NewAppDialog* newAppDialog;
    AppConfigurationDialog* configDialog;
    AppCompressionDialog* compressionDialog;
    AppDeploySuccessDialog* deploySuccessDialog;
};

#endif // APPSFORM_H
