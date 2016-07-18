#include "files.h"
#include <QFile>

const QString Files::BINARY_ARCHIVE_NAME = "bin.zip";
const QString Files::SOURCES_ARCHIVE_NAME = "sources.zip";
const QString Files::VERSION_FILE_NAME = "VERSION.txt";

bool Files::exists(const QDir& dir, QString fname)
{
    QFile file(dir.absoluteFilePath(fname));
    return file.exists();
}

bool Files::existsDir(const QDir& dir, QString fname)
{
    return QDir(dir).cd(fname);
}
