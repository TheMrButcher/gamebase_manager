#ifndef APPDEPLOYER_H
#define APPDEPLOYER_H

#include "app.h"
#include <QRunnable>
#include <QObject>

class FilesManager;
class Compiler;

class AppDeployer : public QObject, public QRunnable
{
    Q_OBJECT

public:
    AppDeployer(const App& app, QString dstPath);

    virtual void run() override;

signals:
    void finishedDeploy(App app, QString path, bool success);

private:
    bool tryDeploy();
    bool formCopyTask();

    App app;
    QString dstPath;
    FilesManager* manager;
    Compiler* compiler;
};

#endif // APPDEPLOYER_H
