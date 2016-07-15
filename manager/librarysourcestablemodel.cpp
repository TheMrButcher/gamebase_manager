#include "librarysourcestablemodel.h"
#include <QIcon>

LibrarySourcesTableModel::LibrarySourcesTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{}

QList<LibrarySource> LibrarySourcesTableModel::get() const
{
    return sources;
}

void LibrarySourcesTableModel::set(const QList<LibrarySource>& sources)
{
    if (!this->sources.empty())
        removeRows(0, this->sources.size(), QModelIndex());
    if (sources.empty())
        return;
    beginInsertRows(QModelIndex(), 0, sources.size() - 1);
    this->sources = sources;
    endInsertRows();
}

void LibrarySourcesTableModel::append(const LibrarySource& source)
{
    beginInsertRows(QModelIndex(), sources.size(), sources.size());
    sources.append(source);
    endInsertRows();
}

int LibrarySourcesTableModel::rowCount(const QModelIndex&) const
{
    return sources.size();
}

int LibrarySourcesTableModel::columnCount(const QModelIndex&) const
{
    return 3;
}

QVariant LibrarySourcesTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= sources.size())
        return QVariant();
    if (role == Qt::DisplayRole) {
        if (index.column() == 1)
            return sources[index.row()].path;
        else
            return "";
    } else if (role == Qt::DecorationRole) {
        if (index.column() == 0) {
            switch (sources[index.row()].type) {
            case LibrarySource::Server: return QIcon(":/images/icons/Web.png");
            case LibrarySource::Directory: return QIcon(":/images/icons/Folder.png");
            default: return QVariant();
            }
        } else if (index.column() == 2){
            switch (sources[index.row()].status) {
            case SourceStatus::Unknown: return QIcon(":/images/icons/Help.png");
            case SourceStatus::OK: return QIcon(":/images/icons/Ok.png");
            case SourceStatus::Broken: return QIcon(":/images/icons/Error.png");
            default: return QVariant();
            }
        }
    } else if (role == Qt::EditRole) {
        if (index.column() == 1) {
            return sources[index.row()].path;
        }
    }
    return QVariant();
}

bool LibrarySourcesTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.isValid() && role == Qt::EditRole && index.column() == 1) {
        auto& source = sources[index.row()];
        source.path = value.toString();
        source.status = SourceStatus::Unknown;
        emit dataChanged(index, this->index(index.row(), 2));
    }
    return false;
}

QVariant LibrarySourcesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return QString("Тип");
        case 1: return QString("Путь");
        case 2: return QString("Статус");
        default: return QVariant();
        }
    }
    return QVariant();
}

bool LibrarySourcesTableModel::removeRows(int row, int count, const QModelIndex&)
{
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i)
        sources.removeAt(row);
    endRemoveRows();
    return true;
}

Qt::ItemFlags LibrarySourcesTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid() || index.column() != 1)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


