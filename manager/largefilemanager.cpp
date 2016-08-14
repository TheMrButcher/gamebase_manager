#include "largefilemanager.h"
#include "progressmanager.h"
#include <QTimer>

namespace {
const int BUFFER_SIZE = 2048;
}

LargeFileManager::LargeFileManager(QObject *parent)
    : QObject(parent)
    , ok(true)
{
    connect(ProgressManager::instance(), SIGNAL(canceled()), this, SLOT(cancel()));

    timer = new QTimer(this);
    timer->setInterval(500);
    connect(timer, SIGNAL(timeout()), this, SLOT(onProgressUpdate()));

    connect(this, SIGNAL(started()), timer, SLOT(start()));
    connect(this, SIGNAL(stopped()), timer, SLOT(stop()), Qt::BlockingQueuedConnection);
}

void LargeFileManager::setRootDirectory(QString path)
{
    rootDir = QDir(path);
}

void LargeFileManager::reset()
{
    totalSize = 0;
    processedSize = 0;
    startFlag = 0;
    cancelFlag = 0;
    ops.clear();
    ok = true;
}

bool LargeFileManager::isCanceled() const
{
    return static_cast<int>(cancelFlag) == 1;
}

bool LargeFileManager::isOK() const
{
    return ok && !isCanceled();
}

void LargeFileManager::copy(QString srcPath, QString dstPath)
{
    QFile file(rootDir.absoluteFilePath(srcPath));
    if (!file.exists()) {
        ok = false;
        return;
    }
    if (!file.open(QIODevice::ReadOnly)) {
        ok = false;
        return;
    }
    totalSize += static_cast<int>(file.size());
    ops.append(OpDesc{ rootDir.absoluteFilePath(srcPath),
                       rootDir.absoluteFilePath(dstPath) });
}

bool LargeFileManager::run()
{
    ProgressManager::invokeStart(totalSize / 1024);
    startFlag.testAndSetOrdered(0, 1);
    emit started();

    foreach (const auto& op, ops) {
        if (isCanceled()) {
            emit stopped();
            return false;
        }

        QFile srcFile(op.srcPath);
        if (!srcFile.open(QIODevice::ReadOnly)) {
            ok = false;
            continue;
        }
        QFile dstFile(op.dstPath);
        if (!dstFile.open(QIODevice::WriteOnly)) {
            ok = false;
            continue;
        }
        char buffer[BUFFER_SIZE];
        while (!srcFile.atEnd()) {
            int readSize = static_cast<int>(srcFile.read(buffer, BUFFER_SIZE));
            if (readSize == 0)
                break;
            if (readSize < 0) {
                ok = false;
                break;
            }
            if (dstFile.write(buffer, readSize) <= 0) {
                ok = false;
                break;
            }
            processedSize += readSize;
        }
    }
    ProgressManager::invokeSetProgress(totalSize / 1024);
    emit stopped();
    return isOK();
}

void LargeFileManager::cancel()
{
    cancelFlag.testAndSetOrdered(0, 1);
}

void LargeFileManager::onProgressUpdate()
{
    if (static_cast<int>(startFlag) != 1 || isCanceled())
        return;
    ProgressManager::instance()->setProgress(static_cast<int>(processedSize) / 1024);
}
