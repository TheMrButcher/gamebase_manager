#include "librariesform.h"
#include "ui_librariesform.h"
#include "settings.h"

LibrariesForm::LibrariesForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LibrariesForm)
{
    ui->setupUi(this);

    librariesModel = new LibrariesTableModel(this);
    ui->librariesTable->setModel(librariesModel);
    ui->librariesTable->setColumnWidth(0, 60);
    ui->librariesTable->setColumnWidth(1, 200);
    ui->librariesTable->setColumnWidth(2, 100);
    ui->librariesTable->setColumnWidth(3, 50);
    ui->librariesTable->setColumnWidth(4, 50);

    connect(ui->updateButton, SIGNAL(clicked()), this, SLOT(update()));
}

LibrariesForm::~LibrariesForm()
{
    delete ui;
}

void LibrariesForm::update()
{
    QList<Library> libraries;
    libraries.append(Library::fromDirectory(Settings::instance().workingDir));
    librariesModel->set(libraries);
    ui->librariesTable->selectionModel()->clearSelection();
}
