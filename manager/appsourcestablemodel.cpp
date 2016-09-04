#include "appsourcestablemodel.h"
#include <QIcon>

AppSourcesTableModel::AppSourcesTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{}

QList<AppSource> AppSourcesTableModel::get() const
{
    return sources;
}

void AppSourcesTableModel::set(const QList<AppSource>& sources)
{
    if (!this->sources.empty())
        removeRows(0, this->sources.size(), QModelIndex());
    if (sources.empty())
        return;
    beginInsertRows(QModelIndex(), 0, sources.size() - 1);
    this->sources = sources;
    endInsertRows();
}

void AppSourcesTableModel::append(const AppSource& source)
{
    beginInsertRows(QModelIndex(), sources.size(), sources.size());
    sources.append(source);
    endInsertRows();
}

void AppSourcesTableModel::update(const AppSource& source)
{
    for (int i = 0; i != sources.size(); ++i) {
        auto& dstSource = sources[i];
        if (dstSource.path == source.path) {
            dstSource.type = source.type;
            dstSource.status = source.status;
            emit dataChanged(index(i, 0), index(i, 2));
            return;
        }
    }
}

int AppSourcesTableModel::rowCount(const QModelIndex&) const
{
    return sources.size();
}

int AppSourcesTableModel::columnCount(const QModelIndex&) const
{
    return 2;
}

QVariant AppSourcesTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= sources.size())
        return QVariant();
    if (role == Qt::DisplayRole) {
        if (index.column() == 0)
            return sources[index.row()].path;
        else
            return "";
    } else if (role == Qt::DecorationRole) {
        if (index.column() == 1){
            switch (sources[index.row()].status) {
            case SourceStatus::Unknown: return QIcon(":/images/icons/Help.png");
            case SourceStatus::OK: return QIcon(":/images/icons/Ok.png");
            case SourceStatus::Broken: return QIcon(":/images/icons/Error.png");
            default: return QVariant();
            }
        }
    } else if (role == Qt::EditRole) {
        if (index.column() == 0) {
            return sources[index.row()].path;
        }
    } else if (role == Qt::ToolTipRole) {
        switch (index.column()) {
        case 0: return sources[index.row()].path;
        case 1:
            switch (sources[index.row()].status) {
            case SourceStatus::Unknown: return "Статус неизвестен";
            case SourceStatus::OK: return "Есть доступ";
            case SourceStatus::Broken: return "Нет доступа";
            default: return QVariant();
            }
        }
    }
    return QVariant();
}

bool AppSourcesTableModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.isValid() && role == Qt::EditRole && index.column() == 0) {
        auto& source = sources[index.row()];
        source.path = value.toString();
        source.status = SourceStatus::Unknown;
        emit dataChanged(index, this->index(index.row(), 1));
    }
    return false;
}

QVariant AppSourcesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return QString("Путь");
        case 1: return QString("Статус");
        default: return QVariant();
        }
    }
    return QVariant();
}

bool AppSourcesTableModel::removeRows(int row, int count, const QModelIndex&)
{
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i)
        sources.removeAt(row);
    endRemoveRows();
    return true;
}

Qt::ItemFlags AppSourcesTableModel::flags(const QModelIndex& index) const
{
    if (!index.isValid() || index.column() != 0)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
