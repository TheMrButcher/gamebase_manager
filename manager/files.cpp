#include "files.h"
#include <QFile>

const QString Files::BINARY_ARCHIVE_NAME = "bin.zip";
const QString Files::SOURCES_ARCHIVE_NAME = "sources.zip";
const QString Files::VERSION_FILE_NAME = "VERSION.txt";
const QString Files::EDITOR_LINK_NAME = "DesignEditor.lnk";
const QString Files::DEPLOYED_ROOT_DIR_NAME = "gamebase";
const QString Files::CONTRIB_DIR_NAME = "contrib";
const QString Files::BIN_DIR_NAME = "bin";
const QString Files::INCLUDE_DIR_NAME = "include";
const QString Files::PACKAGE_DIR_NAME = "package";
const QString Files::EDITOR_PROJECT_NAME = "design_editor";
const QString Files::RESOURCES_DIR_NAME = "resources";
const QString Files::DESIGNS_DIR_NAME = "designs";
const QString Files::FONTS_DIR_NAME = "fonts";

bool Files::exists(const QDir& dir, QString fname)
{
    QFile file(dir.absoluteFilePath(fname));
    return file.exists();
}

bool Files::existsDir(const QDir& dir, QString fname)
{
    return QDir(dir).cd(fname);
}
