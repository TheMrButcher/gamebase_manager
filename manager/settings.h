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
    static QString extractVCVarsPath();

    QList<LibrarySource> librarySources;
    QList<AppSource> appSources;
    QString vcVarsPath;
    QString vcVersion;
    QString outputPath;
    bool isFirstUsage;
    QString selfUpdateSourceUrl;
};

#endif // SETTINGS_H
