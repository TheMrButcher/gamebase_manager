#include "libraryremover.h"
#include "files.h"
#include <QWidget>
#include <QDir>

LibraryRemover::LibraryRemover(const Library& library)
    : library(library)
{}

void LibraryRemover::run()
{
    switch (library.state) {
    case Library::BinaryArchive:
    case Library::SourceCode:
    {
        QDir dir(library.source.path);
        if (!dir.cd(library.archiveName))
            return emitFinish();
        dir.removeRecursively();
        break;
    }

    case Library::Deployed:
    {
        QDir dir(library.source.path);
        dir.remove(Files::EDITOR_LINK_NAME);
        if (dir.exists(Files::DEPLOYED_ROOT_DIR_NAME)) {
            if (!dir.cd(Files::DEPLOYED_ROOT_DIR_NAME))
                return emitFinish();
            dir.removeRecursively();
        } else {
            dir.remove(Files::DEPLOYED_ROOT_DIR_NAME);
        }
        break;
    }

    default: break;
    }
    return emitFinish();
}

bool LibraryRemover::checkHasDeployedFiles(QString path)
{
    QDir dir(path);
    return dir.exists(Files::EDITOR_LINK_NAME) || dir.exists(Files::DEPLOYED_ROOT_DIR_NAME);
}

void LibraryRemover::emitFinish()
{
    emit finishedRemove(library);
}
