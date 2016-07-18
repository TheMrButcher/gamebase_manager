#include "archive.h"

Archive::Archive(QString archiveName)
    : zipFile(archiveName)
    , rootDir(nullptr)
{}

Archive::~Archive()
{
    if (rootDir)
        delete rootDir;
}

bool Archive::open()
{
    if (!zipFile.open(QuaZip::mdUnzip))
        return false;
    rootDir = new QuaZipDir(&zipFile);
    auto entries = rootDir->entryInfoList(QDir::Dirs);
    if (entries.size() != 1)
        return false;
    auto result = entries.first().name;
    if (!rootDir->cd(result))
        return false;
    return true;
}

QString Archive::rootName() const
{
    if (!rootDir)
        return QString();
    return rootDir->dirName();
}

const QuaZipDir& Archive::root() const
{
    return *rootDir;
}

QString Archive::rootName(QString archiveName)
{
    Archive archive(archiveName);
    if (!archive.open())
        return QString();
    return archive.rootName();
}
