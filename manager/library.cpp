#include "library.h"
#include "settings.h"
#include "files.h"
#include <JlCompress.h>
#include <quazip.h>
#include <quazipdir.h>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>

namespace {
Library checkDeployed(const LibrarySource& source, QDir dir)
{
    Version version;
    if (!version.read(dir.absoluteFilePath("VERSION.txt")))
        return Library::makeAbsent(source);

    if (!Files::existsDir(dir, "src") || !Files::existsDir(dir, "resources"))
        return Library::makeAbsent(source);

    dir.cdUp();
    if (!Files::exists(dir, "Editor.exe"))
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

Library Library::fromFileSystem(const LibrarySource& source, QString name)
{
    QDir dir(source.path);
    if (!dir.exists())
        return makeAbsent(source);
    if (!name.isEmpty()) {
        if (!dir.cd(name))
            return makeAbsent(source);
    }
    if (dir.cd("gamebase")) {
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
