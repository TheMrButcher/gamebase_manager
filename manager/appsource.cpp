#include "appsource.h"

#include <QDir>
#include <QHash>

uint qHash(const AppSource& s, uint seed)
{
    return qHash(s.path, seed);
}

SourceStatus AppSource::check()
{
    QDir dir(path);
    if (!dir.exists())
        status = SourceStatus::Broken;
    else
        status = SourceStatus::OK;
    return status;
}
