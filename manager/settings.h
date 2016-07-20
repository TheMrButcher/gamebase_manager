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

    LibrarySource workingDir() const;
    LibrarySource downloadsDir() const;

    static Settings defaultValue();
    static Settings& instance();

    QList<LibrarySource> librarySources;
    QList<AppSource> appSources;
    QString vcVarsPath;
};

#endif // SETTINGS_H
