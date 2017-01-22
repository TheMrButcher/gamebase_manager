#ifndef SELFUPDATER_H
#define SELFUPDATER_H

#include "updatedescriptor.h"
#include <QObject>

class SelfUpdater : public QObject
{
    Q_OBJECT
public:
    explicit SelfUpdater(QObject *parent = 0);

    void checkDownloadsDir();
    Version targetVersion();
    void updateApp();
    static SelfUpdater* instance();

signals:
    void hasUpdates(bool);

public slots:
    void onUpdatesListDownloaded(const QList<UpdateDescriptor>& availableUpdates);
    void onDownloadFinished();

private:
    QList<UpdateDescriptor> updates;
};

#endif // SELFUPDATER_H
