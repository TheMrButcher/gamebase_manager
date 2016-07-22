#include "appsourcemanagerlist.h"
#include <QSet>

AppSourceManagerList::AppSourceManagerList(QWidget* parent)
    : parent(parent)
{}

void AppSourceManagerList::set(const QList<AppSource>& sources)
{
    foreach (const auto& source, sources) {
        if (!managers.contains(source))
            insert(source);
    }

    auto sourcesSet = QSet<AppSource>::fromList(sources);
    foreach (const auto& source, managers.keys()) {
        if (!sourcesSet.contains(source)) {
            auto it = managers.find(source);
            delete it.value();
            managers.erase(it);
        }
    }
}

void AppSourceManagerList::update()
{
    foreach (auto manager, managers.values())
        manager->update();
}

void AppSourceManagerList::insert(const AppSource& source)
{
    auto manager = new AppSourceManager(source, parent);
    managers[source] = manager;
    connect(manager, SIGNAL(finishedUpdate(AppSource,QList<App>)),
            this, SIGNAL(finishedUpdate(AppSource,QList<App>)));
}
