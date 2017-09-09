#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "version.h"
#include <quazip.h>
#include <quazipdir.h>

class Archive
{
public:
    Archive(QString archiveName);
    ~Archive();
    bool open();
    QString rootName() const;
    const QuaZipDir& root() const;
    bool isCompiledVersion();
    const Version& version() const;

private:
    QString findVersionFile();
    Version exctractVersion();

    QString archiveName;
    QuaZip zipFile;
    QuaZipDir* rootDir;
    Version myVersion;
    bool compiledVersion;
};

#endif // ARCHIVE_H
