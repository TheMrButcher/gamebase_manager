#ifndef FILES_H
#define FILES_H

#include <QString>
#include <QDir>

class Files
{
public:
    static const QString BINARY_ARCHIVE_NAME;
    static const QString SOURCES_ARCHIVE_NAME;
    static const QString VERSION_FILE_NAME;
    static const QString EDITOR_LINK_NAME;
    static const QString DEPLOYED_ROOT_DIR_NAME;

    static bool exists(const QDir& dir, QString fname);
    static bool existsDir(const QDir& dir, QString fname);
};

#endif // FILES_H
