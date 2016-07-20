#include "librarysourcemanager.h"
#include "githublibrarydownloader.h"
#include "directorysourcemanager.h"
#include "workingdirmanager.h"
#include <QWidget>

LibrarySourceManager* LibrarySourceManager::create(const LibrarySource& source, QWidget* parent)
{
    if (source.type == LibrarySource::Server)
        return new GithubLibraryDownloader(source, parent);
    if (source.type == LibrarySource::WorkingDirectory)
        return new WorkingDirManager(source, parent);
    return new DirectorySourceManager(source, parent);
}
