#include "appcompressor.h"
#include "progressmanager.h"
#include "filesmanager.h"
#include <QDir>
#include <QFileInfo>

#include <QDebug>

AppCompressor::AppCompressor(const App& srcApp, const App& dstApp)
    : srcApp(srcApp)
    , dstApp(dstApp)
{
    ProgressManager::instance()->show("Сжатие приложения...", "Сжато файлов");
    manager = new FilesManager(this);
    manager->setRootDirectory(srcApp.source.path);
}

void AppCompressor::run()
{
    if (dstApp.source.check() != SourceStatus::OK)
        QDir().mkpath(dstApp.source.path);

    QDir srcDir(srcApp.source.path);
    QDir appDir = srcDir;
    appDir.cd(srcApp.containerName);
    auto entries = appDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList filesToCompress;
    filesToCompress.reserve(entries.size());
    foreach (const auto& entry, entries) {
        if (entry.isDir()) {
            QString name = entry.baseName();
            if (name == "Release" || name == "Debug" || name == "ipch")
                continue;
        } else if (entry.isFile()) {
            QString extension = entry.suffix();
            if (extension == "suo" || extension == "sdf")
                continue;
        } else {
            continue;
        }

        filesToCompress.append(entry.fileName());
    }

    QDir dstDir(dstApp.source.path);
    if (!filesToCompress.isEmpty() && dstDir.exists()) {
        manager->archive(srcDir.absoluteFilePath(srcApp.containerName),
                         filesToCompress,
                         dstDir.absoluteFilePath(dstApp.containerName));
    }
    manager->run();
    emit finishedCompress(dstApp);
}
