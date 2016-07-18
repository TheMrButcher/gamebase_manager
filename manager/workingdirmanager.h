#ifndef WORKINGDIRMANAGER_H
#define WORKINGDIRMANAGER_H

#include "librarysourcemanager.h"

class WorkingDirManager : public LibrarySourceManager
{
public:
    WorkingDirManager(const LibrarySource& source, QObject* parent);

    virtual void update() override;
    virtual void download(const Library& library) override;

private:
    LibrarySource source;
};

#endif // WORKINGDIRMANAGER_H
