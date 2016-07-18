#include "library.h"
#include "settings.h"
#include "files.h"
#include <JlCompress.h>
#include <quazip.h>
#include <quazipdir.h>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QHash>

namespace {
Library checkDeployed(const LibrarySource& source, QDir dir)
{
    Version version;
    if (!version.read(dir.absoluteFilePath(Files::VERSION_FILE_NAME)))
        return Library::makeAbsent(source);

    if (!Files::existsDir(dir, "src") || !Files::existsDir(dir, "resources"))
        return Library::makeAbsent(source);

    dir.cdUp();
    if (!Files::exists(dir, Files::EDITOR_LINK_NAME))
        return Library::makeAbsent(source);
    return Library{ source, Library::Deployed, version, QString() };
}

QString findVersionFile(QString archiveName)
{
    QuaZip archive(archiveName);
    if (!archive.open(QuaZip::mdUnzip))
        return QString();
    QuaZipDir dir(&archive);
    auto entries = dir.entryInfoList(QDir::Dirs);
    if (entries.size() != 1)
        return QString();
    if (!dir.cd(entries.first().name))
        return QString();
    if (dir.exists(Files::VERSION_FILE_NAME))
        return dir.filePath(Files::VERSION_FILE_NAME);
    return QString();
}

Version exctractVersion(QString archiveName)
{
    auto versionFilePath = findVersionFile(archiveName);
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

Library checkArchives(const LibrarySource& source, QDir dir)
{
    if (!Files::exists(dir, Files::SOURCES_ARCHIVE_NAME))
        return Library::makeAbsent(source);
    Library::State state = Library::SourceCode;
    if (Files::exists(dir, Files::BINARY_ARCHIVE_NAME))
        state = Library::BinaryArchive;
    Version version = exctractVersion(dir.absoluteFilePath(Files::SOURCES_ARCHIVE_NAME));
    if (version.empty())
        return Library::makeAbsent(source);
    return Library{ source, state, version, dir.dirName() };
}
}

bool Library::checkAbility(Library::Ability ability) const
{
    switch (ability)
    {
    case Download: return source.type != LibrarySource::DownloadsDirectory
                && state != Absent;
    case Remove: return source.type != LibrarySource::Server
                && state != Absent;
    default: return false;
    }
}

void Library::remove()
{
    if (!checkAbility(Remove))
        return;
    switch (state) {
    case BinaryArchive:
    case SourceCode:
    {
        QDir dir(source.path);
        if (!dir.cd(archiveName))
            return;
        dir.removeRecursively();
        break;
    }

    case Deployed:
    {
        QDir dir(source.path);
        dir.remove(Files::EDITOR_LINK_NAME);
        if (!dir.cd(Files::DEPLOYED_ROOT_DIR_NAME))
            return;
        dir.removeRecursively();
        break;
    }

    default: return;
    }
}

Library Library::fromFileSystem(const LibrarySource& source, QString name)
{
    QDir dir(source.path);
    if (!dir.exists())
        return makeAbsent(source);
    if (!name.isEmpty()) {
        if (!dir.cd(name))
            return makeAbsent(source);
    }
    if (dir.cd(Files::DEPLOYED_ROOT_DIR_NAME)) {
        auto result = checkDeployed(source, dir);
        if (result.state != Library::Absent)
            return result;
    }
    auto result = checkArchives(source, dir);
    if (result.state != Library::Absent)
        return result;
    return makeAbsent(source);
}

Library Library::makeAbsent(const LibrarySource& source)
{
    return Library{
        source, Library::Absent, Version{}, QString() };
}

uint qHash(const Library& lib, uint seed)
{
    return qHash(lib.source, seed) ^ qHash(lib.version.toString(), seed)
            ^ qHash(lib.archiveName, seed) ^ qHash(static_cast<int>(lib.state), seed);
}
