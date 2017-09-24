#include "appdeployer.h"
#include "progressmanager.h"
#include "filesmanager.h"
#include "settings.h"
#include "files.h"
#include "compiler.h"
#include <QDir>
#include <QFileInfo>

AppDeployer::AppDeployer(const App& app, QString dstPath)
    : app(app)
    , dstPath(dstPath)
{
    manager = new FilesManager(this);
    manager->setRootDirectory(dstPath);
    compiler = new Compiler(this);
}

void AppDeployer::run()
{
    bool success = tryDeploy();
    if (!success) {
        ProgressManager::invokeShow("Удаление папки...", "Удалено файлов");
        manager->reset();
        manager->remove(dstPath);
        manager->run();
    }
    emit finishedDeploy(app, dstPath, success);
}

bool AppDeployer::tryDeploy()
{
    if (!formCopyTask())
        return false;
    if (!manager->run())
        return false;

    QDir srcDir(app.source.path);
    srcDir.cd(app.containerName);
    if (compiler->compile(srcDir, app.name, Compiler::BuildType::Release)) {
        if (srcDir.cd("Release")) {
            QString exeName = app.name + ".exe";
            QDir dstDir(dstPath);
            QFile::rename(srcDir.absoluteFilePath(exeName),
                          dstDir.absoluteFilePath(exeName));
            return true;
        }
    }
    return false;
}

bool AppDeployer::formCopyTask()
{
    ProgressManager::invokeShow("Копирование файлов...", "Скопировано файлов");
    QDir srcDir(app.source.path);
    if (!srcDir.cd(app.containerName))
        return false;
    QDir dstDir;
    if (!dstDir.exists(dstPath))
        dstDir.mkpath(dstPath);
    if (!dstDir.cd(dstPath))
        return false;

    auto config = app.config();
    auto newConfig = AppConfig::makeDeployedAppConfig(dstDir, config);
    if (!newConfig.write(dstDir, dstDir.absoluteFilePath(Files::APP_CONFIG_NAME)))
        return false;
    if (!app.version.write(dstDir.absoluteFilePath(Files::VERSION_FILE_NAME)))
        return false;
    if (!app.updateMainCpp(Files::APP_CONFIG_NAME))
        return false;

    dstDir.mkpath(newConfig.imagesPath);
    manager->copyFiles(config.imagesPath, newConfig.imagesPath);

    dstDir.mkpath(newConfig.designPath);
    manager->copyFiles(config.designPath, newConfig.designPath);

    dstDir.mkpath(newConfig.shadersPath);
    manager->copyFiles(config.shadersPath, newConfig.shadersPath);

    dstDir.mkpath(newConfig.fontsPath);
    manager->copyFiles(config.fontsPath, newConfig.fontsPath);

    QDir libsDir(Settings::instance().workingDir().path);
    if (!libsDir.cd(Files::DEPLOYED_ROOT_DIR_NAME)
        || !libsDir.cd(Files::CONTRIB_DIR_NAME)
        || !libsDir.cd(Files::BIN_DIR_NAME)
        || !libsDir.cd(Files::RELEASE_DIR_NAME))
        return false;
    auto libs = libsDir.entryList(QStringList("*.dll"), QDir::Files);
    foreach (auto file, libs)
        manager->copy(libsDir.absoluteFilePath(file), file);
    return true;
}
