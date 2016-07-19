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
    LibrarySourceManagerList::instance()->download(library);
}

void LibrariesForm::install(Library library)
{
    toInstall = Library::makeAbsent();
    if (!library.checkAbility(Library::Install))
        return;
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
            auto remover = new LibraryRemover(resultLibrary, this);
            QThreadPool::globalInstance()->start(remover);
            return;
        }
        auto deployer = new LibraryDeployer(library, this);
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
    bool waitedToInstall =
            toInstall == library && waitedInstallAction == Library::Download;
    if (waitedToInstall)
        toInstall = Library::makeAbsent();
    if (!library.validate()) {
        qDebug() << "Failed to download from " << library.source.path;
        return;
    }
    librariesModel->append(library);
    if (waitedToInstall)
        install(library);
}

void LibrariesForm::onLibrariesSelectionChanged(const QItemSelection& selected, const QItemSelection&)
{
    if (selected.empty()) {
        ui->downloadButton->setEnabled(false);
    } else {
        int row = selected[0].bottom();
        const auto& library = librariesModel->get()[row];
        ui->downloadButton->setEnabled(library.checkAbility(Library::Download));
        ui->removeButton->setEnabled(library.checkAbility(Library::Remove));
        ui->installButton->setEnabled(library.checkAbility(Library::Install));
    }
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
    if (row == 0)
        librariesModel->replaceCurrentLibrary(
                    Library::makeAbsent(Settings::instance().workingDir()));
    else
        librariesModel->removeRow(row);
    auto remover = new LibraryRemover(library, this);
    QThreadPool::globalInstance()->start(remover);
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
