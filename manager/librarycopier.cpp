#include "librarycopier.h"
#include "files.h"
#include <QDir>
#include <QFile>

LibraryCopier::LibraryCopier(const Library& library, const Library& resultLibrary)
    : library(library)
    , resultLibrary(resultLibrary)
{}

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
    QFile::copy(srcDir.absoluteFilePath(Files::SOURCES_ARCHIVE_NAME),
                dstDir.absoluteFilePath(Files::SOURCES_ARCHIVE_NAME));
    if (resultLibrary.state == Library::BinaryArchive)
        QFile::copy(srcDir.absoluteFilePath(Files::BINARY_ARCHIVE_NAME),
                    dstDir.absoluteFilePath(Files::BINARY_ARCHIVE_NAME));
    return emitFinish();
}

void LibraryCopier::emitFinish()
{
    emit finishedCopy(resultLibrary);
}
