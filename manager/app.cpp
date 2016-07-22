#include "app.h"
#include "settings.h"
#include "files.h"
#include <QHash>
#include <QDir>

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

bool App::exists() const
{
    return state != Absent;
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
            QDir workingDir(Settings::instance().workingDir().path);
            if (!version.read(dir.absoluteFilePath("VERSION.txt"))
                || !workingDir.cd(Files::DEPLOYED_ROOT_DIR_NAME)
                || !workingDir.cd(Files::CONTRIB_DIR_NAME)
                || !workingDir.cd(Files::BIN_DIR_NAME)
                || !workingDir.exists(containerName + Files::APP_CONFIG_NAME))
                hasAllFiles = false;
        }
        if (state == NotConfigured && hasAllFiles)
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
