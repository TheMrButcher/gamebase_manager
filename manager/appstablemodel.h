#ifndef APPSTABLEMODEL_H
#define APPSTABLEMODEL_H

#include "app.h"
#include <QAbstractTableModel>
#include <QSet>

class AppsTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    AppsTableModel(QObject* parent = 0);

    const QList<App>& get() const;
    void set(const QList<App>& apps);
    void append(const App& app);
    void replace(const App& appOld, const App& appNew);

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex& parent) const override;
    virtual int columnCount(const QModelIndex& parent) const override;
    virtual QVariant data(const QModelIndex& index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual bool removeRows(int row, int count, const QModelIndex& parent) override;
    virtual Qt::ItemFlags flags(const QModelIndex&) const override;

private:
    QList<App> apps;
    QSet<App> appsSet;
};

#endif // APPSTABLEMODEL_H
