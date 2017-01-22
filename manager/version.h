#ifndef VERSION_H
#define VERSION_H

#include <QStringList>

class Version
{
public:
    QString toString() const;
    QStringList toStringList() const;

    void set(QString versionStr);
    bool read(QString fileName);
    bool write(QString fileName);
    bool empty() const { return version.empty(); }

    static Version fromString(QString versionStr);
    static Version fromFile(QString path);
    static Version selfVersion();

    QList<int> version;
};

bool operator==(const Version& v1, const Version& v2);
bool operator!=(const Version& v1, const Version& v2);
bool operator<(const Version& v1, const Version& v2);
bool operator<=(const Version& v1, const Version& v2);
uint qHash(const Version& version, uint seed);

#endif // VERSION_H
