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
    Version exctractVersion();

    static QString rootName(QString archiveName);
    static Version extractVersion(QString archiveName);

private:
    QString findVersionFile();

    QString archiveName;
    QuaZip zipFile;
    QuaZipDir* rootDir;
};

#endif // ARCHIVE_H
