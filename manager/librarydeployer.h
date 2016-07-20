#ifndef LIBRARYDEPLOYER_H
#define LIBRARYDEPLOYER_H

#include "library.h"
#include <QRunnable>
#include <QObject>
#include <QDir>

class LibraryDeployer : public QObject, public QRunnable
{
    Q_OBJECT

public:
    LibraryDeployer(const Library& library);

    virtual void run() override;

signals:
    void finishedDeploy(Library library);

private:
    bool unarchiveSources(QDir srcDir, QDir dstDir);
    bool compileSources(QDir dir);
    bool compileProject(QDir projectDir);
    void emitFinish();

    Library library;
    LibrarySource workingDir;
};

#endif // LIBRARYDEPLOYER_H