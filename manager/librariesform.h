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

private slots:
    void onLibraryDownloaded(const Library& library);

    void onLibrariesSelectionChanged(const QItemSelection &selected, const QItemSelection &);

    void on_downloadButton_clicked();

private:
    Ui::LibrariesForm *ui;
    LibrariesTableModel* librariesModel;
};

#endif // LIBRARIESFORM_H
