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

signals:
    void finishedUpdate(AppSource source, const QList<App>& apps);

private:
    AppSource source;
};

#endif // APPSOURCEMANAGER_H
