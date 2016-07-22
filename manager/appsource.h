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

    SourceStatus check();
};

inline operator==(const AppSource& s1, const AppSource& s2)
{
    return s1.path == s2.path && s1.type == s2.type;
}

uint qHash(const AppSource& s, uint seed);

#endif // APPSOURCE_H
