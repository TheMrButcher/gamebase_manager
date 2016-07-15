#ifndef APPSOURCE_H
#define APPSOURCE_H

#include "sourcestatus.h"
#include <QString>

class AppSource
{
public:
    QString path;
    SourceStatus status;
};

#endif // APPSOURCE_H
