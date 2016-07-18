#include "librarydeployer.h"
#include "settings.h"
#include "files.h"
#include "archive.h"
#include <JlCompress.h>
#include <QWidget>
#include <QDir>

namespace {
void copyDir(QString srcPath, QString dstPath)
{
    QDir srcDir(srcPath);
    QDir dstDir(dstPath);
    auto entries = srcDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const auto& entry, entries) {
        auto name = entry.fileName();
        auto srcFilePath = srcDir.absoluteFilePath(name);
        auto dstFilePath = dstDir.absoluteFilePath(name);
        if (entry.isDir()) {
            dstDir.mkdir(name);
            copyDir(srcFilePath, dstFilePath);
        } else {
            QFile::copy(srcFilePath, dstFilePath);
        }
    }
}
}

LibraryDeployer::LibraryDeployer(const Library& library, QWidget* widget)
    : library(library)
    , widget(widget)
{
    workingDir = Settings::instance().workingDir();
}

void LibraryDeployer::run()
{
    QDir srcDir(library.source.path);
    if (!srcDir.cd(library.archiveName))
        return emitFinish();

    if (workingDir.check() != SourceStatus::OK)
        return emitFinish();
    Library::removeDeployedFiles(workingDir.path);

    auto sourcesArchivePath = srcDir.absoluteFilePath(Files::SOURCES_ARCHIVE_NAME);
    QDir dstDir(workingDir.path);
    JlCompress::extractDir(sourcesArchivePath, workingDir.path);

    auto unzippedArchiveDirName = Archive::rootName(sourcesArchivePath);
    if (dstDir.cd(unzippedArchiveDirName))
        dstDir.cdUp();
    else
        return emitFinish();
    if (!dstDir.rename(unzippedArchiveDirName, Files::DEPLOYED_ROOT_DIR_NAME))
        return emitFinish();
    dstDir.cd(Files::DEPLOYED_ROOT_DIR_NAME);

    auto packageDir = dstDir;
    packageDir.cd(Files::PACKAGE_DIR_NAME);

    dstDir.mkdir(Files::CONTRIB_DIR_NAME);
    auto contribDir = dstDir;
    contribDir.cd(Files::CONTRIB_DIR_NAME);
    auto contribBinPath = contribDir.absoluteFilePath(Files::BIN_DIR_NAME);
    JlCompress::extractDir(
                packageDir.absoluteFilePath(Files::BIN_DIR_NAME + ".zip"),
                contribBinPath);
    JlCompress::extractDir(
                packageDir.absoluteFilePath(Files::INCLUDE_DIR_NAME + ".zip"),
                contribDir.absoluteFilePath(Files::INCLUDE_DIR_NAME));

    auto resourcesDir = dstDir;
    resourcesDir.cd(Files::RESOURCES_DIR_NAME);
    JlCompress::extractDir(
                packageDir.absoluteFilePath(Files::FONTS_DIR_NAME + ".zip"),
                resourcesDir.absoluteFilePath(Files::FONTS_DIR_NAME));

    resourcesDir.cd(Files::DESIGNS_DIR_NAME);
    copyDir(resourcesDir.absoluteFilePath(Files::EDITOR_PROJECT_NAME),
            contribBinPath);

    auto binariesArchivePath = srcDir.absoluteFilePath(Files::BINARY_ARCHIVE_NAME);
    JlCompress::extractDir(binariesArchivePath, contribBinPath);

    contribDir.cd(Files::BIN_DIR_NAME);
    dstDir.cdUp();
    QFile::link(contribDir.absoluteFilePath(Files::EDITOR_PROJECT_NAME + ".exe"),
                dstDir.absoluteFilePath(Files::EDITOR_LINK_NAME));

    emitFinish();
}

void LibraryDeployer::emitFinish()
{
    QMetaObject::invokeMethod(widget, "onLibraryDeployed", Qt::QueuedConnection,
                              Q_ARG(QString, library.version.toString()));
}
