#ifndef LIBRARYREMOVER_H
#define LIBRARYREMOVER_H

#include "library.h"
#include <QRunnable>

class QWidget;

class LibraryRemover : public QRunnable
{
public:
    LibraryRemover(const Library& library, QWidget* widget);

    virtual void run() override;

    static bool checkHasDeployedFiles(QString path);

private:
    void emitFinish();

    Library library;
    QWidget* widget;
};

#endif // LIBRARYREMOVER_H
