#ifndef GITHUBLIBRARYDOWNLOADER_H
#define GITHUBLIBRARYDOWNLOADER_H

#include "librarysourcemanager.h"
#include "library.h"
#include <QNetworkAccessManager>
#include <QByteArray>

class QJsonObject;
class QProgressDialog;

class GithubLibraryDownloader : public LibrarySourceManager
{
    Q_OBJECT

public:
    GithubLibraryDownloader(const LibrarySource& source, QWidget* parent);

    virtual void update() override;
    virtual void download(const Library& library) override;

private slots:
    void replyFinished(QNetworkReply* reply);
    void onDownload(qint64 bytesReceived, qint64 bytesTotal);
    void onCancel();

private:
    void startDownload(const QJsonObject& releaseObj, const Library& library, Library resultLibrary);
    void reportBrokenSource();
    void processReleases(QNetworkReply* reply, const QByteArray& body);
    void processDownload(QNetworkReply* reply, const QByteArray& body);

    LibrarySource source;
    QNetworkAccessManager* networkManager;
    QString apiUrl;
    bool fatalError;
    QWidget* parent;

    QByteArray lastReleasesAnswer;
    QList<Library> cachedLibraries;

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

    Library libraryToDownload;
    QHash<QString, DownloadDesc> downloadRequests;
    QProgressDialog* progressDialog;
};

#endif // GITHUBLIBRARYDOWNLOADER_H
