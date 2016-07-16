#include "librariestablemodel.h"
#include <QIcon>

LibrariesTableModel::LibrariesTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{}

const QList<Library>& LibrariesTableModel::get() const
{
    return libraries;
}

void LibrariesTableModel::set(const QList<Library>& libraries)
{
    if (!this->libraries.empty())
        removeRows(0, this->libraries.size(), QModelIndex());
    if (libraries.empty())
        return;
    beginInsertRows(QModelIndex(), 0, libraries.size() - 1);
    this->libraries = libraries;
    endInsertRows();
}

void LibrariesTableModel::append(const Library& library)
{
    beginInsertRows(QModelIndex(), libraries.size(), libraries.size());
    libraries.append(library);
    endInsertRows();
}

int LibrariesTableModel::rowCount(const QModelIndex&) const
{
    return libraries.size();
}

int LibrariesTableModel::columnCount(const QModelIndex&) const
{
    return 5;
}

QVariant LibrariesTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= libraries.size())
        return QVariant();
    const auto& library = libraries[index.row()];
    if (role == Qt::DisplayRole) {
        if (index.column() == 1)
            return library.source.path;
        else if (index.column() == 2)
            return library.archiveName;
        else if (index.column() == 3)
            return library.version.toString();
    } else if (role == Qt::DecorationRole) {
        if (index.column() == 0){
            switch (library.source.type) {
            case LibrarySource::Server: return QIcon(":/images/icons/Web.png");
            case LibrarySource::Directory:
                if (library.isCurrent)
                    return QIcon(":/images/icons/Home.png");
                else
                    return QIcon(":/images/icons/Folder.png");
            default: return QVariant();
            }
        } else if (index.column() == 4) {
            switch (library.state) {
            case Library::Absent: return QIcon(":/images/icons/Error.png");
            case Library::SourceCode: return QIcon(":/images/icons/Code.png");
            case Library::BinaryArchive: return QIcon(":/images/icons/Archive.png");
            case Library::Deployed: return QIcon(":/images/icons/Ok.png");
            default: return QVariant();
            }
        }
    }
    return QVariant();
}

QVariant LibrariesTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return QString("Источник");
        case 1: return QString("Путь к источнику");
        case 2: return QString("Архив");
        case 3: return QString("Версия");
        case 4: return QString("Тип");
        default: return QVariant();
        }
    }
    return QVariant();
}

bool LibrariesTableModel::removeRows(int row, int count, const QModelIndex&)
{
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i)
        libraries.removeAt(row);
    endRemoveRows();
    return true;
}

Qt::ItemFlags LibrariesTableModel::flags(const QModelIndex&) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
