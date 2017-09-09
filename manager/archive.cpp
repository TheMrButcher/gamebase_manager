#include "archive.h"
#include "files.h"
#include <QTemporaryFile>
#include <JlCompress.h>
#include <QSet>

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
    auto entries = rootDir->entryInfoList(QDir::Dirs);
    if (entries.size() == 1) {
        auto srcDirName = entries.first().name;
        if (srcDirName != Files::SOURCES_DIR_NAME)
            return false;
        compiledVersion = false;
    } else {
        QSet<QString> subDirNames;
        for (const auto& entry : entries)
            subDirNames.insert(entry.name);
        if (subDirNames.size() != 2
            || !subDirNames.contains(Files::SOURCES_DIR_NAME)
            || !subDirNames.contains(Files::BIN_DIR_NAME))
            return false;
        compiledVersion = true;
    }
    myVersion = exctractVersion();
    return !myVersion.empty();
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
    if (!dir.cd(Files::SOURCES_DIR_NAME))
        return QString();
    if (dir.exists(Files::VERSION_FILE_NAME))
        return dir.filePath(Files::VERSION_FILE_NAME);
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
