#include "archive.h"
#include "files.h"
#include <QTemporaryFile>
#include <JlCompress.h>

#include <QDebug>

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
    if (entries.size() != 1)
        return true;
    auto result = entries.first().name;
    if (!rootDir->cd(result))
        return true;
    return true;
}

QString Archive::rootName() const
{
    if (!rootDir)
        return QString();
    return rootDir->dirName();
}

const QuaZipDir& Archive::root() const
{
    return *rootDir;
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

QString Archive::rootName(QString archiveName)
{
    Archive archive(archiveName);
    if (!archive.open())
        return QString();
    return archive.rootName();
}

Version Archive::extractVersion(QString archiveName)
{
    Archive archive(archiveName);
    if (!archive.open())
        return Version{};
    return archive.exctractVersion();
}

QString Archive::findVersionFile()
{
    const auto& dir = root();
    if (dir.exists(Files::VERSION_FILE_NAME)) {
        if (rootName() == ".")
            return Files::VERSION_FILE_NAME;
        else
            return dir.filePath(Files::VERSION_FILE_NAME);
    }
    return QString();
}
