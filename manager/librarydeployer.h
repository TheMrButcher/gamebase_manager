#ifndef LIBRARYDEPLOYER_H
#define LIBRARYDEPLOYER_H

#include "library.h"
#include <QRunnable>

class QWidget;

class LibraryDeployer : public QRunnable
{
public:
    LibraryDeployer(const Library& library, QWidget* widget);

    virtual void run() override;

private:
    void emitFinish();

    Library library;
    QWidget* widget;
    LibrarySource workingDir;
};

#endif // LIBRARYDEPLOYER_H
