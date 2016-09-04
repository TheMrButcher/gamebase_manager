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
    librariesSet.clear();
    if (libraries.empty())
        return;
    QList<Library> uniqueLibraries;
    uniqueLibraries.reserve(libraries.size());
    foreach (const auto& library, libraries) {
        if (librariesSet.contains(library))
            continue;
        librariesSet.insert(library);
        if (library.source.type == LibrarySource::WorkingDirectory)
            uniqueLibraries.prepend(library);
        else
            uniqueLibraries.append(library);
    }

    beginInsertRows(QModelIndex(), 0, uniqueLibraries.size() - 1);
    this->libraries = uniqueLibraries;
    endInsertRows();
}

void LibrariesTableModel::append(const Library& library)
{
    if (librariesSet.contains(library))
        return;
    if (library.source.type == LibrarySource::WorkingDirectory) {
        replaceCurrentLibrary(library);
    } else {
        librariesSet.insert(library);
        beginInsertRows(QModelIndex(), libraries.size(), libraries.size());
        libraries.append(library);
        endInsertRows();
    }
}

void LibrariesTableModel::replaceCurrentLibrary(const Library& library)
{
    if (libraries.empty()
        || libraries[0].source.type != LibrarySource::WorkingDirectory) {
        librariesSet.insert(library);
        beginInsertRows(QModelIndex(), 0, 0);
        libraries.prepend(library);
        endInsertRows();
        return;
    }

    if (libraries[0] == library)
        return;
    librariesSet.remove(libraries[0]);
    librariesSet.insert(library);
    libraries[0] = library;
    emit dataChanged(index(0, 0), index(0, 4));
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
            case LibrarySource::Directory: return QIcon(":/images/icons/Folder.png");
            case LibrarySource::WorkingDirectory: return QIcon(":/images/icons/Home.png");
            case LibrarySource::DownloadsDirectory: return QIcon(":/images/icons/Download.png");
            default: return QVariant();
            }
        } else if (index.column() == 4) {
            switch (library.state) {
            case Library::Absent: return QIcon(":/images/icons/Empty.png");
            case Library::SourceCode: return QIcon(":/images/icons/Code.png");
            case Library::BinaryArchive: return QIcon(":/images/icons/Archive.png");
            case Library::Deployed: return QIcon(":/images/icons/Ok.png");
            default: return QVariant();
            }
        }
    } else if (role == Qt::ToolTipRole) {
        switch (index.column()) {
        case 0:
            switch (library.source.type) {
            case LibrarySource::Server: return "Сервер";
            case LibrarySource::Directory: return "Папка";
            case LibrarySource::WorkingDirectory: return "Рабочая папка";
            case LibrarySource::DownloadsDirectory: return "Папка для загрузок";
            default: return QVariant();
            }
        case 1: return library.source.path;
        case 2: return library.archiveName;
        case 3: return library.version.toString();
        case 4:
            switch (library.state) {
            case Library::Absent: return "Отсутствует";
            case Library::SourceCode: return "Требуется компиляция";
            case Library::BinaryArchive: return "Архив с построенным пакетом";
            case Library::Deployed: return "Установлен";
            default: return QVariant();
            }
        default: break;
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
    for (int i = 0; i < count; ++i) {
        librariesSet.remove(libraries[row]);
        libraries.removeAt(row);
    }
    endRemoveRows();
    return true;
}

Qt::ItemFlags LibrariesTableModel::flags(const QModelIndex&) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
