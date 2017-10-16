#include "githublibrarydownloader.h"
#include "settings.h"
#include "files.h"
#include <QWidget>

namespace {
const int MAX_RELEASES_NUM = 3;

GithubDownloader::Asset findAsset(const QList<GithubDownloader::Asset>& assets, QString name)
{
    for (const auto& asset : assets)
        if (asset.name == name)
            return asset;
    return GithubDownloader::Asset();
}
}

GithubLibraryDownloader::GithubLibraryDownloader(const LibrarySource& source, QWidget* parent)
    : LibrarySourceManager(parent)
    , source(source)
{
    downloader = new GithubDownloader(source.path, parent);
    connect(downloader, SIGNAL(cancelled()), this, SLOT(onCancel()));
    connect(downloader, SIGNAL(broken()), this, SLOT(reportBrokenSource()));
    connect(downloader, SIGNAL(finishedUpdate(QStringList)),
            this, SLOT(onVersionsUpdated(QStringList)));
    connect(downloader, SIGNAL(finishedDownload()), this, SLOT(onDownloadFinished()));
    connect(downloader, SIGNAL(errorDuringDownload()), this, SLOT(onDownloadFinished()));
}

void GithubLibraryDownloader::update()
{
    if (downloader->check() == SourceStatus::Broken)
        return reportBrokenSource();
    downloader->updateInfo();
}

void GithubLibraryDownloader::download(const Library& library)
{
    Library resultLibrary = library.afterAction(Library::Download);
    if (resultLibrary.state != Library::BinaryArchive) {
        emit finishedDownload(resultLibrary);
        return;
    }

    if (!downloader->hasConnection()) {
        emit finishedDownload(resultLibrary);
        return;
    }

    if (downloader->isBusy()) {
        emit finishedDownload(resultLibrary);
        return;
    }

    auto versionToDownload = library.version.toString();
    auto releases = downloader->releases(QStringList() << versionToDownload);
    if (releases.size() != 1) {
        emit finishedDownload(resultLibrary);
        return;
    }

    auto release = releases.front();
    bool isFull = true;
    //isFull = isFull && !release.sourcesUrl.isEmpty();
    if (resultLibrary.state == Library::BinaryArchive) {
        QStringList assetNames;
        for (const auto& asset : release.assets)
            assetNames.append(asset.name);
        isFull = isFull
                && assetNames.contains(Files::GAMEBASE_ARCHIVE_NAME)
                && assetNames.contains(Files::CONTRIB_ARCHIVE_NAME);
    }
    if (!isFull) {
        emit finishedDownload(resultLibrary);
        return;
    }

    QDir dir(resultLibrary.source.path);
    if (dir.cd(resultLibrary.archiveName)) {
        dir.removeRecursively();
        dir = QDir(resultLibrary.source.path);
    }
    dir.mkdir(resultLibrary.archiveName);
    if (!dir.cd(resultLibrary.archiveName)) {
        emit finishedDownload(resultLibrary);
        return;
    }

    libraryToDownload = resultLibrary;
    QList<GithubDownloader::Request> requests;
    if (resultLibrary.state == Library::BinaryArchive) {
        auto gamebaseFilePath = dir.absoluteFilePath(Files::GAMEBASE_ARCHIVE_NAME);
        requests.append(GithubDownloader::Request{
            findAsset(release.assets, Files::GAMEBASE_ARCHIVE_NAME).url,
            gamebaseFilePath, "application/octet-stream" });

        auto contribFilePath = dir.absoluteFilePath(Files::CONTRIB_ARCHIVE_NAME);
        requests.append(GithubDownloader::Request{
            findAsset(release.assets, Files::CONTRIB_ARCHIVE_NAME).url,
            contribFilePath, "application/octet-stream" });
    }
    /*auto sourcesFilePath = dir.absoluteFilePath(Files::SOURCES_ARCHIVE_NAME);
    requests.append(GithubDownloader::Request{
        release.sourcesUrl, sourcesFilePath, "application/vnd.github.v3+json" });*/
    downloader->download(requests, TaskMode::ShowDialog);
}

void GithubLibraryDownloader::onVersionsUpdated(QStringList versions)
{
    if (versions.empty())
        return reportBrokenSource();

    QList<Version> sortedVersions;
    sortedVersions.reserve(versions.size());
    foreach (auto version, versions)
        sortedVersions.append(Version::fromString(version));
    qSort(sortedVersions);

    source.status = SourceStatus::OK;
    cachedLibraries.clear();

    std::reverse(sortedVersions.begin(), sortedVersions.end());
    auto itLast = std::find_if(sortedVersions.begin(), sortedVersions.end(),
                               [](const Version& v) { return v < Version::CURRENT_MAJOR; });
    sortedVersions.erase(itLast, sortedVersions.end());
    if (sortedVersions.size() > MAX_RELEASES_NUM)
        sortedVersions.erase(sortedVersions.begin() + MAX_RELEASES_NUM, sortedVersions.end());

    for (const auto& version : sortedVersions)
        cachedLibraries.append(Library{ source, Library::BinaryArchive, version, "" });
    emit finishedUpdate(source, cachedLibraries);
}

void GithubLibraryDownloader::onCancel()
{
    emit finishedDownload(libraryToDownload);
}

void GithubLibraryDownloader::reportBrokenSource()
{
    cachedLibraries.clear();
    source.status = SourceStatus::Broken;
    emit finishedUpdate(source, cachedLibraries);
}

void GithubLibraryDownloader::onDownloadFinished()
{
    emit finishedDownload(libraryToDownload);
}
