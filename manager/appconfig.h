#ifndef APPCONFIG_H
#define APPCONFIG_H

#include "config.h"

class AppConfig : public Config {
public:
    QString imagesPath;
    QString designPath;
    QString shadersPath;
    QString fontsPath;
    QString soundsPath;
    QString musicPath;
    QString additionalFontsPath;
    int width;
    int height;
    bool isWindow;

    static AppConfig defaultConfig();
    static AppConfig makeDeployedAppConfig(
            QDir rootDir, const AppConfig& originConfig);

private:
    virtual bool readImpl(QDir rootDir) override;
    virtual bool writeImpl(QDir rootDir, QJsonObject& newRootObj) const override;
};

#endif // APPCONFIG_H
