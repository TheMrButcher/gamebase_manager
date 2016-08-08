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
    void remove(QString path);
    void run();

signals:
    void stopped();

private slots:
    void cancel();
    void onProgressUpdate();

private:
    void removeDir(QString path);
    void removeFile(QString path);

    struct OpDesc {
        enum OpType {
            Remove,
            RemoveDir
        };

        OpType type;
        QString srcPath;
        QString dstPath;
    };

    QDir rootDir;
    QList<OpDesc> ops;
    int processedOps;
    QAtomicInt startFlag;
    QAtomicInt cancelFlag;
    QTimer* timer;
};

#endif // FILESMANAGER_H
