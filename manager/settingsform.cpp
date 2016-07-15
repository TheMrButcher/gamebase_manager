#include "settingsform.h"
#include "ui_settingsform.h"
#include "settings.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>

namespace {
const QString SETTINGS_FILE_NAME = "settings.json";
}

SettingsForm::SettingsForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsForm)
{
    ui->setupUi(this);

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
    ui->workingDir->setText(settings.workingDir);
    librarySourcesModel->set(settings.librarySources);
    appSourcesModel->set(settings.appSources);
}

Settings SettingsForm::get() const
{
    Settings result;
    result.workingDir = ui->workingDir->text();
    result.librarySources = librarySourcesModel->get();
    result.appSources = appSourcesModel->get();
    return result;
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

void SettingsForm::onAppSourcesSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
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
