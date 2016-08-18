#include "appsourcemanager.h"
#include "settings.h"
#include "files.h"
#include <QWidget>
#include <QDir>

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
    auto workingDir = Settings::instance().workingDir();
    auto resultApp = app.afterAction(App::Add);
    if (workingDir.check() != SourceStatus::OK) {
        emit finishedAdd(resultApp);
        return;
    }

    if (app.source.check() != SourceStatus::OK) {
        emit finishedAdd(resultApp);
        return;
    }

    QDir srcDir(app.source.path);
    if (!srcDir.exists(app.containerName)) {
        emit finishedAdd(resultApp);
        return;
    }

    QDir dstDir(workingDir.path);
    if (resultApp.containerName.isEmpty() || dstDir.exists(resultApp.containerName)) {
        emit finishedAdd(resultApp);
        return;
    }

    if (app.state == App::NotConfigured || app.state == App::Full) {
        if (!srcDir.cd(app.containerName)) {
            emit finishedAdd(resultApp);
            return;
        }

        if (!dstDir.mkdir(resultApp.containerName)) {
            emit finishedAdd(resultApp);
            return;
        }
        dstDir.cd(resultApp.containerName);
        Files::copyDir(srcDir, dstDir);
    }

    emit finishedAdd(resultApp);
}
