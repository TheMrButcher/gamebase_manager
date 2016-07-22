#include "app.h"
#include "settings.h"
#include "files.h"
#include <QHash>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileInfo>

#include <QDebug>

namespace {

const QString SET_CONFIG_PREFIX = "app.setConfig(\"";
const QString SET_CONFIG_SUFFIX = "\")";

bool getDeployedConfigsDir(QDir& dir)
{
    auto workingDir = Settings::instance().workingDir();
    if (workingDir.check() != SourceStatus::OK)
        return false;

    dir = QDir(workingDir.path);
    if (!dir.cd(Files::DEPLOYED_ROOT_DIR_NAME)
        || !dir.cd(Files::CONTRIB_DIR_NAME)
        || !dir.cd(Files::BIN_DIR_NAME))
        return false;
    return true;
}

QString changeRootPath(QDir oldRoot, QDir newRoot, QString path)
{
    QFileInfo file(oldRoot, path);
    return newRoot.relativeFilePath(file.canonicalFilePath());
}

}

uint qHash(const App& app, uint seed)
{
    return qHash(app.source, seed) ^ qHash(app.version.toString(), seed)
        ^ qHash(app.containerName, seed) ^ qHash(static_cast<int>(app.state), seed)
        ^ qHash(app.name, seed);
}

bool App::validate()
{
    App expected = *this;
    *this = fromFileSystem(source, containerName);
    return exists();
}

bool App::checkAbility(App::Ability ability) const
{
    switch (ability) {
    case Configure: return (state == NotConfigured || state == Full)
                && source.type == AppSource::WorkingDirectory;
    case Remove: return state != Absent;
    case Compress: return (state == NotConfigured || state == Full)
                && source.type == AppSource::WorkingDirectory;
    case Add: return state != Absent
                && source.type != AppSource::WorkingDirectory;
    case Deploy: return state == Full
                && source.type == AppSource::WorkingDirectory;
    default: break;
    }
    return false;
}

App App::afterAction(App::Ability ability) const
{
    if (!checkAbility(ability))
        return makeAbsent(source);
    switch (ability) {
    case Configure: return App{ source, Full, name, version, containerName };
    case Compress: return App{ source, Archived, name, version, containerName + ".zip" };
    case Add:
    {
        auto workingDir = Settings::instance().workingDir();
        auto newContainerName = makeContainerName(QDir(workingDir.path), name);
        return App{
            AppSource{ AppSource::WorkingDirectory, workingDir.path, SourceStatus::Unknown },
            NotConfigured, name, version, newContainerName };
    }
    default: break;
    }
    return makeAbsent(source);
}

bool App::exists() const
{
    return state != Absent;
}

bool App::copyConfig()
{
    if (!checkAbility(Configure))
        return false;
    if (source.check() != SourceStatus::OK)
        return false;
    QDir srcDir(source.path);
    if (!srcDir.cd(containerName))
        return false;
    if (!srcDir.exists(Files::APP_CONFIG_NAME))
        return false;
    QDir dstDir;
    if (!getDeployedConfigsDir(dstDir))
        return false;

    QFile srcFile(srcDir.absoluteFilePath(Files::APP_CONFIG_NAME));
    if (!srcFile.open(QIODevice::ReadOnly))
        return false;
    QJsonDocument json = QJsonDocument::fromJson(srcFile.readAll());
    QJsonObject rootObj = json.object();
    rootObj["shadersPath"] = changeRootPath(srcDir, dstDir, rootObj["shadersPath"].toString());
    rootObj["imagesPath"] = changeRootPath(srcDir, dstDir, rootObj["imagesPath"].toString());
    rootObj["fontsPath"] = changeRootPath(srcDir, dstDir, rootObj["fontsPath"].toString());
    rootObj["designPath"] = changeRootPath(srcDir, dstDir, rootObj["designPath"].toString());

    QFile dstFile(dstDir.absoluteFilePath(containerName + Files::APP_CONFIG_NAME));
    if (!dstFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream stream(&dstFile);
    stream << QString::fromUtf8(QJsonDocument(rootObj).toJson());
    return true;
}

void App::removeConfig()
{
    QDir dstDir;
    if (!getDeployedConfigsDir(dstDir))
        return;
    dstDir.remove(containerName + Files::APP_CONFIG_NAME);
}

bool App::updateMainCpp()
{
    if (!checkAbility(Configure))
        return false;
    if (source.check() != SourceStatus::OK)
        return false;
    QDir dir(source.path);
    dir.cd(containerName);

    bool found = false;
    {
        QFile srcFile(dir.absoluteFilePath("main.cpp"));
        if (!srcFile.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;

        QFile dstFile(dir.absoluteFilePath("main.cpp_temp"));
        if (!dstFile.open(QIODevice::WriteOnly | QIODevice::Text))
            return false;

        QTextStream inStream(&srcFile);
        QTextStream outStream(&dstFile);
        while (!inStream.atEnd()) {
            auto line = inStream.readLine();
            int startIndex = line.indexOf(SET_CONFIG_PREFIX);
            if (startIndex != -1) {
                int endIndex = line.indexOf(SET_CONFIG_SUFFIX, startIndex + SET_CONFIG_PREFIX.length());
                if (endIndex != -1) {
                    found = true;
                    line = line.mid(0, startIndex + SET_CONFIG_PREFIX.length())
                            + containerName + Files::APP_CONFIG_NAME
                            + line.mid(endIndex);
                }
            }
            outStream << line << endl;
        }
    }

    dir.remove("main.cpp");
    dir.rename("main.cpp_temp", "main.cpp");

    return found;
}

App App::fromFileSystem(const AppSource& source, QString containerName)
{
    QDir dir(source.path);
    if (!dir.exists())
        return makeAbsent(source);
    if (dir.cd(containerName)) {
        State state = Absent;
        QString name;
        Version version;
        if (dir.exists("main.cpp")) {
            auto slnFiles = dir.entryInfoList(QStringList("*.sln"), QDir::Files);
            if (slnFiles.size() == 1) {
                name = slnFiles.first().baseName();
                state = NotConfigured;
            }
        }
        bool hasAllFiles = true;
        if (!dir.exists("VERSION.txt")
            || !version.read(dir.absoluteFilePath("VERSION.txt")))
            hasAllFiles = false;
        if (!dir.exists(Files::APP_CONFIG_NAME)
            || !dir.exists(Files::APP_PROJECT_NAME))
            hasAllFiles = false;

        {
            QDir workingDir;
            if (!version.read(dir.absoluteFilePath("VERSION.txt"))
                || !getDeployedConfigsDir(workingDir)
                || !workingDir.exists(containerName + Files::APP_CONFIG_NAME))
                hasAllFiles = false;
        }
        if (state == NotConfigured && hasAllFiles && source.type != AppSource::Directory)
            state = Full;
        return App{ source, state, name, version, containerName };
    } else {
        return makeAbsent(source);
    }
    return makeAbsent(source);
}

App App::makeAbsent(const AppSource& source)
{
    return App{ source, Absent, "", Version{}, "" };
}

App App::makeAbsent()
{
    return makeAbsent(AppSource{ AppSource::None, "", SourceStatus::Broken });
}

QString App::makeContainerName(QDir dir, QString baseName)
{
    if (dir.exists(baseName)) {
        for (int i = 2; i < 10; ++i) {
            QString containerName = baseName + QString::number(i);
            if (!dir.exists(containerName)) {
                return containerName;
            }
        }
    } else {
        return baseName;
    }
    return QString();
}