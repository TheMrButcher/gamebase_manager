#ifndef VERSION_H
#define VERSION_H

#include <QStringList>

class Version
{
public:
    QString toString() const;

    static Version read(QString fileName);

    QStringList version;
};

bool operator==(const Version& v1, const Version& v2);
bool operator<(const Version& v1, const Version& v2);

#endif // VERSION_H
