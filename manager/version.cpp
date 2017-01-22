#include "version.h"
#include <QFile>
#include <QTextStream>

QString Version::toString() const
{
    return toStringList().join('.');
}

QStringList Version::toStringList() const
{
    QStringList versionStrParts;
    versionStrParts.reserve(version.size());
    foreach (int num, version)
        versionStrParts.append(QString::number(num));
    return versionStrParts;
}

void Version::set(QString versionStr)
{
    QStringList versionStrParts = versionStr.split('.', QString::SkipEmptyParts);
    version.clear();
    version.reserve(versionStrParts.size());
    foreach (auto str, versionStrParts)
        version.append(str.toInt());
}

bool Version::read(QString fileName)
{
    QFile versionFile(fileName);
    if (!versionFile.open(QIODevice::ReadOnly))
        return false;
    QTextStream stream(&versionFile);
    auto versionStr = stream.readAll();
    set(versionStr);
    return true;
}

bool Version::write(QString fileName)
{
    QFile versionFile(fileName);
    if (!versionFile.open(QIODevice::WriteOnly))
        return false;
    versionFile.write(toString().toUtf8());
    return true;
}

Version Version::fromString(QString versionStr)
{
    Version version;
    version.set(versionStr);
    return version;
}

Version Version::fromFile(QString path)
{
    Version version;
    version.read(path);
    return version;
}

Version Version::selfVersion()
{
    static const Version MANAGER_VERSION = fromFile(":/common/VERSION.txt");
    return MANAGER_VERSION;
}

bool operator==(const Version& v1, const Version& v2)
{
    return v1.version == v2.version;
}

bool operator!=(const Version& v1, const Version& v2)
{
    return v1.version != v2.version;
}

bool operator<(const Version& v1, const Version& v2)
{
    return std::lexicographical_compare(
                v1.version.begin(), v1.version.end(),
                v2.version.begin(), v2.version.end());
}

bool operator<=(const Version& v1, const Version& v2)
{
    return !(v2 < v1);
}

uint qHash(const Version& version, uint seed)
{
    return qHash(version.toString(), seed);
}
