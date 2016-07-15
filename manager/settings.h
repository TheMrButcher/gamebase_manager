#ifndef SETTINGS_H
#define SETTINGS_H

#include "librarysource.h"
#include "appsource.h"
#include <QString>
#include <QList>

class Settings
{
public:
    bool read(QString fname);
    void write(QString fname);

    static Settings defaultValue();
    static Settings& instance();

    QString workingDir;
    QList<LibrarySource> librarySources;
    QList<AppSource> appSources;
};

#endif // SETTINGS_H
