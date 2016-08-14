#include "librariesform.h"
#include "ui_librariesform.h"
#include "settings.h"
#include "mainwindow.h"
#include "librarysourcemanagerlist.h"
#include "librarydeployer.h"
#include "libraryremover.h"
#include <QDir>
#include <QMessageBox>
#include <QThreadPool>
#include <QDebug>

LibrariesForm::LibrariesForm(MainWindow *parent) :
    QWidget(parent),
    ui(new Ui::LibrariesForm)
{
    ui->setupUi(this);

    hasActiveDownload = false;

    librariesModel = new LibrariesTableModel(this);
    ui->librariesTable->setModel(librariesModel);
    ui->librariesTable->setColumnWidth(0, 60);
    ui->librariesTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->librariesTable->setColumnWidth(2, 150);
    ui->librariesTable->setColumnWidth(3, 50);
    ui->librariesTable->setColumnWidth(4, 50);

    clearLibrariesTable();

    connect(ui->updateButton, SIGNAL(clicked()), parent, SLOT(updateLibrarySources()));
    connect(ui->librariesTable->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onLibrariesSelectionChanged(QItemSelection,QItemSelection)));
    connect(LibrarySourceManagerList::instance(), SIGNAL(finishedDownload(Library)),
            this, SLOT(onLibraryDownloaded(Library)));
}

LibrariesForm::~LibrariesForm()
{
    delete ui;
}

void LibrariesForm::clearLibrariesTable()
{
    librariesModel->set(QList<Library>());
    ui->librariesTable->selectionModel()->clearSelection();
}

void LibrariesForm::append(const QList<Library>& libraries)
{
    foreach (const auto& library, libraries)
        librariesModel->append(library);
}

void LibrariesForm::download(Library library)
{
    auto downloadsDir = Settings::instance().downloadsDir();
    if (downloadsDir.check() != SourceStatus::OK) {
        auto answer = QMessageBox::question(this, "Создание папки для загрузок",
                                            "Папка для загрузок отсутствует. Создать?");
        if (answer != QMessageBox::Yes)
            return;
        QDir dir;
        dir.mkpath(downloadsDir.path);
        if (downloadsDir.check() != SourceStatus::OK)
            return;
    }
    auto resultLibrary = library.afterAction(Library::Download);
    if (resultLibrary.validate()) {
        onLibraryDownloaded(resultLibrary);
        return;
    }
    hasActiveDownload = true;
    updateButtons();
    LibrarySourceManagerList::instance()->download(library);
}

void LibrariesForm::install(Library library)
{
    toInstall = Library::makeAbsent();
    if (!library.checkAbility(Library::Install))
        return;
    if (library.state == Library::SourceCode) {
        if (Settings::instance().vcVarsPath.isEmpty()) {
            QMessageBox::warning(this, "Не указан путь к vcvarsall.bat",
                                 "Для компиляции кода необходим Microsoft Visual Studio 2010. "
                                 "Пожалуйста, укажите путь к файлу vcvarsall.bat.");
            return;
        }
    }
    if (library.checkAbility(Library::Deploy)) {
        auto resultLibrary = library.afterAction(Library::Deploy);
        if (resultLibrary.source.check() != SourceStatus::OK) {
            auto answer = QMessageBox::question(this, "Создание рабочей папки",
                                                "Рабочая папка отсутствует. Создать?");
            if (answer != QMessageBox::Yes)
                return;
            QDir dir;
            dir.mkpath(resultLibrary.source.path);
            if (resultLibrary.source.check() != SourceStatus::OK)
                return;
        }

        if (LibraryRemover::checkHasDeployedFiles(resultLibrary.source.path)) {
            waitedInstallAction = Library::Remove;
            toInstall = library;
            librariesModel->replaceCurrentLibrary(
                        Library::makeAbsent(Settings::instance().workingDir()));
            remove(resultLibrary);
            return;
        }
        auto deployer = new LibraryDeployer(library);
        connect(deployer, SIGNAL(finishedDeploy(Library)),
                this, SLOT(onLibraryDeployed(Library)));
        QThreadPool::globalInstance()->start(deployer);
        return;
    }
    if (library.checkAbility(Library::Download)) {
        waitedInstallAction = Library::Download;
        toInstall = library.afterAction(Library::Download);
        download(library);
    }
}

void LibrariesForm::onLibraryDeployed(Library library)
{
    if (!library.validate())
        qDebug() << "Failed to deploy library";
    librariesModel->append(library);
}

void LibrariesForm::onLibraryRemoved(Library library)
{
    if (toInstall.exists()
        && library.source.type == LibrarySource::WorkingDirectory
        && waitedInstallAction == Library::Remove) {
        if (!LibraryRemover::checkHasDeployedFiles(Settings::instance().workingDir().path))
            install(toInstall);
    }
}

void LibrariesForm::onLibraryDownloaded(Library library)
{
    hasActiveDownload = false;
    bool waitedToInstall =
            toInstall == library && waitedInstallAction == Library::Download;
    if (waitedToInstall)
        toInstall = Library::makeAbsent();
    if (!library.validate()) {
        qDebug() << "Failed to download library";
        return;
    }
    librariesModel->append(library);
    if (waitedToInstall)
        install(library);
    updateButtons();
}

void LibrariesForm::onLibrariesSelectionChanged(const QItemSelection&, const QItemSelection&)
{
    updateButtons();
}

void LibrariesForm::on_downloadButton_clicked()
{
    int row = selectedRow();
    if (row == -1)
        return;
    download(librariesModel->get()[row]);
}

void LibrariesForm::on_removeButton_clicked()
{
    int row = selectedRow();
    if (row == -1)
        return;
    auto library = librariesModel->get()[row];

    auto answer = QMessageBox::question(this, "Удаление библиотеки",
                                        "Вы уверены, что хотите удалить выбранную версию библиотеки?");
    if (answer != QMessageBox::Yes)
        return;

    if (row == 0)
        librariesModel->replaceCurrentLibrary(
                    Library::makeAbsent(Settings::instance().workingDir()));
    else
        librariesModel->removeRow(row);
    remove(library);
}

void LibrariesForm::on_installButton_clicked()
{
    int row = selectedRow();
    if (row == -1)
        return;
    install(librariesModel->get()[row]);
}

int LibrariesForm::selectedRow() const
{
    auto rows = ui->librariesTable->selectionModel()->selectedRows();
    if (rows.empty())
        return -1;
    return rows[0].row();
}

void LibrariesForm::remove(Library library)
{
    auto remover = new LibraryRemover(library);
    connect(remover, SIGNAL(finishedRemove(Library)),
            this, SLOT(onLibraryRemoved(Library)));
    QThreadPool::globalInstance()->start(remover);
}

void LibrariesForm::updateButtons()
{
    auto rows = ui->librariesTable->selectionModel()->selectedRows();
    if (rows.isEmpty()) {
        ui->downloadButton->setEnabled(false);
        ui->removeButton->setEnabled(false);
        ui->installButton->setEnabled(false);
    } else {
        int row = rows[0].row();
        const auto& library = librariesModel->get()[row];
        ui->downloadButton->setEnabled(library.checkAbility(Library::Download) && !hasActiveDownload);
        ui->removeButton->setEnabled(library.checkAbility(Library::Remove));
        ui->installButton->setEnabled(library.checkAbility(Library::Install) && !hasActiveDownload);
    }
}
