#include "filesmanager.h"
#include "progressmanager.h"
#include <QTimer>
#include <QFileInfo>
#include <QDebug>
#include <QThread>

FilesManager::FilesManager(QObject *parent)
    : QObject(parent)
{
    connect(ProgressManager::instance(), SIGNAL(canceled()), this, SLOT(cancel()));

    timer = new QTimer(this);
    timer->setInterval(500);
    connect(timer, SIGNAL(timeout()), this, SLOT(onProgressUpdate()));
    timer->start();

    connect(this, SIGNAL(stopped()), timer, SLOT(stop()), Qt::BlockingQueuedConnection);
}

void FilesManager::setRootDirectory(QString path)
{
    rootDir = QDir(path);
}

void FilesManager::remove(QString path)
{
    QFileInfo info(rootDir, path);
    if (!info.exists())
        return;
    if (info.isDir()) {
        removeDir(path);
    } else if (info.isFile()) {
        removeFile(path);
    }
}

void FilesManager::run()
{
    ProgressManager::invokeStart(ops.size());
    processedOps = 0;
    startFlag.testAndSetOrdered(0, 1);
    foreach (const auto& op, ops) {
        if (static_cast<int>(cancelFlag) == 1) {
            emit stopped();
            return;
        }
        switch (op.type) {
        case OpDesc::Remove: rootDir.remove(op.srcPath); ++processedOps; break;
        case OpDesc::RemoveDir: rootDir.rmdir(op.srcPath); ++processedOps; break;
        default: ++processedOps; break;
        }
    }
    ProgressManager::invokeSetProgress(ops.size());
    emit stopped();
}

void FilesManager::cancel()
{
    cancelFlag.testAndSetOrdered(0, 1);
}

void FilesManager::onProgressUpdate()
{
    if (static_cast<int>(startFlag) != 1 || static_cast<int>(cancelFlag) == 1)
        return;
    ProgressManager::instance()->setProgress(processedOps);
}

void FilesManager::removeDir(QString path)
{
    QDir dir = rootDir;
    if (!dir.cd(path))
        return;
    auto entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const auto& entry, entries) {
        auto name = entry.fileName();
        auto filePath = dir.absoluteFilePath(name);
        if (entry.isDir()) {
            removeDir(filePath);
        } else {
            removeFile(filePath);
        }
    }
    ops.append(OpDesc{ OpDesc::RemoveDir, path, QString() });
}

void FilesManager::removeFile(QString path)
{
    ops.append(OpDesc{ OpDesc::Remove, path, QString() });
}
