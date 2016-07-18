#ifndef ILIBRARYSOURCEMANAGER_H
#define ILIBRARYSOURCEMANAGER_H

#include "librarysource.h"
#include "library.h"
#include <QObject>

class LibrarySourceManager : public QObject
{
    Q_OBJECT

public:
    LibrarySourceManager(QObject* parent)
        : QObject(parent)
    {}

    static LibrarySourceManager* create(const LibrarySource& source, QObject* parent = nullptr);

    virtual void update() = 0;
    virtual void download(const Library& library) = 0;

signals:
    void finishedUpdate(LibrarySource source, const QList<Library>& libraries);
    void finishedDownload(Library library);
};

#endif // ILIBRARYSOURCEMANAGER_H
