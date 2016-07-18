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
        Remove,
        Deploy,
        Install
    };

    bool validate();
    bool checkAbility(Ability ability) const;
    Library afterAction(Ability ability) const;
    void remove();

    static Library fromFileSystem(const LibrarySource& source, QString name = "");
    static Library makeAbsent(const LibrarySource& source);
    static Library makeAbsent();
    static void removeDeployedFiles(QString path);
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
