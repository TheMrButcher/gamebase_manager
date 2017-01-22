#include "githubupdatedownloader.h"
#include "settings.h"
#include <QWidget>
#include <QDir>

GithubUpdateDownloader::GithubUpdateDownloader(QString sourceUrl, QWidget* parent)
    : QObject(parent)
{
    downloader = new GithubDownloader(sourceUrl, parent);
    connect(downloader, SIGNAL(cancelled()), this, SIGNAL(cancelled()));
    connect(downloader, SIGNAL(broken()), this, SIGNAL(broken()));
    connect(downloader, SIGNAL(finishedUpdate(QStringList)),
            this, SLOT(onVersionsUpdated(QStringList)));
    connect(downloader, SIGNAL(finishedDownload()), this, SIGNAL(finishedDownload()));
    connect(downloader, SIGNAL(errorDuringDownload()), this, SIGNAL(errorDuringDownload()));
}

void GithubUpdateDownloader::updateInfo()
{
    if (downloader->check() == SourceStatus::Broken) {
        emit broken();
        return;
    }
    downloader->updateInfo();
}

void GithubUpdateDownloader::download(
        const QList<UpdateDescriptor>& updates,
        TaskMode mode)
{
    if (!downloader->hasConnection() || downloader->isBusy()) {
        emit errorDuringDownload();
        return;
    }

    QDir dir(Settings::instance().downloadsDir().path);
    if (!dir.exists()) {
        emit errorDuringDownload();
        return;
    }

    QList<GithubDownloader::Request> requests;
    requests.reserve(updates.size());
    foreach (const auto& update, updates) {
        QString path = dir.absoluteFilePath(update.fileName);
        requests.append(GithubDownloader::Request{
            update.url, path, "application/octet-stream" });
    }
    downloader->download(requests, mode);
}

void GithubUpdateDownloader::onVersionsUpdated(QStringList versions)
{
    QList<UpdateDescriptor> updates;
    updates.reserve(versions.size());
    auto releases = downloader->releases(versions);
    foreach (const auto& release, releases) {
        foreach (const auto& asset, release.assets) {
            if (UpdateDescriptor::isUpdate(asset.name))
                updates.append(UpdateDescriptor(asset.url, asset.name));
        }
    }
    emit finishedUpdate(updates);
}
