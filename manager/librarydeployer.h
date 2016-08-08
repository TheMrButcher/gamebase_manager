#ifndef LIBRARYDEPLOYER_H
#define LIBRARYDEPLOYER_H

#include "library.h"
#include <QRunnable>
#include <QObject>
#include <QDir>

class FilesManager;

class LibraryDeployer : public QObject, public QRunnable
{
    Q_OBJECT

public:
    LibraryDeployer(const Library& library);

    virtual void run() override;

signals:
    void finishedDeploy(Library library);

private:
    bool unarchiveSources(QDir srcDir);
    bool compileSources(QDir dir);
    bool compileProject(QDir projectDir);
    bool compileProject(QDir projectDir, QString scriptName);
    void emitFinish();

    Library library;
    LibrarySource workingDir;
    FilesManager* manager;
};

#endif // LIBRARYDEPLOYER_H
