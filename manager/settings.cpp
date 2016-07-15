#include "settings.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

namespace {
const QString DEFAULT_WORKING_DIR = "programs";
const LibrarySource DEFAULT_LIBRARY_SOURCE{
    LibrarySource::Server,
    "https://github.com/TheMrButcher/gamebase",
    SourceStatus::Unknown };
}

bool Settings::read(QString fname)
{
    QFile settingsFile(fname);
    if (!settingsFile.open(QIODevice::ReadOnly))
        return false;
    QJsonParseError errors;
    QJsonDocument json = QJsonDocument::fromJson(settingsFile.readAll(), &errors);
    if (errors.error != QJsonParseError::NoError)
        return false;
    if (!json.isObject())
        return false;

    auto rootObj = json.object();
    workingDir = rootObj["workingDir"].toString(workingDir);

    auto librarySourcesArray = rootObj["librarySources"].toArray();
    librarySources.clear();
    foreach (auto sourceValue, librarySourcesArray) {
        auto sourceObj = sourceValue.toObject();
        auto path = sourceObj["path"].toString();
        if (path.isEmpty())
            continue;
        auto type = sourceObj["type"].toString("directory") == "directory"
                ? LibrarySource::Directory : LibrarySource::Server;
        librarySources.append(LibrarySource{ type, path, SourceStatus::Unknown });
    }

    auto appSourcesArray = rootObj["appSources"].toArray();
    appSources.clear();
    foreach (auto sourceValue, appSourcesArray) {
        auto sourceObj = sourceValue.toObject();
        auto path = sourceObj["path"].toString();
        if (path.isEmpty())
            continue;
        appSources.append(AppSource{ path, SourceStatus::Unknown });
    }

    return true;
}

void Settings::write(QString fname)
{
    QFile settingsFile(fname);
    if (!settingsFile.open(QIODevice::WriteOnly))
        return;

    QJsonObject rootObj;
    rootObj["workingDir"] = workingDir;

    QJsonArray librarySourcesArray;
    foreach (auto source, librarySources) {
        QJsonObject sourceObj;
        sourceObj["path"] = source.path;
        sourceObj["type"] = source.type == LibrarySource::Directory
                ? "directory" : "server";
        librarySourcesArray.append(sourceObj);
    }
    rootObj["librarySources"] = librarySourcesArray;

    QJsonArray appSourcesArray;
    foreach (auto source, appSources) {
        QJsonObject sourceObj;
        sourceObj["path"] = source.path;
        appSourcesArray.append(sourceObj);
    }
    rootObj["appSources"] = appSourcesArray;

    QJsonDocument json(rootObj);
    settingsFile.write(json.toJson());
}

Settings Settings::defaultValue()
{
    QList<LibrarySource> librarySources;
    librarySources.append(DEFAULT_LIBRARY_SOURCE);
    return Settings{ QDir().absoluteFilePath(DEFAULT_WORKING_DIR), librarySources };
}

Settings& Settings::instance()
{
    static Settings settings;
    return settings;
}


