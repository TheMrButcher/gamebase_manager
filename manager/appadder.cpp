#include "appadder.h"
#include "progressmanager.h"
#include "filesmanager.h"
#include "settings.h"
#include <QDir>
#include <QFileInfo>

AppAdder::AppAdder(const App& app)
    : app(app)
{
    ProgressManager::instance()->show("Добавление приложения...", "Добавлено файлов в рабочую папку");
    manager = new FilesManager(this);
    manager->setRootDirectory(Settings::instance().workingDir().path);
}

void AppAdder::run()
{
    auto resultApp = app.afterAction(App::Add);
    formTask(resultApp);
    manager->run();
    emit finishedAdd(resultApp);
}

void AppAdder::formTask(App resultApp)
{
    auto workingDir = Settings::instance().workingDir();
    if (workingDir.check() != SourceStatus::OK)
        return;
    if (app.source.check() != SourceStatus::OK)
        return;
    QDir srcDir(app.source.path);
    if (!srcDir.exists(app.containerName))
        return;
    QDir dstDir(workingDir.path);
    if (resultApp.containerName.isEmpty() || dstDir.exists(resultApp.containerName))
        return;
    if (!dstDir.mkdir(resultApp.containerName))
        return;
    dstDir.cd(resultApp.containerName);

    if (app.state == App::Archived)
        manager->unarchive(srcDir.absoluteFilePath(app.containerName), dstDir.absolutePath());

    if (app.state == App::NotConfigured || app.state == App::Full) {
        if (!srcDir.cd(app.containerName))
            return;
        manager->copy(srcDir.absolutePath(), dstDir.absolutePath());
    }
}
