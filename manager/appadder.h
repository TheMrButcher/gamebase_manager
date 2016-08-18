#ifndef APPADDER_H
#define APPADDER_H

#include "app.h"
#include <QRunnable>
#include <QObject>

class FilesManager;

class AppAdder : public QObject, public QRunnable
{
    Q_OBJECT

public:
    AppAdder(const App& app);

    virtual void run() override;

signals:
    void finishedAdd(App app);

private:
    void formTask(App resultApp);

    App app;
    FilesManager* manager;
};

#endif // APPADDER_H
