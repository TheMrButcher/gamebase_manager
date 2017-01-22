#include "updatedescriptor.h"

namespace {
const QString UPDATE_PREFIX = "UpdateGamebaseManager_";
const QString UPDATE_VERSIONS_DELIM = "_to_";
const QString UPDATE_EXTENSION = ".exe";
}

UpdateDescriptor::UpdateDescriptor(QString url, QString fileName)
    : url(url)
    , fileName(fileName)
{
    QString versions = fileName.mid(UPDATE_PREFIX.size());
    int extensionPos = versions.indexOf(UPDATE_EXTENSION);
    versions = versions.mid(0, extensionPos);
    int delimPos = versions.indexOf(UPDATE_VERSIONS_DELIM);
    from = Version::fromString(versions.mid(0, delimPos).replace('_', '.'));
    to = Version::fromString(versions.mid(delimPos + UPDATE_VERSIONS_DELIM.size()).replace('_', '.'));
}

bool UpdateDescriptor::isUpdate(QString name)
{
    return name.contains(UPDATE_PREFIX)
        && name.contains(UPDATE_VERSIONS_DELIM)
        && name.contains(UPDATE_EXTENSION);
}
