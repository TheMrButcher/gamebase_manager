#include "library.h"
#include "settings.h"

#include <QDir>
#include <QFile>

namespace {
bool isCurrent(const LibrarySource& source)
{
    return Settings::instance().workingDir == source.path;
}

Library makeAbsent(const LibrarySource& source)
{
    return Library{
        source, isCurrent(source), Library::Absent, Version{}, QString() };
}

Library checkDeployed(const LibrarySource& source, QDir dir)
{
    Version version;
    if (!version.read(dir.absoluteFilePath("VERSION.txt")))
        return makeAbsent(source);

    if (dir.cd("src"))
        dir.cdUp();
    else
        return makeAbsent(source);

    if (dir.cd("resources"))
        dir.cdUp();
    else
        return makeAbsent(source);

    dir.cdUp();
    if (!QFile(dir.absoluteFilePath("Editor.exe")).exists())
        return makeAbsent(source);
    return Library{ source, isCurrent(source), Library::Deployed, version, QString() };
}
}

Library Library::fromDirectory(QString path)
{
    LibrarySource source{ LibrarySource::Directory, path, SourceStatus::Unknown };

    QDir dir(path);
    if (!dir.exists())
        return makeAbsent(source);
    if (dir.cd("gamebase")) {
        auto result = checkDeployed(source, dir);
        if (result.state != Absent)
            return result;
    }
    return makeAbsent(source);
}
