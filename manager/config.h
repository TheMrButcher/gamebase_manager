#ifndef CONFIG_H
#define CONFIG_H

#include <QString>
#include <QDir>
#include <QJsonObject>

class Config {
public:
    bool read(QDir rootDir, QString path);
    bool write(QDir rootDir, QString path) const;

protected:
    static QString absPathOr(QDir rootDir, QString value, QString defaultValue);

    virtual bool readImpl(QDir rootDir) = 0;
    virtual bool writeImpl(QDir rootDir, QJsonObject& newRootObj) const = 0;

    QJsonObject rootObj;
};

#endif // CONFIG_H
