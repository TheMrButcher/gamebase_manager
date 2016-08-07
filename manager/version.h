#ifndef VERSION_H
#define VERSION_H

#include <QStringList>

class Version
{
public:
    QString toString() const;

    void set(QString versionStr);
    bool read(QString fileName);
    bool write(QString fileName);
    bool empty() const { return version.empty(); }

    static Version fromString(QString versionStr);

    QStringList version;
};

bool operator==(const Version& v1, const Version& v2);
bool operator<(const Version& v1, const Version& v2);

#endif // VERSION_H
