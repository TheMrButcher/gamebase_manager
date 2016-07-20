#include "librarysourcemanagerlist.h"
#include <QSet>
#include <QWidget>

LibrarySourceManagerList::LibrarySourceManagerList(QWidget* parent)
    : QObject(parent)
    , parent(parent)
{}

void LibrarySourceManagerList::set(const QList<LibrarySource>& sources)
{
    foreach (const auto& source, sources) {
        if (!managers.contains(source))
            insert(source);
    }

    auto sourcesSet = QSet<LibrarySource>::fromList(sources);
    foreach (const auto& source, managers.keys()) {
        if (!sourcesSet.contains(source)) {
            auto it = managers.find(source);
            delete it.value();
            managers.erase(it);
        }
    }
}

void LibrarySourceManagerList::update()
{
    foreach (auto manager, managers.values())
        manager->update();
}

void LibrarySourceManagerList::fastUpdate()
{
    foreach (auto source, managers.keys()) {
        if (source.type != LibrarySource::Server)
            managers[source]->update();
    }
}

void LibrarySourceManagerList::download(const Library& library)
{
    auto it = managers.find(library.source);
    if (it == managers.end()) {
        emit finishedDownload(library.afterAction(Library::Download));
        return;
    }
    it.value()->download(library);
}

void LibrarySourceManagerList::insert(const LibrarySource& source)
{
    auto manager = LibrarySourceManager::create(source, parent);
    if (!manager)
        return;
    managers[source] = manager;
    connect(manager, SIGNAL(finishedUpdate(LibrarySource,QList<Library>)),
            this, SIGNAL(finishedUpdate(LibrarySource,QList<Library>)));
    connect(manager, SIGNAL(finishedDownload(Library)),
            this, SIGNAL(finishedDownload(Library)));
}
