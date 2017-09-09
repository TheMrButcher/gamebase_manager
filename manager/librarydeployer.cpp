#include "librarydeployer.h"
#include "settings.h"
#include "files.h"
#include "archive.h"
#include "filesmanager.h"
#include "progressmanager.h"
#include "compiler.h"
#include <JlCompress.h>
#include <QWidget>
#include <QDir>

namespace {
const QString GAMEBASE_UNARCHIVED_DIR = "gamebase_tmp";
}

LibraryDeployer::LibraryDeployer(const Library& library)
    : library(library)
{
    workingDir = Settings::instance().workingDir();
    manager = new FilesManager(this);
    manager->setRootDirectory(workingDir.path);
    compiler = new Compiler(this);
}

void LibraryDeployer::run()
{
    QDir srcDir(library.source.path);
    if (!srcDir.cd(library.archiveName))
        return emitFinish();

    if (workingDir.check() != SourceStatus::OK)
        return emitFinish();

    QDir dstDir(workingDir.path);
    if (!unarchive(srcDir))
        return emitFinish();

    dstDir.cd(Files::DEPLOYED_ROOT_DIR_NAME);
    auto contribDir = dstDir;
    contribDir.cd(Files::CONTRIB_DIR_NAME);
    auto contribBinPath = contribDir.absoluteFilePath(Files::BIN_DIR_NAME);

    if (library.state == Library::BinaryArchive) {
        auto binariesArchivePath = srcDir.absoluteFilePath(Files::BINARY_ARCHIVE_NAME);
        ProgressManager::invokeShow("Распаковка бинарного архива...", "Распаковано файлов");
        manager->unarchive(binariesArchivePath, contribBinPath);
        if (!manager->run())
            return emitFinish();
        manager->reset();
    } else {
        if (!compileSources(dstDir))
            return emitFinish();
    }

    contribDir.cd(Files::BIN_DIR_NAME);
    dstDir.cdUp();
    QFile::link(contribDir.absoluteFilePath(Files::EDITOR_PROJECT_NAME + ".exe"),
                dstDir.absoluteFilePath(Files::EDITOR_LINK_NAME));

    emitFinish();
}

bool LibraryDeployer::unarchive(QDir srcDir)
{
    auto archivePath = srcDir.absoluteFilePath(Files::GAMEBASE_ARCHIVE_NAME);

    QDir dstDir(workingDir.path);
    if (dstDir.exists(GAMEBASE_UNARCHIVED_DIR)) {
        ProgressManager::invokeShow("Удаление временных файлов...", "Удалено файлов");
        manager->remove(GAMEBASE_UNARCHIVED_DIR);
        if (!manager->run())
            return false;
        manager->reset();
    }

    auto tmpDir = dstDir;
    if (!tmpDir.mkdir(GAMEBASE_UNARCHIVED_DIR)
        || !tmpDir.cd(GAMEBASE_UNARCHIVED_DIR))
        return false;

    ProgressManager::invokeShow("Распаковка корня пакета...", "Распаковано файлов");
    manager->unarchive(archivePath, GAMEBASE_UNARCHIVED_DIR);
    if (!manager->run())
        return false;
    manager->reset();

    if (!QFile::rename(tmpDir.absoluteFilePath(Files::SOURCES_DIR_NAME),
                       dstDir.absoluteFilePath(Files::DEPLOYED_ROOT_DIR_NAME)))
        return false;
    dstDir.cd(Files::DEPLOYED_ROOT_DIR_NAME);

    auto packageDir = dstDir;
    packageDir.cd(Files::PACKAGE_DIR_NAME);

    dstDir.mkdir(Files::CONTRIB_DIR_NAME);
    auto contribDir = dstDir;
    contribDir.cd(Files::CONTRIB_DIR_NAME);
    auto contribBinPath = contribDir.absoluteFilePath(Files::BIN_DIR_NAME);

    if (tmpDir.exists(Files::BIN_DIR_NAME)) {
        ProgressManager::invokeShow("Копирование бинарных файлов...", "Распаковано файлов");
        manager->unarchive(packageDir.absoluteFilePath(Files::BIN_DIR_NAME + ".zip"),
                           contribBinPath);
    }

    ProgressManager::invokeShow("Распакова пакета...", "Распаковано файлов");
    manager->unarchive(packageDir.absoluteFilePath(Files::BIN_DIR_NAME + ".zip"),
                       contribBinPath);
    manager->unarchive(packageDir.absoluteFilePath(Files::INCLUDE_DIR_NAME + ".zip"),
                       contribDir.absoluteFilePath(Files::INCLUDE_DIR_NAME));
    auto resourcesDir = dstDir;
    resourcesDir.cd(Files::RESOURCES_DIR_NAME);
    manager->unarchive(packageDir.absoluteFilePath(Files::FONTS_DIR_NAME + ".zip"),
                       resourcesDir.absoluteFilePath(Files::FONTS_DIR_NAME));
    resourcesDir.cd(Files::DESIGNS_DIR_NAME);
    manager->copyFiles(resourcesDir.absoluteFilePath(Files::EDITOR_PROJECT_NAME),
                       contribBinPath);
    QDir contribBinDir(contribBinPath);
    manager->copy(packageDir.absoluteFilePath("config.json"),
                  contribBinDir.absoluteFilePath(Files::APP_CONFIG_NAME));

    if (!manager->run())
        return false;
    manager->reset();

    return true;
}

bool LibraryDeployer::compileSources(QDir dir)
{
    dir.cd(Files::SOURCES_DIR_NAME);
    dir.cd(Files::GAMEBASE_PROJECT_NAME);
    if (!compiler->compile(dir))
        return false;
    dir.cdUp();
    dir.cd(Files::EDITOR_PROJECT_NAME);
    if (!compiler->compile(dir))
        return false;
    return true;
}

void LibraryDeployer::emitFinish()
{
    emit finishedDeploy(library.afterAction(Library::Deploy));
}
