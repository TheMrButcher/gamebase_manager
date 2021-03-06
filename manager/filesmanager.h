#ifndef FILESMANAGER_H
#define FILESMANAGER_H

#include <QObject>
#include <QDir>
#include <QList>
#include <QAtomicInt>

class QProgressDialog;
class QTimer;

class FilesManager : public QObject
{
    Q_OBJECT
public:
    explicit FilesManager(QObject *parent = 0);

    void setRootDirectory(QString path);
    void reset();
    bool isCanceled() const;
    bool isOK() const;
    void remove(QString path);
    void unarchive(QString archivePath, QString dstPath);
    void rename(QString srcPath, QString dstPath);
    void move(QString srcPath, QString dstPath);
    void copyTree(QString srcPath, QString dstPath);
    void copyFiles(QString srcPath, QString dstPath);
    void moveFiles(QString srcPath, QString dstPath);
    void copy(QString srcPath, QString dstPath);
    void archive(QString srcRootPath, QStringList files, QString dstPath);
    bool run();

signals:
    void started();
    void stopped();

public slots:
    void cancel();

private slots:
    void onProgressUpdate();

private:
    struct OpDesc {
        enum OpType {
            Remove,
            RemoveDir,
            StartUnarchive,
            Unarchive,
            FinishUnarchive,
            Rename,
            Copy,
            Move,
            MakeDir,
            CreateArchive,
            AddToArchive,
            AddDirToArchive,
            CloseArchive
        };

        OpType type;
        QString srcPath;
        QString dstPath;
    };

    void removeDir(QString path);
    void removeFile(QString path);
    void processFiles(QDir srcDir, QDir dstDir, OpDesc::OpType opType);
    void archive(QDir srcRootDir, QString path);

    QDir rootDir;
    QList<OpDesc> ops;
    QAtomicInt processedOps;
    QAtomicInt startFlag;
    QAtomicInt cancelFlag;
    QTimer* timer;
    bool ok;
};

#endif // FILESMANAGER_H
