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

#include <QDebug>

namespace {
const QString GAMEBASE_UNARCHIVED_DIR = "gamebase_tmp";
const QString CONTRIB_UNARCHIVED_DIR = "contrib_tmp";

bool changeDir(QDir dir, QString dirName, QDir& dstDir)
{
    if (!dir.exists(dirName))
        return false;
    dstDir = dir;
    return dstDir.cd(dirName);
}

bool makeDir(QDir dir, QString dirName, QDir& dstDir)
{
    if (!dir.exists(dirName)) {
        if (!dir.mkdir(dirName))
            return false;
    }
    dstDir = dir;
    return dstDir.cd(dirName);
}
}

LibraryDeployer::LibraryDeployer(const Library& library)
    : library(library)
{
    workingDir = Settings::instance().workingDir();
    manager = new FilesManager(this);
    manager->setRootDirectory(workingDir.path);
    compiler = new Compiler(this);
    compiler->setLegacySolution(true);
}

void LibraryDeployer::run()
{
    QDir srcDir(library.source.path);
    if (!srcDir.cd(library.archiveName))
        return emitFinish();

    if (workingDir.check() != SourceStatus::OK)
        return emitFinish();

    QDir dstDir(workingDir.path);
    QDir rootDir;
    if (!unarchive(srcDir, rootDir))
        return emitFinish();
    if (!prepare(rootDir))
        return emitFinish();
    if (!unarchiveContrib(srcDir, rootDir))
        return emitFinish();

    for (int i = 0; i < 100; ++i) {
        if (dstDir.rename(rootDir.absolutePath(),
                          dstDir.absoluteFilePath(Files::DEPLOYED_ROOT_DIR_NAME)))
            break;
        if (i == 99) {
            qDebug() << "Failed to rename " << rootDir.absolutePath()
                     << " to " << dstDir.absoluteFilePath(Files::DEPLOYED_ROOT_DIR_NAME);
            return emitFinish();
        }
    }
    if (!changeDir(dstDir, Files::DEPLOYED_ROOT_DIR_NAME, rootDir))
        return emitFinish();

    if (library.state == Library::SourceCode) {
        if (!compileSources(rootDir)) {
            qDebug() << "Failed to compile sources";
            return emitFinish();
        }
    }

    if (!makeLink(rootDir)) {
        qDebug() << "Failed to make link";
        return emitFinish();
    }

    emitFinish();
}

bool LibraryDeployer::unarchive(QDir srcDir, QDir& rootDir)
{
    auto archivePath = srcDir.absoluteFilePath(Files::GAMEBASE_ARCHIVE_NAME);

    QDir dstDir(workingDir.path);
    if (dstDir.exists(GAMEBASE_UNARCHIVED_DIR)) {
        ProgressManager::invokeShow("Удаление временных файлов...", "Удалено файлов");
        manager->remove(GAMEBASE_UNARCHIVED_DIR);
        if (!manager->run()) {
            qDebug() << "Failed to remove " << dstDir.absoluteFilePath(GAMEBASE_UNARCHIVED_DIR);
            return false;
        }
        manager->reset();
    }

    rootDir = dstDir;
    if (!rootDir.mkdir(GAMEBASE_UNARCHIVED_DIR)
        || !rootDir.cd(GAMEBASE_UNARCHIVED_DIR))
        return false;

    ProgressManager::invokeShow("Распаковка корня пакета...", "Распаковано файлов");
    manager->unarchive(archivePath, GAMEBASE_UNARCHIVED_DIR);
    if (!manager->run()) {
        qDebug() << "Failed to unarchive library";
        return false;
    }
    manager->reset();
    return true;
}

bool LibraryDeployer::prepare(QDir rootDir)
{
    QDir packageDir, resourcesDir, fontsDir, designsDir, editorDesignDir,
            contribDir, binDir, debugDir, releaseDir, editorDir;

    ProgressManager::invokeShow("Подготовка пакета...", "Скопировано файлов");
    if (!changeDir(rootDir, Files::PACKAGE_DIR_NAME, packageDir))
        return false;
    if (!changeDir(rootDir, Files::RESOURCES_DIR_NAME, resourcesDir))
        return false;
    if (!changeDir(resourcesDir, Files::DESIGNS_DIR_NAME, designsDir))
        return false;
    if (!changeDir(designsDir, Files::EDITOR_PROJECT_NAME, editorDesignDir))
        return false;

    if (!makeDir(resourcesDir, Files::FONTS_DIR_NAME, fontsDir))
        return false;
    if (!makeDir(rootDir, Files::CONTRIB_DIR_NAME, contribDir))
        return false;
    if (!makeDir(contribDir, Files::BIN_DIR_NAME, binDir))
        return false;
    if (!makeDir(binDir, Files::DEBUG_DIR_NAME, debugDir))
        return false;
    if (!makeDir(binDir, Files::RELEASE_DIR_NAME, releaseDir))
        return false;
    if (!makeDir(binDir, Files::EDITOR_DIR_NAME, editorDir))
        return false;

    manager->unarchive(packageDir.absoluteFilePath(Files::FONTS_DIR_NAME + ".zip"),
                       fontsDir.absolutePath());
    manager->copyFiles(editorDesignDir.absolutePath(), editorDir.absolutePath());
    manager->copy(packageDir.absoluteFilePath("config.json"),
                  debugDir.absoluteFilePath(Files::APP_CONFIG_NAME));
    manager->copy(packageDir.absoluteFilePath("config.json"),
                  releaseDir.absoluteFilePath(Files::APP_CONFIG_NAME));

    if (!manager->run()) {
        qDebug() << "Failed to prepare library";
        return false;
    }
    manager->reset();

    return true;
}

bool LibraryDeployer::unarchiveContrib(QDir srcDir, QDir rootDir)
{
    QDir contribDir, binDir, debugDir, releaseDir, tmpContribDir;

    ProgressManager::invokeShow("Распаковка зависимостей...", "Распаковано файлов");
    if (!changeDir(rootDir, Files::CONTRIB_DIR_NAME, contribDir))
        return false;
    if (!changeDir(contribDir, Files::BIN_DIR_NAME, binDir))
        return false;
    if (!changeDir(binDir, Files::DEBUG_DIR_NAME, debugDir))
        return false;
    if (!changeDir(binDir, Files::RELEASE_DIR_NAME, releaseDir))
        return false;
    if (!makeDir(contribDir, CONTRIB_UNARCHIVED_DIR, tmpContribDir))
        return false;

    manager->unarchive(srcDir.absoluteFilePath(Files::CONTRIB_ARCHIVE_NAME),
                       tmpContribDir.absolutePath());
    if (!manager->run()) {
        qDebug() << "Failed to unarchive dependency files";
        return false;
    }
    manager->reset();

    ProgressManager::invokeShow("Копирование файлов зависимостей...", "Скопировано файлов");
    manager->rename(tmpContribDir.absoluteFilePath(Files::INCLUDE_DIR_NAME),
                    contribDir.absoluteFilePath(Files::INCLUDE_DIR_NAME));
    manager->moveFiles(tmpContribDir.absoluteFilePath(Files::DEBUG_DIR_NAME),
                       debugDir.absolutePath());
    manager->moveFiles(tmpContribDir.absoluteFilePath(Files::RELEASE_DIR_NAME),
                       releaseDir.absolutePath());
    if (!manager->run()) {
        qDebug() << "Failed to move dependency files";
        return false;
    }
    manager->reset();

    if (!tmpContribDir.removeRecursively()) {
        qDebug() << "Failed to remove " << tmpContribDir.absolutePath();
        return false;
    }

    return true;
}

bool LibraryDeployer::compileSources(QDir rootDir)
{
    auto dir = rootDir;
    dir.cd(Files::SOURCES_DIR_NAME);
    dir.cd(Files::GAMEBASE_PROJECT_NAME);
    if (!compiler->compile(dir, Compiler::BuildType::Debug))
        return false;
    if (!compiler->compile(dir, Compiler::BuildType::Release))
        return false;
    dir.cdUp();
    dir.cd(Files::EDITOR_PROJECT_NAME);
    if (!compiler->compile(dir, Compiler::BuildType::Release))
        return false;
    return true;
}

bool LibraryDeployer::makeLink(QDir rootDir)
{
    QDir parentDir, contribDir, binDir, releaseDir;

    parentDir = rootDir;
    if (!parentDir.cdUp())
        return false;
    if (!changeDir(rootDir, Files::CONTRIB_DIR_NAME, contribDir))
        return false;
    if (!changeDir(contribDir, Files::BIN_DIR_NAME, binDir))
        return false;
    if (!changeDir(binDir, Files::RELEASE_DIR_NAME, releaseDir))
        return false;
    if (!QFile::link(releaseDir.absoluteFilePath(Files::EDITOR_PROJECT_NAME + ".exe"),
                     parentDir.absoluteFilePath(Files::EDITOR_LINK_NAME)))
        return false;
    return true;
}

void LibraryDeployer::emitFinish()
{
    emit finishedDeploy(library.afterAction(Library::Deploy));
}
