#include "librarydeployer.h"
#include "settings.h"
#include "files.h"
#include "archive.h"
#include <JlCompress.h>
#include <QWidget>
#include <QDir>
#include <QProcess>

#include <QThread>
#include <QDebug>

namespace {
const QString COMPILATION_BATCH_NAME = "compile.bat";

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

LibraryDeployer::LibraryDeployer(const Library& library)
    : library(library)
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

    QDir dstDir(workingDir.path);
    if (!unarchiveSources(srcDir, dstDir))
        return emitFinish();

    dstDir.cd(Files::DEPLOYED_ROOT_DIR_NAME);
    auto contribDir = dstDir;
    contribDir.cd(Files::CONTRIB_DIR_NAME);
    auto contribBinPath = contribDir.absoluteFilePath(Files::BIN_DIR_NAME);

    if (library.state == Library::BinaryArchive) {
        auto binariesArchivePath = srcDir.absoluteFilePath(Files::BINARY_ARCHIVE_NAME);
        JlCompress::extractDir(binariesArchivePath, contribBinPath);
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

bool LibraryDeployer::unarchiveSources(QDir srcDir, QDir dstDir)
{
    auto sourcesArchivePath = srcDir.absoluteFilePath(Files::SOURCES_ARCHIVE_NAME);
    auto unzippedArchiveDirName = Archive::rootName(sourcesArchivePath);
    if (dstDir.exists(unzippedArchiveDirName)) {
        auto oldArchiveDir = dstDir;
        oldArchiveDir.cd(unzippedArchiveDirName);
        oldArchiveDir.removeRecursively();
    }
    JlCompress::extractDir(sourcesArchivePath, workingDir.path);

    if (dstDir.cd(unzippedArchiveDirName))
        dstDir.cdUp();
    else
        return false;
    if (!dstDir.rename(unzippedArchiveDirName, Files::DEPLOYED_ROOT_DIR_NAME))
        return false;
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
    return true;
}

bool LibraryDeployer::compileSources(QDir dir)
{
    qDebug() << "Started compilation";
    dir.cd(Files::SOURCES_DIR_NAME);
    dir.cd(Files::GAMEBASE_PROJECT_NAME);
    if (!compileProject(dir))
        return false;
    dir.cdUp();
    dir.cd(Files::EDITOR_PROJECT_NAME);
    if (!compileProject(dir))
        return false;
    return true;
}

bool LibraryDeployer::compileProject(QDir projectDir)
{
    QFile::copy(":/scripts/compile.bat", projectDir.absoluteFilePath(COMPILATION_BATCH_NAME));

    QProcess cmdProcess;
    cmdProcess.setWorkingDirectory(projectDir.absolutePath());
    cmdProcess.setProcessChannelMode(QProcess::MergedChannels);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    auto vcVarsPath = Settings::instance().vcVarsPath;
    if (vcVarsPath.isEmpty())
        return false;
    env.insert("VISUAL_CPP_VARIABLES_PATH", vcVarsPath);
    env.insert("SOLUTION_TO_BUILD_NAME", projectDir.dirName() + ".sln");
    cmdProcess.setProcessEnvironment(env);

    QStringList arguments;
    arguments << "/U" << "/C" << COMPILATION_BATCH_NAME;
    cmdProcess.start("cmd.exe", arguments);
    qDebug() << "Started process";
    if (!cmdProcess.waitForStarted(5000))
        return false;
    while (cmdProcess.state() == QProcess::Running) {
        while (cmdProcess.waitForReadyRead(2000)) {
            while (cmdProcess.canReadLine())
                qDebug() << QString::fromUtf8(cmdProcess.readLine());
        }
    }
    while (cmdProcess.canReadLine())
        qDebug() << QString::fromUtf8(cmdProcess.readLine());
    return cmdProcess.exitCode() == 0;
}

void LibraryDeployer::emitFinish()
{
    emit finishedDeploy(library.afterAction(Library::Deploy));
}
