#ifndef APPSOURCEMANAGERLIST_H
#define APPSOURCEMANAGERLIST_H

#include "appsourcemanager.h"
#include <QObject>
#include <QHash>

class QWidget;

class AppSourceManagerList : public QObject
{
    Q_OBJECT
public:
    explicit AppSourceManagerList(QWidget* parent);

    static AppSourceManagerList* instance();

    void set(const QList<AppSource>& sources);
    void update();

signals:
    void finishedUpdate(AppSource source, const QList<App>& apps);

private:
    void insert(const AppSource& source);

    QHash<AppSource, AppSourceManager*> managers;
    QWidget* parent;
};

#endif // APPSOURCEMANAGERLIST_H
