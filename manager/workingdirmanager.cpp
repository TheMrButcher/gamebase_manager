#include "workingdirmanager.h"
#include <QDir>

WorkingDirManager::WorkingDirManager(const LibrarySource& source, QObject* parent)
    : LibrarySourceManager(parent)
    , source(source)
{}

void WorkingDirManager::update()
{
    source.check();
    QList<Library> libraries;
    libraries.append(Library::fromFileSystem(source));
    emit finishedUpdate(source, libraries);
}

void WorkingDirManager::download(const Library& library)
{
    emit finishedDownload(Library::makeAbsent(source));
}
