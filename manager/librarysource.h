#ifndef LIBRARYSOURCE_H
#define LIBRARYSOURCE_H

#include "sourcestatus.h"
#include <QString>

class LibrarySource
{
public:
    enum Type {
        None,
        Server,
        Directory,
        DownloadsDirectory,
        WorkingDirectory
    };

    Type type;
    QString path;
    SourceStatus status;

    SourceStatus check();
};

inline bool operator==(const LibrarySource& s1, const LibrarySource& s2)
{
    return s1.path == s2.path && s1.type == s2.type;
}

uint qHash(const LibrarySource& s, uint seed);

#endif // LIBRARYSOURCE_H
