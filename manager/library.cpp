#include "library.h"
#include "settings.h"
#include "files.h"
#include "archive.h"
#include <JlCompress.h>
#include <quazip.h>
#include <quazipdir.h>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>
#include <QHash>

#include <QDebug>

namespace {
const QString BINARY_RESULT_DIR_PREFIX = "Gamebase_Binary_";
const QString SOURCES_RESULT_DIR_PREFIX = "Gamebase_Sources_";

Library checkDeployed(const LibrarySource& source, QDir dir)
{
    Version version;
    if (!version.read(dir.absoluteFilePath(Files::VERSION_FILE_NAME)))
        return Library::makeAbsent(source);

    if (!Files::existsDir(dir, "src") || !Files::existsDir(dir, Files::RESOURCES_DIR_NAME))
        return Library::makeAbsent(source);

    dir.cdUp();
    if (!Files::exists(dir, Files::EDITOR_LINK_NAME))
        return Library::makeAbsent(source);

    return Library{ source, Library::Deployed, version, QString() };
}

QString findVersionFile(QString archiveName)
{
    Archive archive(archiveName);
    if (!archive.open())
        return QString();
    auto dir = archive.root();
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

bool Library::validate()
{
    if (source.type == LibrarySource::Server)
        return false;
    Library expected = *this;
    *this = fromFileSystem(source, archiveName);
    return state != Absent && expected == *this;
}

bool Library::checkAbility(Library::Ability ability) const
{
    switch (ability)
    {
    case Download: return source.type != LibrarySource::DownloadsDirectory
                && state != Absent;
    case Remove: return source.type != LibrarySource::Server
                && state != Absent;
    case Deploy: return source.type == LibrarySource::DownloadsDirectory
                && state == BinaryArchive;
    case Install: return source.type != LibrarySource::WorkingDirectory
                && state != Absent;
    default: return false;
    }
}

Library Library::afterAction(Library::Ability ability) const
{
    if (!checkAbility(ability))
        return makeAbsent(source);
    switch (ability)
    {
    case Download:
    {
        QString prefix;
        switch (state) {
        case SourceCode:
            prefix = SOURCES_RESULT_DIR_PREFIX;
            break;

        case BinaryArchive:
        case Deployed:
            prefix = BINARY_RESULT_DIR_PREFIX;
            break;

        default:
            return makeAbsent(source);
        }
        State newState = state == Deployed ? BinaryArchive : state;
        auto suffix = version.toString().replace('.', '_');
        auto dirName = prefix + suffix;
        return Library{ Settings::instance().downloadsDir(),
                    newState, version, dirName };
    }
    case Deploy:
    case Install:
        return Library{ Settings::instance().workingDir(),
                    Deployed, version, "" };
    default: return makeAbsent(source);
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
        removeDeployedFiles(source.path);
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

Library Library::makeAbsent()
{
    return Library::makeAbsent(
        LibrarySource{ LibrarySource::Directory, "", SourceStatus::Broken });
}

void Library::removeDeployedFiles(QString path)
{
    QDir dir(path);
    dir.remove(Files::EDITOR_LINK_NAME);
    if (!dir.cd(Files::DEPLOYED_ROOT_DIR_NAME))
        return;
    dir.removeRecursively();
}

uint qHash(const Library& lib, uint seed)
{
    return qHash(lib.source, seed) ^ qHash(lib.version.toString(), seed)
            ^ qHash(lib.archiveName, seed) ^ qHash(static_cast<int>(lib.state), seed);
}
