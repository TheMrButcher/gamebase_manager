#include "librarysource.h"

#include <QDir>
#include <QHash>

uint qHash(const LibrarySource& s, uint seed)
{
    return qHash(s.path, seed) ^ qHash(static_cast<int>(s.type), seed);
}

SourceStatus LibrarySource::check()
{
    if (type == Server)
        return status;
    QDir dir(path);
    if (!dir.exists())
        status = SourceStatus::Broken;
    else
        status = SourceStatus::OK;
    return status;
}
