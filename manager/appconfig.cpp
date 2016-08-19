#include "appconfig.h"
#include "files.h"
#include "settings.h"
#include "librarysource.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

QString absPathOr(QDir rootDir, QString value, QString defaultValue)
{
    if (value.isEmpty())
        return defaultValue;
    return Files::absPath(rootDir, value);
}

}

bool AppConfig::read(QDir rootDir, QString path)
{
    QFile srcFile(path);
    if (!srcFile.open(QIODevice::ReadOnly))
        return false;
    QJsonDocument json = QJsonDocument::fromJson(srcFile.readAll());
    rootObj = json.object();
    imagesPath = absPathOr(rootDir, rootObj["imagesPath"].toString(), imagesPath);
    designPath = absPathOr(rootDir, rootObj["designPath"].toString(), designPath);
    shadersPath = absPathOr(rootDir, rootObj["shadersPath"].toString(), shadersPath);
    fontsPath = absPathOr(rootDir, rootObj["fontsPath"].toString(), fontsPath);
    width = rootObj["width"].toInt(width);
    height = rootObj["height"].toInt(height);
    QString mode = rootObj["mode"].toString();
    if (!mode.isEmpty())
        isWindow = mode == "window";
    return true;
}

bool AppConfig::write(QDir rootDir, QString path) const
{
    QFile dstFile(path);
    if (!dstFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QJsonObject newRootObj = rootObj;
    newRootObj["imagesPath"] = rootDir.relativeFilePath(imagesPath);
    newRootObj["designPath"] = rootDir.relativeFilePath(designPath);
    newRootObj["shadersPath"] = rootDir.relativeFilePath(shadersPath);
    newRootObj["fontsPath"] = rootDir.relativeFilePath(fontsPath);
    newRootObj["width"] = width;
    newRootObj["height"] = height;
    newRootObj["mode"] = isWindow ? QString("window") : QString("fullscreen");

    QTextStream stream(&dstFile);
    stream << QString::fromUtf8(QJsonDocument(newRootObj).toJson());
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
    return config;
}

AppConfig AppConfig::makeDeployedAppConfig(QDir rootDir, const AppConfig& originConfig)
{
    AppConfig config = originConfig;
    config.imagesPath = rootDir.absoluteFilePath("resources/images");
    config.designPath = rootDir.absoluteFilePath("resources/designs");
    config.shadersPath = rootDir.absoluteFilePath("resources/shaders");
    config.fontsPath = rootDir.absoluteFilePath("resources/fonts");
    return config;
}
