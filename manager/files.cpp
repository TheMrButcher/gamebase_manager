#include "files.h"
#include <QFile>

const QString Files::BINARY_ARCHIVE_NAME = "bin.zip";
const QString Files::SOURCES_ARCHIVE_NAME = "sources.zip";
const QString Files::VERSION_FILE_NAME = "VERSION.txt";
const QString Files::EDITOR_LINK_NAME = "DesignEditor.lnk";
const QString Files::DEPLOYED_ROOT_DIR_NAME = "gamebase";

bool Files::exists(const QDir& dir, QString fname)
{
    QFile file(dir.absoluteFilePath(fname));
    return file.exists();
}

bool Files::existsDir(const QDir& dir, QString fname)
{
    return QDir(dir).cd(fname);
}
