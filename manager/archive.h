#ifndef ARCHIVE_H
#define ARCHIVE_H

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

    static QString rootName(QString archiveName);

private:
    QuaZip zipFile;
    QuaZipDir* rootDir;
};

#endif // ARCHIVE_H
