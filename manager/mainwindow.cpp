#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include "librarysourcemanagerlist.h"

namespace {
LibrarySourceManagerList* librarySourceManagers;
}

LibrarySourceManagerList* LibrarySourceManagerList::instance()
{
    return librarySourceManagers;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qRegisterMetaType<Library>("Library");

    librarySourceManagers = new LibrarySourceManagerList(this);
    connect(librarySourceManagers, SIGNAL(finishedUpdate(LibrarySource,QList<Library>)),
            this, SLOT(onUpdateFinished(LibrarySource,QList<Library>)));

    about = new AboutWindow(this);

    settings = new SettingsForm(this);
    ui->settingsLayout->addWidget(settings);

    libraries = new LibrariesForm(this);
    ui->librariesLayout->addWidget(libraries);

    apps = new AppsForm(this);
    ui->appsLayout->addWidget(apps);

    librarySourceManagers->set(Settings::instance().librarySources);
    librarySourceManagers->fastUpdate();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateLibrarySources(const QList<LibrarySource>& sources)
{
    settings->setAllUnknown();
    libraries->clearLibrariesTable();
    librarySourceManagers->set(sources);
    librarySourceManagers->update();
}

void MainWindow::updateLibrarySources()
{
    updateLibrarySources(Settings::instance().librarySources);
}

void MainWindow::on_aboutAction_triggered()
{
    about->show();
}

void MainWindow::onUpdateFinished(LibrarySource source, const QList<Library>& libraries)
{
    settings->updateLibrarySource(source);
    this->libraries->append(libraries);
}
