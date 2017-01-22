#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutwindow.h"
#include "maintabform.h"
#include "settingsform.h"
#include "librariesform.h"
#include "firstusagedialog.h"
#include "appsform.h"
#include "settings.h"
#include "librarysourcemanagerlist.h"
#include "appsourcemanagerlist.h"
#include "progressmanager.h"
#include "githubupdatedownloader.h"
#include "selfupdater.h"
#include <QMessageBox>
#include <QTimer>

namespace {
LibrarySourceManagerList* librarySourceManagers;
AppSourceManagerList* appSourceManagers;
ProgressManager* progressManager;
GithubUpdateDownloader* selfUpdateDownloader = nullptr;
SelfUpdater* selfUpdater = nullptr;
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

GithubUpdateDownloader* GithubUpdateDownloader::instance()
{
    return selfUpdateDownloader;
}

SelfUpdater* SelfUpdater::instance()
{
    return selfUpdater;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    askAboutUpdates(true)
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

    if (Settings::instance().isFirstUsage) {
        FirstUsageDialog* dialog = new FirstUsageDialog(this);
        dialog->show();
    }

    selfUpdateDownloader = new GithubUpdateDownloader(Settings::instance().selfUpdateSourceUrl, this);
    selfUpdater = new SelfUpdater(this);
    connect(selfUpdateDownloader, SIGNAL(finishedUpdate(QList<UpdateDescriptor>)),
            selfUpdater, SLOT(onUpdatesListDownloaded(QList<UpdateDescriptor>)));
    connect(selfUpdateDownloader, SIGNAL(finishedDownload()),
            selfUpdater, SLOT(onDownloadFinished()));
    connect(selfUpdater, SIGNAL(hasUpdates(bool)), this, SLOT(gotUpdates(bool)));

    libraries = new LibrariesForm(this);
    ui->librariesLayout->addWidget(libraries);

    apps = new AppsForm(this);
    ui->appsLayout->addWidget(apps);

    progressManager = new ProgressManager(this);

    mainTab = new MainTabForm(this);
    ui->mainTabLayout->addWidget(mainTab);

    QTimer::singleShot(0, this, SLOT(updateAll()));
    QTimer::singleShot(3000, this, SLOT(startAdditionalBackgroundTasks()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateAll(const Settings& curSettings)
{
    mainTab->updateState();
    updateLibrarySources(curSettings);
    updateAppSources(curSettings);
}

SettingsForm*MainWindow::settingsForm() const
{
    return settings;
}

LibrariesForm* MainWindow::librariesForm() const
{
    return libraries;
}

AppsForm* MainWindow::appsForm() const
{
    return apps;
}

void MainWindow::updateAll()
{
    updateAll(Settings::instance());
}

void MainWindow::updateLibrarySources()
{
    updateLibrarySources(Settings::instance());
}

void MainWindow::updateAppSources()
{
    updateAppSources(Settings::instance());
}

void MainWindow::gotUpdates(bool any)
{
    if (!any)
        return;
    if (!askAboutUpdates)
        return;
    askAboutUpdates = false;
    auto answer = QMessageBox::question(this, "Установка обновления",
                                        "Доступно обновление до версии "
                                        + selfUpdater->targetVersion().toString()
                                        + ". Установить?");
    if (answer != QMessageBox::Yes)
        return;
    selfUpdater->updateApp();
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

void MainWindow::startAdditionalBackgroundTasks()
{
    selfUpdateDownloader->updateInfo();
    selfUpdater->checkDownloadsDir();
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
