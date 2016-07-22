#ifndef LIBRARYCOPIER_H
#define LIBRARYCOPIER_H

#include "library.h"
#include <QRunnable>
#include <QObject>

class LibraryCopier : public QObject, public QRunnable
{
    Q_OBJECT

public:
    LibraryCopier(const Library& library, const Library& resultLibrary);

    virtual void run() override;

signals:
    void finishedCopy(Library library);

private:
    void emitFinish();

    Library library;
    Library resultLibrary;
};

#endif // LIBRARYCOPIER_H
