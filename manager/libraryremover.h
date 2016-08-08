#ifndef LIBRARYREMOVER_H
#define LIBRARYREMOVER_H

#include "library.h"
#include <QRunnable>
#include <QObject>

class QProgressDialog;
class FilesManager;

class LibraryRemover : public QObject, public QRunnable
{
    Q_OBJECT

public:
    LibraryRemover(const Library& library);

    virtual void run() override;

    static bool checkHasDeployedFiles(QString path);

signals:
    void finishedRemove(Library library);

private:
    void emitFinish();

    Library library;
    FilesManager* manager;
};

#endif // LIBRARYREMOVER_H
