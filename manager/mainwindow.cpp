#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settings.h"
#include "librarysourcemanagerlist.h"
#include "appsourcemanagerlist.h"
#include "progressmanager.h"

namespace {
LibrarySourceManagerList* librarySourceManagers;
AppSourceManagerList* appSourceManagers;
ProgressManager* progressManager;
}

LibrarySourceManagerList* LibrarySourceManagerList::instance()
{
    return librarySourceManagers;
}

AppSourceManagerList* AppSourceManagerList::instance()
{
    return appSourceManagers;
}

ProgressManager* ProgressManager::instance()
{
    return progressManager;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    qRegisterMetaType<Library>("Library");
    qRegisterMetaType<App>("App");

    librarySourceManagers = new LibrarySourceManagerList(this);
    appSourceManagers = new AppSourceManagerList(this);
    connect(librarySourceManagers, SIGNAL(finishedUpdate(LibrarySource,QList<Library>)),
            this, SLOT(onLibrarySourceUpdateFinished(LibrarySource,QList<Library>)));
    connect(appSourceManagers, SIGNAL(finishedUpdate(AppSource,QList<App>)),
            this, SLOT(onAppSourceUpdateFinished(AppSource,QList<App>)));

    about = new AboutWindow(this);

    settings = new SettingsForm(this);
    ui->settingsLayout->addWidget(settings);

    libraries = new LibrariesForm(this);
    ui->librariesLayout->addWidget(libraries);

    apps = new AppsForm(this);
    ui->appsLayout->addWidget(apps);

    librarySourceManagers->set(Settings::instance().librarySources);
    librarySourceManagers->fastUpdate();
    appSourceManagers->set(Settings::instance().appSources);
    appSourceManagers->update();

    progressManager = new ProgressManager(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update(const Settings& curSettings)
{
    updateLibrarySources(curSettings);
    updateAppSources(curSettings);
}

void MainWindow::update()
{
    update(Settings::instance());
}

void MainWindow::updateLibrarySources()
{
    updateLibrarySources(Settings::instance());
}

void MainWindow::updateAppSources()
{
    updateAppSources(Settings::instance());
}

void MainWindow::on_aboutAction_triggered()
{
    about->show();
}

void MainWindow::onLibrarySourceUpdateFinished(LibrarySource source, const QList<Library>& libraries)
{
    settings->updateLibrarySource(source);
    this->libraries->append(libraries);
}

void MainWindow::onAppSourceUpdateFinished(AppSource source, const QList<App>& apps)
{
    settings->updateAppSource(source);
    this->apps->append(apps);
}

void MainWindow::updateLibrarySources(const Settings& curSettings)
{
    settings->setAllLibrarySourcesUnknown();
    libraries->clearLibrariesTable();
    librarySourceManagers->set(curSettings.librarySources);
    librarySourceManagers->update();
}

void MainWindow::updateAppSources(const Settings& curSettings)
{
    settings->setAllAppSourcesUnknown();
    apps->clearAppsTable();
    appSourceManagers->set(curSettings.appSources);
    appSourceManagers->update();
}
