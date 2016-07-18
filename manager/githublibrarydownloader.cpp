#include "githublibrarydownloader.h"
#include "settings.h"
#include "files.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDir>
#include <QFile>

namespace {
const QString GITHUB_HOST_NAME = "github.com";
const QString API_PREFIX = "https://api.github.com/repos";
const QString RELEASES_SUFFIX = "/releases";
const QString VERSION_KEY = "tag_name";
const QString BINARY_RESULT_DIR_PREFIX = "Gamebase_Binary_";
const int MAX_RELEASES_NUM = 3;

QNetworkRequest makeRequest(QUrl url, const char* accept)
{
    QNetworkRequest request(url);
    request.setRawHeader(QByteArray("Accept"), QByteArray(accept));
    request.setHeader(QNetworkRequest::UserAgentHeader, QByteArray("GamebaseManager"));
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    return request;
}
}

GithubLibraryDownloader::GithubLibraryDownloader(const LibrarySource& source, QObject* parent)
    : LibrarySourceManager(parent)
    , source(source)
    , fatalError(false)
{
    networkManager = new QNetworkAccessManager(parent);
    int hostNameStart = source.path.indexOf(GITHUB_HOST_NAME);
    if (hostNameStart == -1) {
        fatalError = true;
        this->source.status = SourceStatus::Broken;
    } else {
        QString url = API_PREFIX + source.path.mid(hostNameStart + GITHUB_HOST_NAME.size());
        apiUrl = QUrl(url).url(QUrl::StripTrailingSlash);
    }

    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}

void GithubLibraryDownloader::update()
{
    if (fatalError || networkManager->networkAccessible() != QNetworkAccessManager::Accessible) {
        reportBrokenSource();
        return;
    }

    QUrl url(apiUrl + RELEASES_SUFFIX);
    qDebug() << "Request to " << url.toString();
    networkManager->get(makeRequest(url, "application/vnd.github.v3+json"));
}

void GithubLibraryDownloader::download(const Library& library)
{
    if (networkManager->networkAccessible() != QNetworkAccessManager::Accessible) {
        emit finishedDownload(Library::makeAbsent(source));
        return;
    }

    QJsonDocument json = QJsonDocument::fromJson(lastReleasesAnswer);
    auto releases = json.array();
    auto versionToDownload =  library.version.toString();
    foreach (auto jsonValue, releases) {
        auto releaseObj = jsonValue.toObject();
        auto versionStr = releaseObj[VERSION_KEY].toString();
        if (!versionStr.isEmpty() && versionStr[0] == 'v')
            versionStr = versionStr.mid(1);
        if (versionStr == versionToDownload) {
            startDownload(releaseObj, library);
            return;
        }
    }
    emit finishedDownload(Library::makeAbsent(source));
}

void GithubLibraryDownloader::startDownload(const QJsonObject& releaseObj, const Library& library)
{
    if (releaseObj["zipball_url"].toString().isEmpty()) {
        emit finishedDownload(Library::makeAbsent(source));
        return;
    }

    QJsonObject assetObj;
    auto assets = releaseObj["assets"].toArray();
    if (library.state == Library::BinaryArchive) {
        if (assets.empty()) {
            emit finishedDownload(Library::makeAbsent(source));
            return;
        }
        assetObj = assets[0].toObject();
        if (assetObj["name"].toString() != Files::BINARY_ARCHIVE_NAME
            || assetObj["browser_download_url"].toString().isEmpty()) {
            emit finishedDownload(Library::makeAbsent(source));
            return;
        }
    }

    auto downloadsDir = Settings::instance().downloadsDir();
    if (downloadsDir.check() != SourceStatus::OK) {
        emit finishedDownload(Library::makeAbsent(source));
        return;
    }
    auto versionSuffix = library.version.toString().replace('.', '_');
    auto dirName = BINARY_RESULT_DIR_PREFIX + versionSuffix;
    QDir dir(downloadsDir.path);
    if (dir.cd(dirName)) {
        dir.removeRecursively();
        dir = QDir(downloadsDir.path);
    }
    dir.mkdir(dirName);
    if (!dir.cd(dirName)) {
        emit finishedDownload(Library::makeAbsent(source));
        return;
    }
    Library resultLibrary{ downloadsDir, library.state, library.version, dirName };

    if (filesToDownload.contains(resultLibrary.archiveName))
        return;

    if (library.state == Library::BinaryArchive) {
        QUrl binUrl(assetObj["browser_download_url"].toString());
        auto binFilePath = dir.absoluteFilePath(Files::BINARY_ARCHIVE_NAME);
        filesToDownload[resultLibrary.archiveName].append(binFilePath);
        downloadRequests[binUrl.toString()] = DownloadDesc{
            DownloadDesc::Binaries, resultLibrary, binFilePath };
        networkManager->get(makeRequest(binUrl, "application/octet-stream"));
    }

    QUrl sourcesUrl(releaseObj["zipball_url"].toString());
    auto sourcesFilePath = dir.absoluteFilePath(Files::SOURCES_ARCHIVE_NAME);
    filesToDownload[resultLibrary.archiveName].append(sourcesFilePath);
    downloadRequests[sourcesUrl.toString()] = DownloadDesc{
        DownloadDesc::Sources, resultLibrary, sourcesFilePath };
    networkManager->get(makeRequest(sourcesUrl, "application/vnd.github.v3+json"));
}

void GithubLibraryDownloader::replyFinished(QNetworkReply* reply)
{
    auto request = reply->request();
    auto key = request.url().toString();
    qDebug() << "Got answer for request: " << key;

    auto body = reply->readAll();

    QFile debugFile("github_answer.txt");
    debugFile.open(QIODevice::WriteOnly);
    debugFile.write(body);

    if (reply->error() != QNetworkReply::NoError)
        qDebug() << "Network error: " << reply->errorString();

    bool isDownload = downloadRequests.contains(key);
    Library resultLibrary;
    if (isDownload) {
        resultLibrary = downloadRequests[key].library;
        processDownload(reply, body);
    } else {
        processReleases(reply, body);
    }
    downloadRequests.remove(key);
    if (isDownload) {
        bool areAllDownloadsFinished = true;
        foreach (const auto& desc, downloadRequests.values()) {
            if (desc.library.version == resultLibrary.version
                && desc.library.state == resultLibrary.state)
                areAllDownloadsFinished = false;
        }
        bool areAllFilesCreated = true;
        foreach (const auto& path, filesToDownload[resultLibrary.archiveName]) {
            QFile file(path);
            if (!file.exists())
                areAllFilesCreated = false;
        }
        if (areAllDownloadsFinished && areAllFilesCreated) {
            filesToDownload.remove(resultLibrary.archiveName);
            emit finishedDownload(resultLibrary);
        }
    }
    reply->deleteLater();
}

void GithubLibraryDownloader::reportBrokenSource()
{
    cachedLibraries.clear();
    source.status = SourceStatus::Broken;
    emit finishedUpdate(source, cachedLibraries);
}

void GithubLibraryDownloader::processReleases(QNetworkReply* reply, const QByteArray& body)
{
    qDebug() << "Got releases";

    if (reply->error() != QNetworkReply::NoError) {
        reportBrokenSource();
        return;
    }

    QJsonDocument json = QJsonDocument::fromJson(body);
    if (json.isArray()) {
        lastReleasesAnswer = body;
        source.status = SourceStatus::OK;
        cachedLibraries.clear();

        auto releases = json.array();
        int i = 0;
        foreach (auto jsonValue, releases) {
            if (i++ >= MAX_RELEASES_NUM)
                break;
            auto releaseObj = jsonValue.toObject();
            auto versionStr = releaseObj[VERSION_KEY].toString();
            if (!versionStr.isEmpty() && versionStr[0] == 'v')
                versionStr = versionStr.mid(1);
            Version version;
            version.set(versionStr);
            cachedLibraries.append(Library{ source, Library::BinaryArchive, version, "" });
            cachedLibraries.append(Library{ source, Library::SourceCode, version, "" });
        }

        emit finishedUpdate(source, cachedLibraries);
        return;
    }
    reportBrokenSource();
}

void GithubLibraryDownloader::processDownload(QNetworkReply* reply, const QByteArray& body)
{
    qDebug() << "Downloaded " << body.size() << " bytes";

    if (reply->error() != QNetworkReply::NoError) {
        emit finishedDownload(Library::makeAbsent(source));
        return;
    }

    auto downloadsDir = Settings::instance().downloadsDir();
    if (downloadsDir.check() != SourceStatus::OK) {
        emit finishedDownload(Library::makeAbsent(source));
        return;
    }
    const auto& desc = downloadRequests[reply->request().url().toString()];
    QFile resultFile(desc.resultFileName);
    if (!resultFile.open(QIODevice::WriteOnly)) {
        emit finishedDownload(Library::makeAbsent(source));
        return;
    }
    resultFile.write(body);
}
