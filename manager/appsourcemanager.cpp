#include "appsourcemanager.h"
#include "settings.h"
#include "files.h"
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
