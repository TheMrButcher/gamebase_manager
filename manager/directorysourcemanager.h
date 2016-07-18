#ifndef DIRECTORYSOURCEMANAGER_H
#define DIRECTORYSOURCEMANAGER_H

#include "librarysourcemanager.h"

class DirectorySourceManager : public LibrarySourceManager
{
public:
    DirectorySourceManager(const LibrarySource& source, QObject* parent);

    virtual void update() override;
    virtual void download(const Library& library) override;

private:
    LibrarySource source;
};

#endif // DIRECTORYSOURCEMANAGER_H
