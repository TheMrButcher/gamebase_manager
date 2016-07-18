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

    enum Ability {
        Download,
        Remove
    };

    bool checkAbility(Ability ability) const;
    void remove();

    static Library fromFileSystem(const LibrarySource& source, QString name = "");
    static Library makeAbsent(const LibrarySource& source);
};

inline operator==(const Library& lib1, const Library& lib2)
{
    return lib1.source == lib2.source
            && lib1.state == lib2.state
            && lib1.version == lib2.version
            && lib1.archiveName == lib2.archiveName;
}

uint qHash(const Library& lib, uint seed);

#endif // LIBRARY_H
