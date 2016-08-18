#include "filesmanager.h"
#include "progressmanager.h"
#include <QTimer>
#include <QFileInfo>
#include <QDebug>
#include <QThread>
#include <JlCompress.h>

namespace {
const int UNARCHIVE_BATCH_SIZE = 64;
const int BUFFER_SIZE = 2048;

bool makeDirInZip(QuaZip* zip, QString srcPath, QString dstPath)
{
    QuaZipFile dirZipFile(zip);
    QuaZipNewInfo dirZipInfo(dstPath + "/", srcPath);
    if (!dirZipFile.open(QIODevice::WriteOnly, dirZipInfo, 0, 0, 0))
        return false;
    dirZipFile.close();
    return true;
}

bool compressFile(QuaZip* zip, QString srcPath, QString dstPath)
{
    QFile srcFile(srcPath);
    if (!srcFile.open(QIODevice::ReadOnly))
        return false;
    QuaZipFile dstFile(zip);
    QuaZipNewInfo dstFileInfo(dstPath, srcPath);
    if (!dstFile.open(QIODevice::WriteOnly, dstFileInfo))
        return false;

    char buffer[BUFFER_SIZE];
    while (!srcFile.atEnd()) {
        int readSize = static_cast<int>(srcFile.read(buffer, BUFFER_SIZE));
        if (readSize == 0)
            break;
        if (readSize < 0)
            return false;
        if (dstFile.write(buffer, readSize) != readSize)
            return false;
    }

    if (dstFile.getZipError() != UNZ_OK)
        return false;

    dstFile.close();
    if (dstFile.getZipError() != UNZ_OK)
        return false;
    return true;
}
}

FilesManager::FilesManager(QObject *parent)
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

void FilesManager::setRootDirectory(QString path)
{
    rootDir = QDir(path);
}

void FilesManager::reset()
{
    processedOps = 0;
    startFlag = 0;
    cancelFlag = 0;
    ops.clear();
    ok = true;
}

bool FilesManager::isCanceled() const
{
    return static_cast<int>(cancelFlag) == 1;
}

bool FilesManager::isOK() const
{
    return ok && !isCanceled();
}

void FilesManager::remove(QString path)
{
    QFileInfo info(rootDir, path);
    if (!info.exists())
        return;
    if (info.isDir()) {
        removeDir(path);
        return;
    }
    if (info.isFile()) {
        removeFile(path);
        return;
    }
    ok = false;
}

void FilesManager::unarchive(QString archivePath, QString dstPath)
{
    QString absArchivePath = rootDir.absoluteFilePath(archivePath);
    QString absDstPath = rootDir.absoluteFilePath(dstPath);
    QStringList files = JlCompress::getFileList(absArchivePath);
    if (files.empty()) {
        ok = false;
        return;
    }
    ops.append(OpDesc{ OpDesc::StartUnarchive, absArchivePath, absDstPath });
    foreach (QString file, files)
        ops.append(OpDesc{ OpDesc::Unarchive, file, QString() });
    ops.append(OpDesc{ OpDesc::FinishUnarchive, QString(), QString() });
}

void FilesManager::rename(QString srcPath, QString dstPath)
{
    ops.append(OpDesc{ OpDesc::Rename, srcPath, dstPath });
}

void FilesManager::copyFiles(QString srcPath, QString dstPath)
{
    if (!QFileInfo(rootDir, srcPath).isDir()) {
        ok = false;
        return;
    }
    QDir srcDir(rootDir.absoluteFilePath(srcPath));
    QDir dstDir(rootDir.absoluteFilePath(dstPath));
    copyFiles(srcDir, dstDir);
}

void FilesManager::copy(QString srcPath, QString dstPath)
{
    ops.append(OpDesc{ OpDesc::Copy,
                       rootDir.absoluteFilePath(srcPath),
                       rootDir.absoluteFilePath(dstPath) });
}

void FilesManager::archive(QString srcRootPath, QStringList files, QString dstPath)
{
    QString absSrcRootPath = rootDir.absoluteFilePath(srcRootPath);
    QString absDstPath = rootDir.absoluteFilePath(dstPath);
    QDir srcRootDir(absSrcRootPath);
    if (!rootDir.exists()) {
        ok = false;
        return;
    }

    ops.append(OpDesc{ OpDesc::CreateArchive, QString(), absDstPath });
    foreach (QString file, files)
        archive(srcRootDir, file);
    ops.append(OpDesc{ OpDesc::CloseArchive, QString(), absDstPath });
}

bool FilesManager::run()
{
    int opsNum = ops.isEmpty() ? 1 : ops.size();
    ProgressManager::invokeStart(opsNum);
    startFlag.testAndSetOrdered(0, 1);
    emit started();

    if (isCanceled()) {
        emit stopped();
        return false;
    }

    if (ops.isEmpty()) {
        ProgressManager::invokeSetProgress(opsNum);
        emit stopped();
        return isOK();
    }

    QString curArchive;
    QString curDst;
    QStringList filesBatch;
    QuaZip* dstZip = nullptr;
    foreach (const auto& op, ops) {
        if (!ok)
            break;

        if (isCanceled()) {
            emit stopped();
            if (dstZip != nullptr)
                delete dstZip;
            return false;
        }

        switch (op.type) {
        case OpDesc::Remove:
            ok = ok && rootDir.remove(op.srcPath);
            ++processedOps;
            break;

        case OpDesc::RemoveDir:
            ok = ok && rootDir.rmdir(op.srcPath);
            ++processedOps;
            break;

        case OpDesc::StartUnarchive:
            curArchive = op.srcPath;
            curDst = op.dstPath;
            ++processedOps;
            break;

        case OpDesc::Unarchive:
            filesBatch.append(op.srcPath);
            if (filesBatch.size() >= UNARCHIVE_BATCH_SIZE) {
                ok = ok && JlCompress::extractFiles(curArchive, filesBatch, curDst).size() != 0;
                processedOps += filesBatch.size();
                filesBatch.clear();
            }
            break;

        case OpDesc::FinishUnarchive:
            ++processedOps;
            if (!filesBatch.empty()) {
                ok = ok && JlCompress::extractFiles(curArchive, filesBatch, curDst).size() != 0;
                processedOps += filesBatch.size();
                filesBatch.clear();
            }
            break;

        case OpDesc::Rename:
            ok = ok && rootDir.rename(op.srcPath, op.dstPath);
            ++processedOps;
            break;

        case OpDesc::MakeDir:
            ok = ok && rootDir.mkdir(op.dstPath);
            ++processedOps;
            break;

        case OpDesc::Copy:
            ok = ok && QFile::copy(op.srcPath, op.dstPath);
            ++processedOps;
            break;

        case OpDesc::CreateArchive:
            dstZip = new QuaZip(op.dstPath);
            if (!dstZip->open(QuaZip::mdCreate)) {
                ok = false;
                delete dstZip;
                dstZip = nullptr;
            }
            ++processedOps;
            break;

        case OpDesc::AddToArchive:
            if (dstZip)
                ok = ok && compressFile(dstZip, op.srcPath, op.dstPath);
            ++processedOps;
            break;

        case OpDesc::AddDirToArchive:
            if (dstZip)
                ok = ok && makeDirInZip(dstZip, op.srcPath, op.dstPath);
            ++processedOps;
            break;

        case OpDesc::CloseArchive:
            if (dstZip) {
                dstZip->close();
                delete dstZip;
                dstZip = nullptr;
            }
            ++processedOps;
            break;

        default: break;
        }
    }
    ProgressManager::invokeSetProgress(opsNum);
    emit stopped();
    if (dstZip != nullptr)
        delete dstZip;
    return isOK();
}

void FilesManager::cancel()
{
    cancelFlag.testAndSetOrdered(0, 1);
}

void FilesManager::onProgressUpdate()
{
    if (static_cast<int>(startFlag) != 1 || isCanceled())
        return;
    ProgressManager::instance()->setProgress(static_cast<int>(processedOps));
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

void FilesManager::copyFiles(QDir srcDir, QDir dstDir)
{
    auto entries = srcDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    foreach (const auto& entry, entries) {
        auto name = entry.fileName();
        auto srcFilePath = srcDir.absoluteFilePath(name);
        auto dstFilePath = dstDir.absoluteFilePath(name);
        if (entry.isDir()) {
            ops.append(OpDesc{ OpDesc::MakeDir, QString(), dstFilePath });
            copyFiles(QDir(srcFilePath), QDir(dstFilePath));
        } else {
            ops.append(OpDesc{ OpDesc::Copy, srcFilePath, dstFilePath });
        }
    }
}

void FilesManager::archive(QDir srcRootDir, QString path)
{
    QFileInfo fileInfo(srcRootDir, path);
    auto srcFilePath = srcRootDir.absoluteFilePath(path);
    auto dstFilePath = srcRootDir.relativeFilePath(srcFilePath);
    if (fileInfo.isDir()) {
        ops.append(OpDesc{ OpDesc::AddDirToArchive, srcFilePath, dstFilePath });
        QDir dir(srcRootDir.absoluteFilePath(path));
        auto entries = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
        foreach (const auto& entry, entries) {
            auto name = entry.fileName();
            auto filePath = dir.absoluteFilePath(name);
            archive(srcRootDir, filePath);
        }
    } else if (fileInfo.isFile()) {
        ops.append(OpDesc{ OpDesc::AddToArchive, srcFilePath, dstFilePath });
    }
}
