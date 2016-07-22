#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H

#include "librarysourcestablemodel.h"
#include "appsourcestablemodel.h"
#include <QWidget>
#include <QItemSelection>

namespace Ui {
class SettingsForm;
}

class MainWindow;
class Settings;

class SettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsForm(MainWindow *parent = 0);
    ~SettingsForm();

    void set(const Settings& settings);
    Settings get() const;

    void setAllUnknown();
    void setAllLibrarySourcesUnknown();
    void setAllAppSourcesUnknown();
    void updateLibrarySource(const LibrarySource& source);
    void updateAppSource(const AppSource& source);

private slots:
    void on_acceptButton_clicked();

    void on_cancelButton_clicked();

    void onGamebaseSourcesSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void on_removeGamebasePathButton_clicked();

    void on_addGamebaseServerButton_clicked();

    void on_addGamebaseDirectoryButton_clicked();

    void onAppSourcesSelectionChanged(const QItemSelection &selected, const QItemSelection &);

    void on_addAppDirectoryButton_clicked();

    void on_removeAppPathButton_clicked();

    void on_chooseWorkingDir_clicked();

    void on_chooseDownloadsDir_clicked();

    void on_chooseVCVarsPath_clicked();

    void on_updateButton_clicked();

private:
    Ui::SettingsForm *ui;
    MainWindow* mainWindow;
    LibrarySourcesTableModel* librarySourcesModel;
    AppSourcesTableModel* appSourcesModel;
};

#endif // SETTINGSFORM_H
