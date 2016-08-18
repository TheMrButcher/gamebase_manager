#ifndef LARGEFILEMANAGER_H
#define LARGEFILEMANAGER_H

#include <QObject>
#include <QDir>
#include <QList>
#include <QAtomicInt>

class QTimer;
class QIODevice;

class LargeFileManager : public QObject
{
    Q_OBJECT
public:
    explicit LargeFileManager(QObject *parent = 0);

    void setRootDirectory(QString path);
    void reset();
    bool isCanceled() const;
    bool isOK() const;
    void copy(QString srcPath, QString dstPath);
    bool run();

    static bool copy(QIODevice* src, QIODevice* dst);

signals:
    void started();
    void stopped();

private slots:
    void cancel();
    void onProgressUpdate();

private:
    struct OpDesc {
        QString srcPath;
        QString dstPath;
    };

    QDir rootDir;
    QList<OpDesc> ops;
    int totalSize;
    QAtomicInt processedSize;
    QAtomicInt startFlag;
    QAtomicInt cancelFlag;
    QTimer* timer;
    bool ok;
};

#endif // LARGEFILEMANAGER_H
