#include "settings.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcessEnvironment>

namespace {
const QString DEFAULT_WORKING_DIR = "programs";
const QString DEFAULT_DOWNLOADS_DIR = "downloads";
const LibrarySource DEFAULT_LIBRARY_SOURCE{
    LibrarySource::Server,
    "https://github.com/TheMrButcher/gamebase",
    SourceStatus::Unknown };
const QString MSVC_2010_TOOLS_KEY = "VS100COMNTOOLS";
const QString VC_VARS_BAT_NAME = "vcvarsall.bat";

QString extractVCVarsPath()
{
    auto env = QProcessEnvironment::systemEnvironment();
    if (!env.contains(MSVC_2010_TOOLS_KEY))
        return QString();
    QDir vcDir(env.value(MSVC_2010_TOOLS_KEY));
    if (!vcDir.exists())
        return QString();
    if (!vcDir.cdUp())
        return QString();
    if (!vcDir.cdUp())
        return QString();
    if (!vcDir.cd("VC"))
        return QString();
    if (!vcDir.exists(VC_VARS_BAT_NAME))
        return QString();
    return vcDir.absoluteFilePath(VC_VARS_BAT_NAME);
}
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
    auto workingDir = rootObj["workingDir"].toString(this->workingDir().path);
    auto downloadsDir = rootObj["downloadsDir"].toString(this->downloadsDir().path);
    vcVarsPath = rootObj["vcVarsPath"].toString(extractVCVarsPath());

    auto librarySourcesArray = rootObj["librarySources"].toArray();
    librarySources.clear();
    librarySources.append(LibrarySource{
        LibrarySource::WorkingDirectory, workingDir, SourceStatus::Unknown });
    librarySources.append(LibrarySource{
        LibrarySource::DownloadsDirectory, downloadsDir, SourceStatus::Unknown });
    foreach (auto sourceValue, librarySourcesArray) {
        auto sourceObj = sourceValue.toObject();
        auto path = sourceObj["path"].toString();
        if (path.isEmpty())
            continue;
        LibrarySource::Type type = LibrarySource::Directory;
        auto typeStr = sourceObj["type"].toString("directory");
        if (typeStr == "server")
            type = LibrarySource::Server;
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
    rootObj["workingDir"] = workingDir().path;
    rootObj["downloadsDir"] = downloadsDir().path;
    rootObj["vcVarsPath"] = vcVarsPath;

    QJsonArray librarySourcesArray;
    foreach (auto source, librarySources) {
        QJsonObject sourceObj;
        sourceObj["path"] = source.path;
        switch (source.type) {
        case LibrarySource::Server: sourceObj["type"] = "server"; break;
        case LibrarySource::Directory: sourceObj["type"] = "directory"; break;
        default: continue;
        }
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

LibrarySource Settings::workingDir() const
{
    foreach (const auto& source, librarySources)
        if (source.type == LibrarySource::WorkingDirectory)
            return source;
    return LibrarySource{ LibrarySource::None, "", SourceStatus::Broken };
}

LibrarySource Settings::downloadsDir() const
{
    foreach (const auto& source, librarySources)
        if (source.type == LibrarySource::DownloadsDirectory)
            return source;
    return LibrarySource{ LibrarySource::None, "", SourceStatus::Broken };
}

Settings Settings::defaultValue()
{
    QList<LibrarySource> librarySources;
    librarySources.append(DEFAULT_LIBRARY_SOURCE);

    QString workingPath = QDir().absoluteFilePath(DEFAULT_WORKING_DIR);
    librarySources.append(LibrarySource{
            LibrarySource::WorkingDirectory, workingPath, SourceStatus::Unknown });

    QString downloadsPath = QDir().absoluteFilePath(DEFAULT_DOWNLOADS_DIR);
    librarySources.append(LibrarySource{
            LibrarySource::DownloadsDirectory, downloadsPath, SourceStatus::Unknown });

    return Settings{ librarySources, QList<AppSource>(), extractVCVarsPath() };
}

Settings& Settings::instance()
{
    static Settings settings;
    return settings;
}
