#ifndef LIBRARYSOURCEMANAGERLIST_H
#define LIBRARYSOURCEMANAGERLIST_H

#include "librarysourcemanager.h"
#include <QObject>
#include <QHash>

class QWidget;

class LibrarySourceManagerList : public QObject
{
    Q_OBJECT
public:
    explicit LibrarySourceManagerList(QWidget* parent);

    static LibrarySourceManagerList* instance();

    void set(const QList<LibrarySource>& sources);
    void update();
    void fastUpdate();
    void download(const Library& library);

signals:
    void finishedUpdate(LibrarySource source, const QList<Library>& libraries);
    void finishedDownload(Library library);

private:
    void insert(const LibrarySource& source);

    QHash<LibrarySource, LibrarySourceManager*> managers;
    QWidget* parent;
};

#endif // LIBRARYSOURCEMANAGERLIST_H
