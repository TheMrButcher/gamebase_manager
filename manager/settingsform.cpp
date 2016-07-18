#include "settingsform.h"
#include "ui_settingsform.h"
#include "settings.h"
#include "mainwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>

namespace {
const QString SETTINGS_FILE_NAME = "settings.json";
}

SettingsForm::SettingsForm(MainWindow* parent) :
    QWidget(parent),
    ui(new Ui::SettingsForm)
{
    ui->setupUi(this);

    mainWindow = parent;

    librarySourcesModel = new LibrarySourcesTableModel(this);
    ui->gamebaseSources->setModel(librarySourcesModel);
    ui->gamebaseSources->setColumnWidth(0, 50);
    ui->gamebaseSources->setColumnWidth(1, 250);
    ui->gamebaseSources->setColumnWidth(2, 50);

    appSourcesModel = new AppSourcesTableModel(this);
    ui->appSources->setModel(appSourcesModel);
    ui->appSources->setColumnWidth(0, 300);
    ui->appSources->setColumnWidth(1, 50);

    if (!Settings::instance().read(SETTINGS_FILE_NAME)) {
        QMessageBox::warning(this, QString("Ошибка при чтении настроек"),
                             QString("Невозможно прочесть файл с настройками. Возможно, он отсутствует или испорчен"));
        Settings::instance() = Settings::defaultValue();
        Settings::instance().write(SETTINGS_FILE_NAME);
    }

    set(Settings::instance());

    connect(ui->gamebaseSources->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onGamebaseSourcesSelectionChanged(QItemSelection,QItemSelection)));
    connect(ui->appSources->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onAppSourcesSelectionChanged(QItemSelection,QItemSelection)));
}

SettingsForm::~SettingsForm()
{
    delete ui;
}

void SettingsForm::set(const Settings& settings)
{
    QList<LibrarySource> librarySources;
    librarySources.reserve(settings.librarySources.size());
    foreach (const auto& source, settings.librarySources) {
        switch (source.type) {
        case LibrarySource::WorkingDirectory: ui->workingDir->setText(source.path); break;
        case LibrarySource::DownloadsDirectory: ui->downloadsDir->setText(source.path); break;
        default: librarySources.append(source);
        }
    }
    librarySourcesModel->set(librarySources);
    appSourcesModel->set(settings.appSources);
}

Settings SettingsForm::get() const
{
    Settings result;
    result.librarySources = librarySourcesModel->get();
    result.librarySources.append(LibrarySource{
        LibrarySource::WorkingDirectory, ui->workingDir->text(), SourceStatus::Unknown });
    result.librarySources.append(LibrarySource{
        LibrarySource::DownloadsDirectory, ui->downloadsDir->text(), SourceStatus::Unknown });
    result.appSources = appSourcesModel->get();
    return result;
}

void SettingsForm::setAllUnknown()
{
    foreach (auto source, librarySourcesModel->get()) {
        source.status = SourceStatus::Unknown;
        updateLibrarySource(source);
    }
}

void SettingsForm::updateLibrarySource(const LibrarySource& source)
{
    librarySourcesModel->update(source);
}

void SettingsForm::on_acceptButton_clicked()
{
    Settings::instance() = get();
    Settings::instance().write(SETTINGS_FILE_NAME);
}

void SettingsForm::on_cancelButton_clicked()
{
    set(Settings::instance());
}

void SettingsForm::onGamebaseSourcesSelectionChanged(
        const QItemSelection& selected, const QItemSelection&)
{
    ui->removeGamebasePathButton->setEnabled(!selected.empty());
}

void SettingsForm::on_removeGamebasePathButton_clicked()
{
    auto selectedRows = ui->gamebaseSources->selectionModel()->selectedRows();
    if (selectedRows.empty())
        return;
    int row = selectedRows[0].row();
    librarySourcesModel->removeRow(row);
    ui->gamebaseSources->selectionModel()->clearSelection();
}

void SettingsForm::on_addGamebaseServerButton_clicked()
{
    bool ok;
    QString path = QInputDialog::getText(this, "Ввод адреса сервера",
                                         "Адрес сервера:", QLineEdit::Normal,
                                         "", &ok);
    if (ok && !path.isEmpty()) {
        librarySourcesModel->append(
                    LibrarySource{ LibrarySource::Server, path, SourceStatus::Unknown });
    }
}

void SettingsForm::on_addGamebaseDirectoryButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Ввод пути к папке-источнику");
    if (!path.isEmpty()) {
        librarySourcesModel->append(
                    LibrarySource{ LibrarySource::Directory, path, SourceStatus::OK });
    }
}

void SettingsForm::onAppSourcesSelectionChanged(const QItemSelection& selected, const QItemSelection&)
{
    ui->removeAppPathButton->setEnabled(!selected.empty());
}

void SettingsForm::on_addAppDirectoryButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Ввод пути к папке-источнику");
    if (!path.isEmpty()) {
        appSourcesModel->append(
                    AppSource{ path, SourceStatus::OK });
    }
}

void SettingsForm::on_removeAppPathButton_clicked()
{
    auto selectedRows = ui->appSources->selectionModel()->selectedRows();
    if (selectedRows.empty())
        return;
    int row = selectedRows[0].row();
    appSourcesModel->removeRow(row);
    ui->appSources->selectionModel()->clearSelection();
}

void SettingsForm::on_chooseWorkingDir_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Ввод пути к рабочей папке");
    if (!path.isEmpty()) {
        ui->workingDir->setText(path);
    }
}

void SettingsForm::on_updateGamebaseSourcesButton_clicked()
{
    mainWindow->updateLibrarySources(get().librarySources);
}

void SettingsForm::on_chooseDownloadsDir_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Ввод пути к папке для загрузок");
    if (!path.isEmpty()) {
        ui->downloadsDir->setText(path);
    }
}
