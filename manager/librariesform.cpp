#include "librariesform.h"
#include "ui_librariesform.h"
#include "settings.h"
#include "mainwindow.h"
#include "librarysourcemanagerlist.h"
#include <QDir>
#include <QMessageBox>
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

void LibrariesForm::onLibraryDownloaded(const Library& library)
{
    if (library.state == Library::Absent) {
        qDebug() << "Failed to download from " << library.source.path;
        return;
    }
    librariesModel->append(library);
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
    }
}

void LibrariesForm::on_downloadButton_clicked()
{
    auto rows = ui->librariesTable->selectionModel()->selectedRows();
    if (rows.empty())
        return;
    int row = rows[0].row();
    auto library = librariesModel->get()[row];
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
    LibrarySourceManagerList::instance()->download(library);
}

void LibrariesForm::on_removeButton_clicked()
{
    auto rows = ui->librariesTable->selectionModel()->selectedRows();
    if (rows.empty())
        return;
    int row = rows[0].row();
    auto library = librariesModel->get()[row];
    librariesModel->removeRow(row);
    library.remove();
}
