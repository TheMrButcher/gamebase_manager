#ifndef APPSOURCE_H
#define APPSOURCE_H

#include "sourcestatus.h"
#include <QString>

class AppSource
{
public:
    enum Type {
        None,
        Directory,
        WorkingDirectory
    };

    Type type;
    QString path;
    SourceStatus status;
};

inline operator==(const AppSource& s1, const AppSource& s2)
{
    return s1.path == s2.path;
}

#endif // APPSOURCE_H
