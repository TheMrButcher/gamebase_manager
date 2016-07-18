#ifndef LIBRARY_H
#define LIBRARY_H

#include "librarysource.h"
#include "version.h"

class Library
{
public:
    LibrarySource source;

    enum State {
        Absent,
        SourceCode,
        BinaryArchive,
        Deployed
    };
    State state;
    Version version;
    QString archiveName;

    static Library fromFileSystem(const LibrarySource& source, QString name = "");
    static Library makeAbsent(const LibrarySource& source);
};

#endif // LIBRARY_H
