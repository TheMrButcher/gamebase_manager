#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QString>
#include <QDir>
#include <QJsonObject>

class AppConfig {
public:
    QString imagesPath;
    QString designPath;
    QString shadersPath;
    QString fontsPath;
    int width;
    int height;
    bool isWindow;

    bool read(QDir rootDir, QString path);
    bool write(QDir rootDir, QString path) const;

    static AppConfig defaultConfig();

private:
    QJsonObject rootObj;
};

#endif // APPCONFIG_H
