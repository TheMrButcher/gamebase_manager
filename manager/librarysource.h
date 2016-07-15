#ifndef LIBRARYSOURCE_H
#define LIBRARYSOURCE_H

#include "sourcestatus.h"
#include <QString>

class LibrarySource
{
public:
    enum Type {
        Server,
        Directory
    };

    Type type;
    QString path;
    SourceStatus status;
};

#endif // LIBRARYSOURCE_H
