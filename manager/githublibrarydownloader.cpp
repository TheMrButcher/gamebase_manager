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
#include <QProgressDialog>
#include <QProgressBar>

namespace {
const QString GITHUB_HOST_NAME = "github.com";
const QString API_PREFIX = "https://api.github.com/repos";
const QString RELEASES_SUFFIX = "/releases";
const QString VERSION_KEY = "tag_name";
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

GithubLibraryDownloader::GithubLibraryDownloader(const LibrarySource& source, QWidget* parent)
    : LibrarySourceManager(parent)
    , source(source)
    , fatalError(false)
    , parent(parent)
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
    progressDialog = nullptr;
}

void GithubLibraryDownloader::update()
{
    if (fatalError || networkManager->networkAccessible() != QNetworkAccessManager::Accessible) {
        reportBrokenSource();
        return;
    }

    QUrl url(apiUrl + RELEASES_SUFFIX);
    networkManager->get(makeRequest(url, "application/vnd.github.v3+json"));
}

void GithubLibraryDownloader::download(const Library& library)
{
    Library resultLibrary = library.afterAction(Library::Download);
    if (networkManager->networkAccessible() != QNetworkAccessManager::Accessible) {
        emit finishedDownload(resultLibrary);
        return;
    }

    if (!downloadRequests.empty()) {
        emit finishedDownload(resultLibrary);
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
            startDownload(releaseObj, library, resultLibrary);
            return;
        }
    }
    emit finishedDownload(resultLibrary);
}

void GithubLibraryDownloader::startDownload(
        const QJsonObject& releaseObj,
        const Library& library,
        Library resultLibrary)
{
    if (releaseObj["zipball_url"].toString().isEmpty()) {
        emit finishedDownload(resultLibrary);
        return;
    }

    QJsonObject assetObj;
    auto assets = releaseObj["assets"].toArray();
    if (library.state == Library::BinaryArchive) {
        if (assets.empty()) {
            emit finishedDownload(resultLibrary);
            return;
        }
        assetObj = assets[0].toObject();
        if (assetObj["name"].toString() != Files::BINARY_ARCHIVE_NAME
            || assetObj["browser_download_url"].toString().isEmpty()) {
            emit finishedDownload(resultLibrary);
            return;
        }
    }

    if (resultLibrary.source.check() != SourceStatus::OK) {
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

    if (!progressDialog) {
        progressDialog = new QProgressDialog(parent);

        auto bar = new QProgressBar(progressDialog);
        bar->setTextVisible(false);
        progressDialog->setBar(bar);

        progressDialog->setWindowTitle("Загрузка");
        progressDialog->setMinimumDuration(0);
        progressDialog->setRange(0, 0);
        progressDialog->setModal(true);
        connect(progressDialog, SIGNAL(canceled()), this, SLOT(onCancel()));
    }
    progressDialog->setLabelText("");
    progressDialog->show();

    if (library.state == Library::BinaryArchive) {
        QUrl binUrl(assetObj["browser_download_url"].toString());
        auto binFilePath = dir.absoluteFilePath(Files::BINARY_ARCHIVE_NAME);
        auto reply = networkManager->get(makeRequest(binUrl, "application/octet-stream"));
        downloadRequests[binUrl.toString()] = DownloadDesc(binFilePath, reply);
        connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
                this, SLOT(onDownload(qint64,qint64)));
    }

    QUrl sourcesUrl(releaseObj["zipball_url"].toString());
    auto sourcesFilePath = dir.absoluteFilePath(Files::SOURCES_ARCHIVE_NAME);
    auto reply = networkManager->get(makeRequest(sourcesUrl, "application/vnd.github.v3+json"));
    downloadRequests[sourcesUrl.toString()] = DownloadDesc(sourcesFilePath, reply);
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(onDownload(qint64,qint64)));
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
    if (isDownload) {
        processDownload(reply, body);
    } else {
        processReleases(reply, body);
    }
    if (isDownload) {
        downloadRequests[key].isRunning = false;
        bool areAllDownloadsFinished = true;
        foreach (const auto& desc, downloadRequests.values()) {
            if (desc.isRunning)
                areAllDownloadsFinished = false;
        }
        if (areAllDownloadsFinished) {
            downloadRequests.clear();
            progressDialog->hide();
            emit finishedDownload(libraryToDownload);
        }
    }
    reply->deleteLater();
}

void GithubLibraryDownloader::onDownload(qint64 bytesReceived, qint64 /*bytesTotal*/)
{
    auto sender = QObject::sender();
    int received = 0;
    for (auto it = downloadRequests.begin(); it != downloadRequests.end(); ++it) {
        DownloadDesc& desc = it.value();
        if (desc.reply == sender)
            desc.bytesReceived = bytesReceived;
        received += static_cast<int>(desc.bytesReceived);
    }

    progressDialog->setLabelText(QString("Загружено %1 МБ").arg(received / (1024.0 * 1024.0)));
}

void GithubLibraryDownloader::onCancel()
{
    foreach (const auto& desc, downloadRequests.values()) {
        if (desc.isRunning)
            desc.reply->abort();
    }
    progressDialog->hide();
    emit finishedDownload(libraryToDownload);
}

void GithubLibraryDownloader::reportBrokenSource()
{
    cachedLibraries.clear();
    source.status = SourceStatus::Broken;
    emit finishedUpdate(source, cachedLibraries);
}

void GithubLibraryDownloader::processReleases(QNetworkReply* reply, const QByteArray& body)
{
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
    if (reply->error() != QNetworkReply::NoError) {
        emit finishedDownload(libraryToDownload);
        return;
    }

    auto downloadsDir = Settings::instance().downloadsDir();
    if (downloadsDir.check() != SourceStatus::OK) {
        emit finishedDownload(libraryToDownload);
        return;
    }
    const auto& desc = downloadRequests[reply->request().url().toString()];
    QFile resultFile(desc.resultFileName);
    if (!resultFile.open(QIODevice::WriteOnly)) {
        emit finishedDownload(libraryToDownload);
        return;
    }
    resultFile.write(body);
}
