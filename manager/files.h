#ifndef FILES_H
#define FILES_H

#include <QString>
#include <QDir>

class Files
{
public:
    static const QString GAMEBASE_PROJECT_NAME;

    static const QString CONTRIB_ARCHIVE_NAME;
    static const QString GAMEBASE_ARCHIVE_NAME;
    static const QString VERSION_FILE_NAME;

    static const QString DEPLOYED_ROOT_DIR_NAME;

    static const QString CONTRIB_DIR_NAME;
    static const QString BIN_DIR_NAME;
    static const QString INCLUDE_DIR_NAME;
    static const QString DEBUG_DIR_NAME;
    static const QString RELEASE_DIR_NAME;
    static const QString EDITOR_DIR_NAME;

    static const QString PACKAGE_DIR_NAME;

    static const QString EDITOR_LINK_NAME;
    static const QString EDITOR_PROJECT_NAME;
    static const QString EDITOR_CONFIG_NAME;

    static const QString RESOURCES_DIR_NAME;
    static const QString DESIGNS_DIR_NAME;
    static const QString FONTS_DIR_NAME;

    static const QString SOURCES_DIR_NAME;

    static const QString APP_CONFIG_NAME;
    static const QString APP_PROJECT_NAME;

    static const QString UPDATE_SCRIPT_NAME;

    static bool exists(const QDir& dir, QString fname);
    static bool existsDir(const QDir& dir, QString fname);
    static bool copyTextFile(QString srcPath, QString dstPath, bool withBOM = false);
    static bool writeTextFile(QString data, QString dstPath, bool withBOM = false);
    static void copyDir(QString srcPath, QString dstPath);
    static void copyDir(QDir srcDir, QDir dstDir);
    static QString absPath(QString path);
    static QString absPath(QDir dir, QString path);
};

#endif // FILES_H
