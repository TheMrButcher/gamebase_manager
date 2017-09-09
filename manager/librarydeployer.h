#ifndef LIBRARYDEPLOYER_H
#define LIBRARYDEPLOYER_H

#include "library.h"
#include <QRunnable>
#include <QObject>
#include <QDir>

class FilesManager;
class Compiler;

class LibraryDeployer : public QObject, public QRunnable
{
    Q_OBJECT

public:
    LibraryDeployer(const Library& library);

    virtual void run() override;

signals:
    void finishedDeploy(Library library);

private:
    bool unarchive(QDir srcDir);
    bool compileSources(QDir dir);
    void emitFinish();

    Library library;
    LibrarySource workingDir;
    FilesManager* manager;
    Compiler* compiler;
};

#endif // LIBRARYDEPLOYER_H
