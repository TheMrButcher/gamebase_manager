#ifndef APP_H
#define APP_H

#include "appsource.h"
#include "version.h"
#include <QDir>

class App
{
public:
    AppSource source;

    enum State {
        Absent,
        Archived,
        NotConfigured,
        Full
    };
    State state;
    QString name;
    Version version;
    QString containerName;

    enum Ability {
        Configure,
        Remove,
        Compress,
        Add,
        Deploy
    };

    bool validate();
    bool checkAbility(Ability ability) const;
    App afterAction(Ability ability) const;
    bool exists() const;
    bool copyConfig();
    void removeConfig();
    bool updateMainCpp();

    static App fromFileSystem(const AppSource& source, QString containerName);
    static App makeAbsent(const AppSource& source);
    static App makeAbsent();
    static QString makeContainerName(QDir dir, QString baseName);
};

inline operator==(const App& app1, const App& app2)
{
    return app1.source == app2.source
            && app1.name == app2.name
            && app1.state == app2.state
            && app1.version == app2.version
            && app1.containerName == app2.containerName;
}

uint qHash(const App& app, uint seed);

#endif // APP_H
