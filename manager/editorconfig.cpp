#include "editorconfig.h"
#include "files.h"
#include "settings.h"
#include <QJsonArray>

bool EditorConfig::readImpl(QDir rootDir)
{
    isVirtual = false;
    workingPath = absPathOr(rootDir, rootObj["workingPath"].toString(), workingPath);
    designedObjectImagesPath = absPathOr(
        rootDir, rootObj["designedObjectImagesPath"].toString(), designedObjectImagesPath);
    soundsPath = absPathOr(rootDir, rootObj["soundsPath"].toString(), soundsPath);
    musicPath = absPathOr(rootDir, rootObj["musicPath"].toString(), musicPath);
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
    width = rootObj["width"].toInt(width);
    height = rootObj["height"].toInt(height);
    QString mode = rootObj["mode"].toString();
    if (!mode.isEmpty())
        isWindow = mode == "window";
    return true;
}

bool EditorConfig::writeImpl(QDir rootDir, QJsonObject& newRootObj) const
{
    newRootObj["version"] = "VER3";
    newRootObj["workingPath"] = rootDir.relativeFilePath(workingPath);
    newRootObj["designedObjectImagesPath"] = rootDir.relativeFilePath(designedObjectImagesPath);
    if (!soundsPath.isEmpty())
        newRootObj["soundsPath"] = rootDir.relativeFilePath(soundsPath);
    if (!musicPath.isEmpty())
        newRootObj["musicPath"] = rootDir.relativeFilePath(musicPath);
    if (additionalFontsPath.isEmpty()) {
        newRootObj["fontsPath"] = rootDir.relativeFilePath(fontsPath);
    } else {
        QJsonArray fontsPathArray;
        fontsPathArray.append(rootDir.relativeFilePath(fontsPath));
        fontsPathArray.append(rootDir.relativeFilePath(additionalFontsPath));
        newRootObj["fontsPath"] = fontsPathArray;
    }
    newRootObj["width"] = width;
    newRootObj["height"] = height;
    newRootObj["mode"] = isWindow ? QString("window") : QString("fullscreen");
    return true;
}

QDir EditorConfig::editorDir()
{
    auto workingDir = Settings::instance().workingDir().path;
    QDir dir(workingDir);
    if (!dir.cd(Files::DEPLOYED_ROOT_DIR_NAME))
        return dir;
    if (!dir.cd(Files::CONTRIB_DIR_NAME))
        return dir;
    if (!dir.cd(Files::BIN_DIR_NAME))
        return dir;
    if (!dir.cd(Files::RELEASE_DIR_NAME))
        return dir;
    return dir;
}

QString EditorConfig::editorConfigPath()
{
    auto workingDir = Settings::instance().workingDir().path;
    QDir dir(workingDir);
    if (!dir.cd(Files::DEPLOYED_ROOT_DIR_NAME))
        return QString();
    if (!dir.cd(Files::CONTRIB_DIR_NAME))
        return QString();
    if (!dir.cd(Files::BIN_DIR_NAME))
        return QString();
    if (!dir.cd(Files::EDITOR_DIR_NAME))
        return QString();
    return dir.absoluteFilePath(Files::EDITOR_CONFIG_NAME);
}

QString EditorConfig::editorPath()
{
    return editorDir().absoluteFilePath(Files::EDITOR_PROJECT_NAME + ".exe");
}

EditorConfig& EditorConfig::instance()
{
    static EditorConfig config;
    return config;
}

bool EditorConfig::read()
{
    EditorConfig config;
    QDir rootDir = editorDir();
    if (!config.Config::read(rootDir, editorConfigPath()))
        config = EditorConfig();
    *this = config;
    return !isVirtual;
}

bool EditorConfig::write()
{
    QDir rootDir = editorDir();
    return Config::write(rootDir, editorConfigPath());
}
