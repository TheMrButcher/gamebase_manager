#include "appsourcemanager.h"
#include "settings.h"
#include "files.h"
#include "appadder.h"
#include <QWidget>
#include <QDir>
#include <QThreadPool>

namespace {
void addApps(const AppSource& source, const QStringList& files, QList<App>& apps)
{
    foreach (auto child, files) {
        auto app = App::fromFileSystem(source, child);
        if (app.state == App::Absent)
            continue;
        apps.append(app);
    }
}
}

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
    auto dirs = dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs);
    auto files = dir.entryList(QStringList("*.zip"), QDir::Files);

    QList<App> apps;
    apps.reserve(dirs.size() + files.size());
    addApps(source, dirs, apps);
    addApps(source, files, apps);
    emit finishedUpdate(source, apps);
}

void AppSourceManager::addToWorkingDir(App app)
{
    AppAdder* adder = new AppAdder(app);
    connect(adder, SIGNAL(finishedAdd(App)), this, SIGNAL(finishedAdd(App)));
    QThreadPool::globalInstance()->start(adder);
}
