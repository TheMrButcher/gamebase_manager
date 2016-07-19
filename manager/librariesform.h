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

public slots:
    void onLibraryDeployed(Library library);
    void onLibraryRemoved(Library library);

private slots:
    void onLibraryDownloaded(Library library);

    void onLibrariesSelectionChanged(const QItemSelection &selected, const QItemSelection &);

    void on_downloadButton_clicked();

    void on_removeButton_clicked();

    void on_installButton_clicked();

private:
    int selectedRow() const;

    Ui::LibrariesForm *ui;
    LibrariesTableModel* librariesModel;
    Library toInstall;
    Library::Ability waitedInstallAction;
};

#endif // LIBRARIESFORM_H
