#ifndef GITHUBLIBRARYDOWNLOADER_H
#define GITHUBLIBRARYDOWNLOADER_H

#include "librarysourcemanager.h"
#include "library.h"
#include "librarysource.h"
#include "githubdownloader.h"

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
    void onVersionsUpdated(QStringList versions);
    void onCancel();
    void reportBrokenSource();
    void onDownloadFinished();

private:
    GithubDownloader* downloader;
    LibrarySource source;
    QList<Library> cachedLibraries;
    Library libraryToDownload;
};

#endif // GITHUBLIBRARYDOWNLOADER_H
