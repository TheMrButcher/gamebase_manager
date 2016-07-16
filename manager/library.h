#ifndef LIBRARY_H
#define LIBRARY_H

#include "librarysource.h"
#include "version.h"

class Library
{
public:
    LibrarySource source;
    bool isCurrent;

    enum State {
        Absent,
        SourceCode,
        BinaryArchive,
        Deployed
    };
    State state;
    Version version;
    QString archiveName;

    static Library fromDirectory(QString path);
};

#endif // LIBRARY_H
