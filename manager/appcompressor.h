#ifndef APPCOMPRESSOR_H
#define APPCOMPRESSOR_H

#include "app.h"
#include <QRunnable>
#include <QObject>

class FilesManager;

class AppCompressor : public QObject, public QRunnable
{
    Q_OBJECT

public:
    AppCompressor(const App& srcApp, const App& dstApp);

    virtual void run() override;

signals:
    void finishedCompress(App app);

private:
    App srcApp;
    App dstApp;
    FilesManager* manager;
};

#endif // APPCOMPRESSOR_H
