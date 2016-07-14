#include "version.h"
#include <QFile>
#include <QTextStream>

QString Version::toString() const
{
    return version.join('.');
}

Version Version::read(QString fileName)
{
    QFile versionFile(fileName);
    versionFile.open(QIODevice::ReadOnly);
    QTextStream stream(&versionFile);
    auto versionStr = stream.readAll();
    return Version{ versionStr.split('.', QString::SkipEmptyParts) };
}

bool operator==(const Version& v1, const Version& v2)
{
    return v1.version.size() == v2.version.size()
            && std::equal(v1.version.begin(), v1.version.end(), v2.version.begin());
}

bool operator<(const Version& v1, const Version& v2)
{
    return std::lexicographical_compare(
                v1.version.begin(), v1.version.end(),
                v2.version.begin(), v2.version.end());
}
