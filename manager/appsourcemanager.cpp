#include "appsourcemanager.h"
#include <QWidget>
#include <QDir>

AppSourceManager::AppSourceManager(const AppSource& source, QWidget* parent)
    : QObject(parent)
    , source(source)
{}

void AppSourceManager::update()
{
    if (source.check() != SourceStatus::OK) {
        emit finishedUpdate(source, QList<App>());
        return;
    }

    QDir dir(source.path);
    auto children = source.type == AppSource::WorkingDirectory
            ? dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs)
            : dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
    QList<App> apps;
    apps.reserve(children.size());
    foreach (auto child, children) {
        auto app = App::fromFileSystem(source, child);
        if (app.state == App::Absent)
            continue;
        apps.append(app);
    }
    emit finishedUpdate(source, apps);
}
