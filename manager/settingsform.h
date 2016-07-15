#ifndef SETTINGSFORM_H
#define SETTINGSFORM_H

#include "librarysourcestablemodel.h"
#include "appsourcestablemodel.h"
#include <QWidget>
#include <QItemSelection>

namespace Ui {
class SettingsForm;
}

class Settings;

class SettingsForm : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsForm(QWidget *parent = 0);
    ~SettingsForm();

    void set(const Settings& settings);
    Settings get() const;

private slots:
    void on_acceptButton_clicked();

    void on_cancelButton_clicked();

    void onGamebaseSourcesSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void on_removeGamebasePathButton_clicked();

    void on_addGamebaseServerButton_clicked();

    void on_addGamebaseDirectoryButton_clicked();

    void onAppSourcesSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void on_addAppDirectoryButton_clicked();

    void on_removeAppPathButton_clicked();

    void on_chooseWorkingDir_clicked();

private:
    Ui::SettingsForm *ui;
    LibrarySourcesTableModel* librarySourcesModel;
    AppSourcesTableModel* appSourcesModel;
};

#endif // SETTINGSFORM_H
