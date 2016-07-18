#ifndef GITHUBLIBRARYDOWNLOADER_H
#define GITHUBLIBRARYDOWNLOADER_H

#include "librarysourcemanager.h"
#include "library.h"
#include <QNetworkAccessManager>
#include <QByteArray>

class QJsonObject;

class GithubLibraryDownloader : public LibrarySourceManager
{
    Q_OBJECT

public:
    GithubLibraryDownloader(const LibrarySource& source, QObject* parent);

    virtual void update() override;
    virtual void download(const Library& library) override;

private slots:
    void replyFinished(QNetworkReply* reply);

private:
    void startDownload(const QJsonObject& releaseObj, const Library& library, Library resultLibrary);
    void reportBrokenSource();
    void processReleases(QNetworkReply* reply, const QByteArray& body);
    void processDownload(QNetworkReply* reply, const QByteArray& body, const Library& resultLibrary);

    LibrarySource source;
    QNetworkAccessManager* networkManager;
    QString apiUrl;
    bool fatalError;

    QByteArray lastReleasesAnswer;
    QList<Library> cachedLibraries;

    struct DownloadDesc {
        enum Type {
            Binaries,
            Sources
        };
        Type type;
        Library library;
        QString resultFileName;
    };

    QHash<QString, DownloadDesc> downloadRequests;
    QHash<QString, QList<QString>> filesToDownload;
};

#endif // GITHUBLIBRARYDOWNLOADER_H
