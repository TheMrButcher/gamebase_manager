#include "githubdownloader.h"
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

QNetworkRequest makeRequest(QUrl url, QByteArray accept)
{
    QNetworkRequest request(url);
    request.setRawHeader(QByteArray("Accept"), accept);
    request.setHeader(QNetworkRequest::UserAgentHeader, QByteArray("GamebaseManager"));
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    return request;
}

QString extractVersion(const QJsonObject& releaseObj)
{
    auto result = releaseObj[VERSION_KEY].toString();
    if (!result.isEmpty() && result[0] == 'v')
        result = result.mid(1);
    return result;
}
}

GithubDownloader::GithubDownloader(QString sourceUrl, QWidget* parent)
    : QObject(parent)
    , sourceUrl(sourceUrl)
    , sourceStatus(SourceStatus::Unknown)
    , fatalError(false)
    , parent(parent)
{
    networkManager = new QNetworkAccessManager(parent);
    int hostNameStart = sourceUrl.indexOf(GITHUB_HOST_NAME);
    if (hostNameStart == -1) {
        fatalError = true;
        sourceStatus = SourceStatus::Broken;
    } else {
        QString url = API_PREFIX + sourceUrl.mid(hostNameStart + GITHUB_HOST_NAME.size());
        apiUrl = QUrl(url).url(QUrl::StripTrailingSlash);
    }

    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    progressDialog = nullptr;
}

bool GithubDownloader::hasConnection() const
{
    return networkManager->networkAccessible() == QNetworkAccessManager::Accessible;
}

SourceStatus GithubDownloader::check()
{
    if (fatalError || !hasConnection())
        reportBrokenSource();
    return sourceStatus;
}

void GithubDownloader::updateInfo()
{
    QUrl url(apiUrl + RELEASES_SUFFIX);
    networkManager->get(makeRequest(url, "application/vnd.github.v3+json"));
}

QList<GithubDownloader::Release> GithubDownloader::releases(QStringList versions)
{
    QJsonDocument json = QJsonDocument::fromJson(lastReleasesAnswer);
    auto releases = json.array();

    QList<Release> result;
    result.reserve(releases.size());
    foreach (auto jsonReleaseValue, releases) {
        auto releaseObj = jsonReleaseValue.toObject();
        auto versionStr = extractVersion(releaseObj);
        if (!versions.contains(versionStr))
            continue;

        result.append(Release());
        auto& release = result.back();
        release.sourcesUrl = releaseObj["zipball_url"].toString();
        auto assets = releaseObj["assets"].toArray();
        release.assets.reserve(assets.size());
        foreach (auto jsonAssetValue, assets) {
            auto assetObj = jsonAssetValue.toObject();
            release.assets.append(Asset{
                assetObj["name"].toString(),
                assetObj["browser_download_url"].toString() });
        }

    }
    return result;
}

void GithubDownloader::download(const QList<GithubDownloader::Request>& requests)
{
    if (!progressDialog) {
        progressDialog = new QProgressDialog(parent);

        auto bar = new QProgressBar(progressDialog);
        bar->setTextVisible(false);
        progressDialog->setBar(bar);

        progressDialog->setWindowTitle("Загрузка");
        progressDialog->setMinimumWidth(400);
        progressDialog->setMinimumDuration(0);
        progressDialog->setRange(0, 0);
        progressDialog->setModal(true);
        connect(progressDialog, SIGNAL(canceled()), this, SLOT(onCancel()));
    }
    progressDialog->setLabelText("");
    progressDialog->show();

    foreach (const auto& request, requests) {
        QUrl url(request.url);
        auto reply = networkManager->get(makeRequest(url, request.format));
        downloadRequests[url.toString()] = DownloadDesc(request.dstPath, reply);
        connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
                this, SLOT(onDownload(qint64,qint64)));
    }
}

void GithubDownloader::replyFinished(QNetworkReply* reply)
{
    auto request = reply->request();
    auto key = request.url().toString();
    qDebug() << "Got answer for request: " << key;

    auto body = reply->readAll();

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
            emit finishedDownload();
        }
    }
    reply->deleteLater();
}

void GithubDownloader::onDownload(qint64 bytesReceived, qint64 /*bytesTotal*/)
{
    auto sender = QObject::sender();
    int received = 0;
    for (auto it = downloadRequests.begin(); it != downloadRequests.end(); ++it) {
        DownloadDesc& desc = it.value();
        if (desc.reply == sender)
            desc.bytesReceived = bytesReceived;
        received += static_cast<int>(desc.bytesReceived);
    }

    progressDialog->setLabelText(QString("Загружено: %1 МБ").arg(received / (1024.0 * 1024.0)));
}

void GithubDownloader::onCancel()
{
    foreach (const auto& desc, downloadRequests.values()) {
        if (desc.isRunning)
            desc.reply->abort();
    }
    progressDialog->hide();
    emit cancelled();
}

void GithubDownloader::reportBrokenSource()
{
    sourceStatus = SourceStatus::Broken;
    emit broken();
}

void GithubDownloader::processReleases(QNetworkReply* reply, const QByteArray& body)
{
    if (reply->error() != QNetworkReply::NoError)
        return reportBrokenSource();

    QJsonDocument json = QJsonDocument::fromJson(body);
    if (json.isArray()) {
        lastReleasesAnswer = body;
        sourceStatus = SourceStatus::OK;

        auto releases = json.array();
        QStringList versions;
        versions.reserve(releases.size());
        foreach (auto jsonValue, releases) {
            auto releaseObj = jsonValue.toObject();
            auto versionStr = releaseObj[VERSION_KEY].toString();
            if (!versionStr.isEmpty() && versionStr[0] == 'v')
                versionStr = versionStr.mid(1);
            versions << versionStr;
        }

        emit finishedUpdate(versions);
        return;
    }
    reportBrokenSource();
}

void GithubDownloader::processDownload(QNetworkReply* reply, const QByteArray& body)
{
    if (reply->error() != QNetworkReply::NoError) {
        emit errorDuringDownload();
        return;
    }

    auto downloadsDir = Settings::instance().downloadsDir();
    if (downloadsDir.check() != SourceStatus::OK) {
        emit errorDuringDownload();
        return;
    }
    const auto& desc = downloadRequests[reply->request().url().toString()];
    QFile resultFile(desc.resultFileName);
    if (!resultFile.open(QIODevice::WriteOnly)) {
        emit errorDuringDownload();
        return;
    }
    resultFile.write(body);
}
