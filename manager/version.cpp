#include "version.h"
#include <QFile>
#include <QTextStream>

QString Version::toString() const
{
    return version.join('.');
}

void Version::set(QString versionStr)
{
    version = versionStr.split('.', QString::SkipEmptyParts);
}

bool Version::read(QString fileName)
{
    QFile versionFile(fileName);
    if (!versionFile.exists())
        return false;
    versionFile.open(QIODevice::ReadOnly);
    QTextStream stream(&versionFile);
    auto versionStr = stream.readAll();
    set(versionStr);
    return true;
}

Version Version::fromString(QString versionStr)
{
    Version version;
    version.set(versionStr);
    return version;
}

bool operator==(const Version& v1, const Version& v2)
{
    return v1.version == v2.version;
}

bool operator<(const Version& v1, const Version& v2)
{
    return std::lexicographical_compare(
                v1.version.begin(), v1.version.end(),
                v2.version.begin(), v2.version.end());
}
