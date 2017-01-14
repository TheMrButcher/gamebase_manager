#ifndef EDITORCONFIG_H
#define EDITORCONFIG_H

#include "config.h"

class EditorConfig : public Config {
public:
    EditorConfig() : isVirtual(true) {}

    QString workingPath;
    QString designedObjectImagesPath;
    int width;
    int height;
    bool isWindow;
    bool isVirtual;

    static QDir editorDir();
    static QString editorConfigPath();
    static QString editorPath();
    static EditorConfig& instance();
    bool read();
    bool write();

private:
    virtual bool readImpl(QDir rootDir) override;
    virtual bool writeImpl(QDir rootDir, QJsonObject& newRootObj) const override;
};

#endif // EDITORCONFIG_H
