#include "selfupdater.h"
#include "settings.h"
#include "githubupdatedownloader.h"
#include "files.h"
#include <QDir>
#include <QCoreApplication>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QProcess>

namespace {
QList<UpdateDescriptor> formUpdatePath(const QList<UpdateDescriptor>& allUpdates)
{
    QList<UpdateDescriptor> result;
    if (allUpdates.empty())
        return result;

    Version from = Version::selfVersion();
    Version to = allUpdates.front().to;
    foreach (auto update, allUpdates)
        to = std::max(to, update.to);
    if (to <= from)
        return result;

    QHash<Version, UpdateDescriptor> updateMap;
    updateMap.reserve(allUpdates.size());
    foreach (auto update, allUpdates) {
        if (update.from < from)
            continue;
        if (updateMap.contains(update.to)) {
            auto& updateInMap = updateMap[update.to];
            if (update.from < updateInMap.from)
                updateInMap = update;
        } else {
            updateMap[update.to] = update;
        }
    }

    Version cur = to;
    while (cur != from) {
        if (!updateMap.contains(cur)) {
            return QList<UpdateDescriptor>();
        }
        const auto& update = updateMap[cur];
        result.prepend(update);
        cur = update.from;
    }

    return result;
}
}

SelfUpdater::SelfUpdater(QObject *parent) : QObject(parent)
{}

void SelfUpdater::checkDownloadsDir()
{
    auto downloadsDir = Settings::instance().downloadsDir();
    if (downloadsDir.check() != SourceStatus::OK)
        return;
    QDir dir(downloadsDir.path);
    auto children = dir.entryList(QDir::Files);

    QList<UpdateDescriptor> allUpdates;
    allUpdates.reserve(children.size());
    foreach (auto child, children) {
        if (!UpdateDescriptor::isUpdate(child))
            continue;
        allUpdates.append(UpdateDescriptor("", child));
    }
    updates = formUpdatePath(allUpdates);
    emit hasUpdates(!updates.empty());
}

Version SelfUpdater::targetVersion()
{
    if (updates.empty())
        return Version::selfVersion();
    return updates.back().to;
}

void SelfUpdater::updateApp()
{
    if (updates.empty())
        return;
    QDir workingDir(QCoreApplication::applicationDirPath());
    if (!workingDir.cdUp())
        return;

    auto scriptPath = workingDir.absoluteFilePath(Files::UPDATE_SCRIPT_NAME);
    if (!Files::copyTextFile(":/scripts/" + Files::UPDATE_SCRIPT_NAME, scriptPath))
        return;

    auto downloadsDir = Settings::instance().downloadsDir();
    if (downloadsDir.check() != SourceStatus::OK)
        return;
    QDir updatesDir(downloadsDir.path);

    QFile file(scriptPath);
    if (!file.open(QIODevice::Append | QIODevice::Text))
        return;
    QTextStream stream(&file);

    foreach (const auto& update, updates) {
        auto path = updatesDir.absoluteFilePath(update.fileName);
        stream << "start \"\" /wait /d . \"" << path << "\"\n"
               << "if %ERRORLEVEL% NEQ 0 goto fail\n";
    }
    stream << "goto success\n";
    stream.flush();
    file.close();

    QProcess::startDetached("cmd.exe", QStringList() << "/C" << scriptPath, workingDir.absolutePath());
    QApplication::quit();
}

void SelfUpdater::onUpdatesListDownloaded(const QList<UpdateDescriptor>& availableUpdates)
{
    if (Settings::instance().downloadsDir().check() != SourceStatus::OK)
        return;
    auto allUpdates = updates + availableUpdates;
    auto newUpdatesPath = formUpdatePath(allUpdates);
    QList<UpdateDescriptor> updatesToDownload;
    updatesToDownload.reserve(availableUpdates.size());
    foreach (const auto& update, newUpdatesPath)
        if (!update.url.isEmpty())
            updatesToDownload.append(update);
    GithubUpdateDownloader::instance()->download(updatesToDownload, TaskMode::InBackground);
}

void SelfUpdater::onDownloadFinished()
{
    checkDownloadsDir();
}
