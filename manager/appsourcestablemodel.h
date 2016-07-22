#ifndef APPSOURCESTABLEMODEL_H
#define APPSOURCESTABLEMODEL_H

#include "appsource.h"
#include <QAbstractTableModel>

class AppSourcesTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    AppSourcesTableModel(QObject* parent = 0);

    QList<AppSource> get() const;
    void set(const QList<AppSource>& sources);
    void append(const AppSource& source);
    void update(const AppSource& source);

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex& parent) const override;
    virtual int columnCount(const QModelIndex& parent) const override;
    virtual QVariant data(const QModelIndex& index, int role) const override;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual bool removeRows(int row, int count, const QModelIndex& parent) override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
    QList<AppSource> sources;
};

#endif // APPSOURCESTABLEMODEL_H
