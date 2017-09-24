#include "config.h"
#include "files.h"
#include <QTextStream>
#include <QJsonDocument>

bool Config::read(QDir rootDir, QString path)
{
    QFile srcFile(path);
    if (!srcFile.open(QIODevice::ReadOnly))
        return false;
    QJsonDocument json = QJsonDocument::fromJson(srcFile.readAll());
    rootObj = json.object();
    if (!readImpl(rootDir))
        return false;
    return true;
}

bool Config::write(QDir rootDir, QString path) const
{
    QFile dstFile(path);
    if (!dstFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    QJsonObject newRootObj = rootObj;
    if (!writeImpl(rootDir, newRootObj))
        return false;

    QTextStream stream(&dstFile);
    stream.setCodec("UTF-8");
    stream.setGenerateByteOrderMark(false);
    stream << QJsonDocument(newRootObj).toJson();
    return true;
}

QString Config::absPathOr(QDir rootDir, QString value, QString defaultValue)
{
    if (value.isEmpty())
        return defaultValue;
    return Files::absPath(rootDir, value);
}
