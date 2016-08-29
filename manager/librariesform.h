#ifndef LIBRARIESFORM_H
#define LIBRARIESFORM_H

#include "librariestablemodel.h"
#include <QWidget>

namespace Ui {
class LibrariesForm;
}

class MainWindow;
class QItemSelection;

class LibrariesForm : public QWidget
{
    Q_OBJECT

public:
    explicit LibrariesForm(MainWindow *parent = 0);
    ~LibrariesForm();

    void clearLibrariesTable();
    void append(const QList<Library>& libraries);
    void download(Library library);
    void install(Library library);

signals:
    void finishedInstall(Library srcLibrary, bool success);
    void finishedRemove(Library library);

private:
    void installImpl(Library library);
    void finishInstall(bool success);

private slots:
    void onLibraryDeployed(Library library);
    void onLibraryRemoved(Library library);
    void onLibraryDownloaded(Library library);

    void onLibrariesSelectionChanged(const QItemSelection &selected, const QItemSelection &);

    void on_downloadButton_clicked();

    void on_removeButton_clicked();

    void on_installButton_clicked();

private:
    int selectedRow() const;
    void remove(Library library);
    void updateButtons();

    Ui::LibrariesForm* ui;
    MainWindow* parent;
    LibrariesTableModel* librariesModel;
    Library srcLibToInstall;
    Library toInstall;
    Library::Ability waitedInstallAction;

    bool hasActiveDownload;
};

#endif // LIBRARIESFORM_H
