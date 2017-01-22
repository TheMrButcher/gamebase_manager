#ifndef GITHUBUPDATEDOWNLOADER_H
#define GITHUBUPDATEDOWNLOADER_H

#include "githubdownloader.h"
#include "updatedescriptor.h"

class GithubUpdateDownloader : public QObject
{
    Q_OBJECT

public:
    GithubUpdateDownloader(QString sourceUrl, QWidget* parent);

    SourceStatus check() { return downloader->check(); }
    void updateInfo();
    void download(const QList<UpdateDescriptor>& updates, TaskMode mode);

    static GithubUpdateDownloader* instance();

signals:
    void broken();
    void cancelled();
    void finishedUpdate(const QList<UpdateDescriptor>&);
    void finishedDownload();
    void errorDuringDownload();

private slots:
    void onVersionsUpdated(QStringList versions);

private:
    GithubDownloader* downloader;
};

#endif // GITHUBUPDATEDOWNLOADER_H
