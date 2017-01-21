#ifndef GITHUBDOWNLOADER_H
#define GITHUBDOWNLOADER_H

#include "sourcestatus.h"
#include <QNetworkAccessManager>
#include <QByteArray>

class QJsonObject;
class QProgressDialog;

class GithubDownloader : public QObject
{
    Q_OBJECT

public:
    GithubDownloader(QString sourceUrl, QWidget* parent);

    bool hasConnection() const;
    bool isBusy() const { return !downloadRequests.empty(); }
    SourceStatus check();
    SourceStatus status() const { return sourceStatus; }
    void updateInfo();

    struct Asset
    {
        QString name;
        QString url;
    };

    struct Release
    {
        QString sourcesUrl;
        QList<Asset> assets;
    };

    QList<Release> releases(QStringList versions);

    struct Request
    {
        QString url;
        QString dstPath;
        QByteArray format;
    };

    void download(const QList<Request>& requests);

signals:
    void broken();
    void cancelled();
    void finishedDownload();
    void errorDuringDownload();
    void finishedUpdate(QStringList);

private slots:
    void replyFinished(QNetworkReply* reply);
    void onDownload(qint64 bytesReceived, qint64 bytesTotal);
    void onCancel();

private:
    void reportBrokenSource();
    void processReleases(QNetworkReply* reply, const QByteArray& body);
    void processDownload(QNetworkReply* reply, const QByteArray& body);

    QString sourceUrl;
    SourceStatus sourceStatus;
    QNetworkAccessManager* networkManager;
    QString apiUrl;
    bool fatalError;
    QWidget* parent;

    QByteArray lastReleasesAnswer;

    struct DownloadDesc {
        DownloadDesc() {}
        DownloadDesc(QString resultFileName, QNetworkReply* reply)
            : resultFileName(resultFileName)
            , reply(reply)
            , bytesReceived(0)
            , bytesTotal(0)
            , isRunning(true)
        {}

        QString resultFileName;
        QNetworkReply* reply;
        qint64 bytesReceived;
        qint64 bytesTotal;
        bool isRunning;
    };

    QHash<QString, DownloadDesc> downloadRequests;
    QProgressDialog* progressDialog;
};

#endif // GITHUBDOWNLOADER_H
