#include "directorysourcemanager.h"
#include "librarycopier.h"
#include <QDir>
#include <QThreadPool>

DirectorySourceManager::DirectorySourceManager(const LibrarySource& source, QObject* parent)
    : LibrarySourceManager(parent)
    , source(source)
{}

void DirectorySourceManager::update()
{
    if (source.check() != SourceStatus::OK) {
        emit finishedUpdate(source, QList<Library>());
        return;
    }

    QDir dir(source.path);
    auto children = dir.entryList(QDir::NoDotAndDotDot | QDir::AllDirs);
    QList<Library> libraries;
    libraries.reserve(children.size());
    foreach (auto child, children) {
        auto library = Library::fromFileSystem(source, child);
        if (library.state == Library::Absent)
            continue;
        libraries.append(library);
    }
    emit finishedUpdate(source, libraries);
}

void DirectorySourceManager::download(const Library& library)
{
    auto copier = new LibraryCopier(library, library.afterAction(Library::Download));
    connect(copier, SIGNAL(finishedCopy(Library)), this, SIGNAL(finishedDownload(Library)));
    QThreadPool::globalInstance()->start(copier);
}
