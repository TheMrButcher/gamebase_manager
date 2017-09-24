#include "librarycopier.h"
#include "files.h"
#include "largefilemanager.h"
#include "progressmanager.h"
#include <QDir>
#include <QFile>

LibraryCopier::LibraryCopier(const Library& library, const Library& resultLibrary)
    : library(library)
    , resultLibrary(resultLibrary)
{
    ProgressManager::invokeShow("Копирование архива...", "Скопировано килобайт");
    manager = new LargeFileManager(this);
}

void LibraryCopier::run()
{
    if (!library.validate())
        return emitFinish();
    if (resultLibrary.source.check() != SourceStatus::OK)
        return emitFinish();
    if (resultLibrary.archiveName.isEmpty())
        return emitFinish();
    QDir srcDir(library.source.path);
    QDir dstDir(resultLibrary.source.path);
    if (dstDir.cd(resultLibrary.archiveName)) {
        dstDir.removeRecursively();
        dstDir = QDir(resultLibrary.source.path);
    }
    dstDir.mkdir(resultLibrary.archiveName);
    srcDir.cd(library.archiveName);
    dstDir.cd(resultLibrary.archiveName);
    manager->copy(srcDir.absoluteFilePath(Files::CONTRIB_ARCHIVE_NAME),
                  dstDir.absoluteFilePath(Files::CONTRIB_ARCHIVE_NAME));
    manager->copy(srcDir.absoluteFilePath(Files::GAMEBASE_ARCHIVE_NAME),
                  dstDir.absoluteFilePath(Files::GAMEBASE_ARCHIVE_NAME));
    manager->run();
    return emitFinish();
}

void LibraryCopier::emitFinish()
{
    emit finishedCopy(resultLibrary);
}
