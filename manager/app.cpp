#include "app.h"
#include "settings.h"
#include "files.h"
#include "appconfig.h"
#include "archive.h"
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
const QString SET_CONFIG_SUFFIX = "\");";
const QString DEFINE_APP_LINE = "MyApp app;";

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

bool updateMainCppImpl(QDir dir, QString configName, int fixIteration)
{
    QFile srcFile(dir.absoluteFilePath("main.cpp"));
    if (!srcFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QFile dstFile(dir.absoluteFilePath("main.cpp_temp"));
    if (!dstFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    bool found = false;
    QTextStream inStream(&srcFile);
    QTextStream outStream(&dstFile);
    while (!inStream.atEnd()) {
        auto line = inStream.readLine();
        if (fixIteration == 0) {
            int startIndex = line.indexOf(SET_CONFIG_PREFIX);
            if (startIndex != -1) {
                int endIndex = line.indexOf(SET_CONFIG_SUFFIX, startIndex + SET_CONFIG_PREFIX.length());
                if (endIndex != -1) {
                    found = true;
                    line = line.mid(0, startIndex + SET_CONFIG_PREFIX.length())
                            + configName
                            + line.mid(endIndex);
                }
            }
        }
        outStream << line << endl;
        if (fixIteration == 1) {
            int startIndex = line.indexOf(DEFINE_APP_LINE);
            if (startIndex != -1) {
                found = true;
                QString newLine = line.mid(0, startIndex) +
                        SET_CONFIG_PREFIX + configName +
                        SET_CONFIG_SUFFIX;
                outStream << newLine << endl;
            }
        }
    }
    return found;
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
                && (source.type != AppSource::WorkingDirectory || state == Archived);
    case Deploy: return state == Full
                && source.type == AppSource::WorkingDirectory;
    case OpenSolution: return state == NotConfigured || state == Full;
    case OpenDirectory: return state != Absent;
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

bool App::exists(QString fileName) const
{
    QDir dir(source.path);
    if (!dir.cd(containerName))
        return false;
    return dir.exists(fileName);
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

    AppConfig config;
    if (!config.read(srcDir, srcDir.absoluteFilePath(Files::APP_CONFIG_NAME)))
        return false;
    return config.write(dstDir, dstDir.absoluteFilePath(containerName + Files::APP_CONFIG_NAME));
}

AppConfig App::config()
{
    AppConfig config = AppConfig::defaultConfig();
    QDir srcDir(source.path);
    if (!srcDir.cd(containerName))
        return config;
    if (!srcDir.exists(Files::APP_CONFIG_NAME))
        return config;
    config.read(srcDir, srcDir.absoluteFilePath(Files::APP_CONFIG_NAME));
    return config;
}

bool App::setConfig(const AppConfig& config)
{
    QDir dir(source.path);
    if (!dir.cd(containerName))
        return false;
    return config.write(dir, dir.absoluteFilePath(Files::APP_CONFIG_NAME));
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
    return updateMainCpp(containerName + Files::APP_CONFIG_NAME);
}

bool App::updateMainCpp(QString configName)
{
    if (!checkAbility(Configure))
        return false;
    if (source.check() != SourceStatus::OK)
        return false;
    QDir dir(source.path);
    dir.cd(containerName);

    bool found = updateMainCppImpl(dir, configName, 0);
    if (!found)
        found = updateMainCppImpl(dir, configName, 1);

    dir.remove("main.cpp");
    dir.rename("main.cpp_temp", "main.cpp");

    return found;
}

bool App::isMainCppOK()
{
    if (!checkAbility(Configure))
        return false;
    if (source.check() != SourceStatus::OK)
        return false;
    QDir dir(source.path);
    dir.cd(containerName);

    QFile srcFile(dir.absoluteFilePath("main.cpp"));
    if (!srcFile.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream inStream(&srcFile);
    while (!inStream.atEnd()) {
        auto line = inStream.readLine();
        int startIndex = line.indexOf(SET_CONFIG_PREFIX);
        if (startIndex != -1) {
            int endIndex = line.indexOf(SET_CONFIG_SUFFIX, startIndex + SET_CONFIG_PREFIX.length());
            if (endIndex != -1)
                return true;
        }
    }
    return false;
}

bool App::configurate()
{
    QDir dir(source.path);
    if (!dir.cd(containerName))
        return false;

    bool isAllOK = true;
    if (!setConfig(config()))
        isAllOK = false;
    if (!copyConfig())
        isAllOK = false;
    if (version.empty()) {
        if (!version.read(dir.absoluteFilePath("VERSION.txt")))
            version = Version::fromString("1.0.0");
    }
    if (!version.write(dir.absoluteFilePath("VERSION.txt")))
        isAllOK = false;
    if (!updateMainCpp())
        isAllOK = false;
    if (!dir.exists("ManagerProject.json")) {
        QFile projFile(dir.absoluteFilePath("ManagerProject.json"));
        if (projFile.open(QIODevice::WriteOnly))
            projFile.write("{}");
        else
            isAllOK = false;
    }
    return isAllOK;
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
        if (!dir.exists(Files::VERSION_FILE_NAME)
            || !version.read(dir.absoluteFilePath(Files::VERSION_FILE_NAME)))
            hasAllFiles = false;
        if (!dir.exists(Files::APP_CONFIG_NAME)
            || !dir.exists(Files::APP_PROJECT_NAME))
            hasAllFiles = false;

        {
            QDir workingDir;
            if (!version.read(dir.absoluteFilePath(Files::VERSION_FILE_NAME))
                || !getDeployedConfigsDir(workingDir)
                || !workingDir.exists(containerName + Files::APP_CONFIG_NAME))
                hasAllFiles = false;
        }
        if (state == NotConfigured && hasAllFiles && source.type != AppSource::Directory)
            state = Full;
        return App{ source, state, name, version, containerName };
    } else {
        Archive archive(dir.absoluteFilePath(containerName));
        if (!archive.open())
            return makeAbsent(source);
        const auto& root = archive.root();
        if (root.exists("main.cpp")) {
            auto slnFiles = root.entryInfoList(QStringList("*.sln"), QDir::Files);
            if (slnFiles.size() == 1) {
                QString name = slnFiles.first().name;
                name = name.mid(0, name.size() - 4);
                Version version = archive.exctractVersion();
                return App{ source, Archived, name, version, containerName };
            }
        }
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

QString App::makeContainerName(QDir dir, QString baseName, int maxIndex)
{
    if (dir.exists(baseName)) {
        for (int i = 2; i < maxIndex; ++i) {
            QString containerName = baseName + "_" + QString::number(i);
            if (!dir.exists(containerName)) {
                return containerName;
            }
        }
    } else {
        return baseName;
    }
    return QString();
}

bool App::createSolution(QDir dir, QString name)
{
    QDir pkgDir(Settings::instance().workingDir().path);
    if (!pkgDir.cd(Files::DEPLOYED_ROOT_DIR_NAME)
        || !pkgDir.cd(Files::PACKAGE_DIR_NAME)
        || !pkgDir.cd("project_template"))
        return false;

    QFile srcSolution(pkgDir.absoluteFilePath("project_template.sln"));
    if (!srcSolution.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QString solutionData = QString::fromUtf8(srcSolution.readAll());
    solutionData.replace("project_template", name);
    if (!Files::writeTextFile(solutionData, dir.absoluteFilePath(name + ".sln"), true))
        return false;
    return true;
}
