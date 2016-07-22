#ifndef APPSOURCEMANAGER_H
#define APPSOURCEMANAGER_H

#include "appsource.h"
#include "app.h"
#include <QObject>

class QWidget;

class AppSourceManager : public QObject
{
    Q_OBJECT

public:
    AppSourceManager(const AppSource& source, QWidget* parent);

    void update();
    void addToWorkingDir(App app);

signals:
    void finishedUpdate(AppSource source, const QList<App>& apps);
    void finishedAdd(App app);

private:
    AppSource source;
};

#endif // APPSOURCEMANAGER_H
