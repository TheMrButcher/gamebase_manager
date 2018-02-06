#include "appconfig.h"
#include "files.h"
#include "settings.h"
#include "librarysource.h"
#include <QJsonObject>
#include <QJsonArray>

bool AppConfig::readImpl(QDir rootDir)
{
    imagesPath = absPathOr(rootDir, rootObj["imagesPath"].toString(), imagesPath);
    designPath = absPathOr(rootDir, rootObj["designPath"].toString(), designPath);
    shadersPath = absPathOr(rootDir, rootObj["shadersPath"].toString(), shadersPath);
    if (rootObj["fontsPath"].isArray()) {
        auto fontsPathArray = rootObj["fontsPath"].toArray();
        if (fontsPathArray.size() > 0)
            fontsPath = absPathOr(rootDir, fontsPathArray[0].toString(), fontsPath);
        if (fontsPathArray.size() > 1)
            additionalFontsPath = absPathOr(
                    rootDir, fontsPathArray[1].toString(), additionalFontsPath);
    } else {
        fontsPath = absPathOr(rootDir, rootObj["fontsPath"].toString(), fontsPath);
        additionalFontsPath = "";
    }
    soundsPath = absPathOr(rootDir, rootObj["soundsPath"].toString(), soundsPath);
    musicPath = absPathOr(rootDir, rootObj["musicPath"].toString(), musicPath);
    width = rootObj["width"].toInt(width);
    height = rootObj["height"].toInt(height);
    QString mode = rootObj["mode"].toString();
    if (!mode.isEmpty())
        isWindow = mode == "window";
    return true;
}

bool AppConfig::writeImpl(QDir rootDir, QJsonObject& newRootObj) const
{
    newRootObj["version"] = "VER3";
    newRootObj["imagesPath"] = rootDir.relativeFilePath(imagesPath);
    newRootObj["designPath"] = rootDir.relativeFilePath(designPath);
    newRootObj["shadersPath"] = rootDir.relativeFilePath(shadersPath);
    if (additionalFontsPath.isEmpty()) {
        newRootObj["fontsPath"] = rootDir.relativeFilePath(fontsPath);
    } else {
        QJsonArray fontsPathArray;
        fontsPathArray.append(rootDir.relativeFilePath(fontsPath));
        fontsPathArray.append(rootDir.relativeFilePath(additionalFontsPath));
        newRootObj["fontsPath"] = fontsPathArray;
    }
    if (!soundsPath.isEmpty())
        newRootObj["soundsPath"] = rootDir.relativeFilePath(soundsPath);
    if (!musicPath.isEmpty())
        newRootObj["musicPath"] = rootDir.relativeFilePath(musicPath);
    newRootObj["width"] = width;
    newRootObj["height"] = height;
    newRootObj["mode"] = isWindow ? QString("window") : QString("fullscreen");
    return true;
}

AppConfig AppConfig::defaultConfig()
{
    AppConfig config;
    config.width = 1024;
    config.height = 768;
    config.isWindow = true;

    auto workingDir = Settings::instance().workingDir();
    if (workingDir.check() != SourceStatus::OK)
        return config;
    QDir dir(workingDir.path);
    if (!dir.cd(Files::DEPLOYED_ROOT_DIR_NAME))
        return config;
    if (!dir.cd(Files::RESOURCES_DIR_NAME))
        return config;

    config.imagesPath = dir.absoluteFilePath("images");
    config.designPath = dir.absoluteFilePath(Files::DESIGNS_DIR_NAME);
    config.shadersPath = dir.absoluteFilePath("shaders");
    config.fontsPath = dir.absoluteFilePath(Files::FONTS_DIR_NAME);
    config.soundsPath = "";
    config.musicPath = "";
    config.additionalFontsPath = "";
    return config;
}

AppConfig AppConfig::makeDeployedAppConfig(QDir rootDir, const AppConfig& originConfig)
{
    AppConfig config = originConfig;
    config.imagesPath = rootDir.absoluteFilePath("resources/images");
    config.designPath = rootDir.absoluteFilePath("resources/design");
    config.shadersPath = rootDir.absoluteFilePath("resources/shaders");
    config.fontsPath = rootDir.absoluteFilePath("resources/fonts");
    if (!originConfig.soundsPath.isEmpty())
        config.soundsPath = rootDir.absoluteFilePath("resources/sounds");
    if (!originConfig.musicPath.isEmpty())
        config.musicPath = rootDir.absoluteFilePath("resources/music");
    config.additionalFontsPath = "";
    return config;
}
