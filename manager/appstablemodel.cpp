#include "appstablemodel.h"
#include <QIcon>

AppsTableModel::AppsTableModel(QObject* parent)
    : QAbstractTableModel(parent)
{}

const QList<App>& AppsTableModel::get() const
{
    return apps;
}

void AppsTableModel::set(const QList<App>& apps)
{
    if (!this->apps.empty())
        removeRows(0, this->apps.size(), QModelIndex());
    appsSet.clear();
    if (apps.empty())
        return;
    QList<App> uniqueApps;
    uniqueApps.reserve(apps.size());
    foreach (const auto& app, apps) {
        if (appsSet.contains(app))
            continue;
        appsSet.insert(app);
        if (app.source.type == AppSource::WorkingDirectory)
            uniqueApps.prepend(app);
        else
            uniqueApps.append(app);
    }

    beginInsertRows(QModelIndex(), 0, uniqueApps.size() - 1);
    this->apps = uniqueApps;
    endInsertRows();
}

void AppsTableModel::append(const App& app)
{
    if (appsSet.contains(app))
        return;
    appsSet.insert(app);
    if (app.source.type == AppSource::WorkingDirectory) {
        beginInsertRows(QModelIndex(), 0, 0);
        apps.prepend(app);
        endInsertRows();
    } else {
        beginInsertRows(QModelIndex(), apps.size(), apps.size());
        apps.append(app);
        endInsertRows();
    }
}

void AppsTableModel::replace(const App& oldApp, const App& newApp)
{
    int i = 0;
    foreach (const auto& appInTable, apps) {
        if (appInTable == oldApp) {
            apps[i] = newApp;
            emit dataChanged(index(i, 0), index(i, 4));
        }
        ++i;
    }
}

void AppsTableModel::replace(const AppSource& source, QString containerName, const App& newApp)
{
    int i = 0;
    foreach (const auto& appInTable, apps) {
        if (appInTable.source == source && appInTable.containerName == containerName) {
            apps[i] = newApp;
            emit dataChanged(index(i, 0), index(i, 4));
        }
        ++i;
    }
}

int AppsTableModel::rowCount(const QModelIndex&) const
{
    return apps.size();
}

int AppsTableModel::columnCount(const QModelIndex&) const
{
    return 5;
}

QVariant AppsTableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= apps.size())
        return QVariant();
    const auto& app = apps[index.row()];
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case 0: return app.source.path;
        case 1: return app.name;
        case 2: return app.containerName;
        case 3: return app.version.toString();
        default: break;
        }
    } else if (role == Qt::DecorationRole) {
        if (index.column() == 4) {
            switch (app.state) {
            case App::Archived: return QIcon(":/images/icons/Archive.png");
            case App::NotConfigured: return QIcon(":/images/icons/NotConfigured.png");
            case App::Full: return QIcon(":/images/icons/Ok.png");
            default: return QVariant();
            }
        }
    }
    return QVariant();
}

QVariant AppsTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return QString("Источник");
        case 1: return QString("Название");
        case 2: return QString("Папка/архив");
        case 3: return QString("Версия");
        case 4: return QString("Тип");
        default: return QVariant();
        }
    }
    return QVariant();
}

bool AppsTableModel::removeRows(int row, int count, const QModelIndex&)
{
    beginRemoveRows(QModelIndex(), row, row + count - 1);
    for (int i = 0; i < count; ++i) {
        appsSet.remove(apps[row]);
        apps.removeAt(row);
    }
    endRemoveRows();
    return true;
}

Qt::ItemFlags AppsTableModel::flags(const QModelIndex&) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}
