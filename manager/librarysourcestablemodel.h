#ifndef LIBRARYSOURCES_H
#define LIBRARYSOURCES_H

#include "librarysource.h"
#include <QAbstractTableModel>

class LibrarySourcesTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    LibrarySourcesTableModel(QObject* parent = 0);

    QList<LibrarySource> get() const;
    void set(const QList<LibrarySource>& sources);
    void append(const LibrarySource& source);
    void update(const LibrarySource& source);

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
    QList<LibrarySource> sources;
};

#endif // LIBRARYSOURCES_H
