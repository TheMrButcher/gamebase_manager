#ifndef LIBRARIESTABLEMODEL_H
#define LIBRARIESTABLEMODEL_H

#include "library.h"
#include <QAbstractTableModel>
#include <QSet>

class LibrariesTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    LibrariesTableModel(QObject* parent = 0);

    const QList<Library>& get() const;
    void set(const QList<Library>& libraries);
    void append(const Library& library);

    // QAbstractItemModel interface
public:
    virtual int rowCount(const QModelIndex& parent) const override;
    virtual int columnCount(const QModelIndex& parent) const override;
    virtual QVariant data(const QModelIndex& index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    virtual bool removeRows(int row, int count, const QModelIndex& parent) override;
    virtual Qt::ItemFlags flags(const QModelIndex&) const override;

private:
    QList<Library> libraries;
    QSet<Library> librariesSet;
};

#endif // LIBRARIESTABLEMODEL_H
