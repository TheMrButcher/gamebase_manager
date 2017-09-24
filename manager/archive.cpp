#include "archive.h"
#include "files.h"
#include <QTemporaryFile>
#include <JlCompress.h>

Archive::Archive(QString archiveName)
    : archiveName(archiveName)
    , zipFile(archiveName)
    , rootDir(nullptr)
{}

Archive::~Archive()
{
    if (rootDir)
        delete rootDir;
}

bool Archive::open()
{
    if (!zipFile.open(QuaZip::mdUnzip))
        return false;
    rootDir = new QuaZipDir(&zipFile);
    myVersion = exctractVersion();
    if (myVersion.empty())
        return false;

    compiledVersion = false;
    auto dir = *rootDir;
    if (dir.cd(Files::CONTRIB_DIR_NAME)) {
        auto binDir = dir;
        if (binDir.cd(Files::BIN_DIR_NAME)) {
            auto debugDir = binDir;
            bool hasDebugFiles = false;
            if (debugDir.cd(Files::DEBUG_DIR_NAME)) {
                hasDebugFiles = debugDir.exists(Files::GAMEBASE_PROJECT_NAME + ".dll")
                        && debugDir.exists(Files::GAMEBASE_PROJECT_NAME + ".lib")
                        && debugDir.exists(Files::GAMEBASE_PROJECT_NAME + ".pdb");
            }

            auto releaseDir = binDir;
            bool hasReleaseFiles = false;
            if (releaseDir.cd(Files::RELEASE_DIR_NAME)) {
                hasReleaseFiles = releaseDir.exists(Files::GAMEBASE_PROJECT_NAME + ".dll")
                        && releaseDir.exists(Files::GAMEBASE_PROJECT_NAME + ".lib")
                        && releaseDir.exists(Files::EDITOR_PROJECT_NAME + ".exe");

            }
            compiledVersion = hasDebugFiles + hasReleaseFiles;
        }
    }
    return true;
}

const QuaZipDir& Archive::root() const
{
    return *rootDir;
}

bool Archive::isCompiledVersion()
{
    return compiledVersion;
}

const Version &Archive::version() const
{
    return myVersion;
}

QString Archive::findVersionFile()
{
    auto dir = root();
    if (dir.exists(Files::VERSION_FILE_NAME))
        return Files::VERSION_FILE_NAME;
    return QString();
}

Version Archive::exctractVersion()
{
    auto versionFilePath = findVersionFile();
    if (versionFilePath.isEmpty())
        return Version{};

    QTemporaryFile tempFile;
    if (!tempFile.open())
        return Version{};
    auto tempFileName = tempFile.fileName();
    JlCompress::extractFile(archiveName, versionFilePath, tempFileName);

    Version version;
    if (!version.read(tempFileName))
        return Version{};
    return version;
}
