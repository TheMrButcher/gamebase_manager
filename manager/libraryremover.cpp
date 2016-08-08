#include "libraryremover.h"
#include "files.h"
#include "filesmanager.h"
#include "progressmanager.h"
#include <QWidget>
#include <QDir>

LibraryRemover::LibraryRemover(const Library& library)
    : library(library)
{
    ProgressManager::instance()->show("Удаление библиотеки...", "Подготавливается список файлов...",
                                      "Удалено файлов");
    manager = new FilesManager(this);
    manager->setRootDirectory(library.source.path);
}

void LibraryRemover::run()
{
    switch (library.state) {
    case Library::BinaryArchive:
    case Library::SourceCode:
    {
        manager->remove(library.archiveName);
        break;
    }

    case Library::Deployed:
    {
        manager->remove(Files::EDITOR_LINK_NAME);
        manager->remove(Files::DEPLOYED_ROOT_DIR_NAME);
        break;
    }

    default: break;
    }
    manager->run();
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
