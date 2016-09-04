#include "maintabform.h"
#include "ui_maintabform.h"
#include "mainwindow.h"
#include "librariesform.h"
#include "settings.h"
#include "settingsform.h"
#include "appsform.h"
#include "librarysourcemanagerlist.h"
#include "appsourcemanagerlist.h"
#include <QMessageBox>
#include <QPushButton>
#include <QLabel>

MainTabForm::MainTabForm(MainWindow* parent)
    : QWidget(parent)
    , ui(new Ui::MainTabForm)
    , parent(parent)
{
    ui->setupUi(this);

    connect(parent->librariesForm(), SIGNAL(finishedInstall(Library,bool)),
            this, SLOT(onLibraryDeployed(Library,bool)));
    connect(parent->librariesForm(), SIGNAL(finishedRemove(Library)),
            this, SLOT(onLibraryRemoved(Library)));
    connect(LibrarySourceManagerList::instance(), SIGNAL(finishedUpdate(LibrarySource,QList<Library>)),
            this, SLOT(onLibrarySourceUpdate(LibrarySource,QList<Library>)));

    connect(AppSourceManagerList::instance(), SIGNAL(finishedUpdate(AppSource,QList<App>)),
            this, SLOT(onAppSourceUpdate(AppSource,QList<App>)));
    connect(parent->appsForm(), SIGNAL(addedApp(App,bool)), this, SLOT(onAppAdded(App,bool)));
    connect(parent->appsForm(), SIGNAL(removedApp(App)), this, SLOT(onAppRemoved(App)));

    action = Idle;
    curRow = 0;
}

MainTabForm::~MainTabForm()
{
    delete ui;
}

void MainTabForm::updateState()
{
    curLib = Library::fromFileSystem(Settings::instance().workingDir());
    neededLib = Library::makeAbsent();
    apps.clear();
    action = Idle;

    updateView();
}

void MainTabForm::updateView()
{
    for (int i = 0; i <= curRow; ++i)
        ui->stateLayout->setRowStretch(i, 0);
    qDeleteAll(stateWidgets);
    stateWidgets.clear();
    curRow = 0;

    updateDirsState();
    updateLibraryState();
    updateAppsState();
    ui->stateLayout->setColumnStretch(1, 1);

    QWidget* spacer = new QWidget();
    ui->stateLayout->addWidget(spacer, curRow, 0, 3, 1);
    ui->stateLayout->setRowStretch(curRow, 1);
    stateWidgets.append(spacer);
}

void MainTabForm::updateDirsState()
{
    const auto& settings = Settings::instance();
    if (settings.workingDir().check() != SourceStatus::OK) {
        QPushButton* button = addRow(curRow++, Error,
                "Рабочая папка отсутствует " + settings.workingDir().path + ".", "Создать");
        connect(button, SIGNAL(clicked()), this, SLOT(createWorkingDir()));
    }
    if (settings.downloadsDir().check() != SourceStatus::OK) {
        QPushButton* button = addRow(curRow++, Error,
                "Папка для загрузок отсутствует " + settings.downloadsDir().path + ".", "Создать");
        connect(button, SIGNAL(clicked()), this, SLOT(createDownloadsDir()));
    }
    if (!QDir(settings.outputPath).exists()) {
        QPushButton* button = addRow(curRow++, Error,
                "Папка для построенных приложений отсутствует " + settings.outputPath + ".", "Создать");
        connect(button, SIGNAL(clicked()), this, SLOT(createOutputDir()));
    }
    if (settings.vcVarsPath.isEmpty() || !QFile(settings.vcVarsPath).exists()) {
        QPushButton* button = addRow(curRow++, Error,
                "Microsoft Visual Studio не обнаружен.", "Искать");
        connect(button, SIGNAL(clicked()), this, SLOT(detectMSVC()));
    }
}

void MainTabForm::updateLibraryState()
{
    if (!curLib.exists() && !neededLib.exists()) {
        QPushButton* button = addRow(curRow++, Error,
                "Рабочая библиотека \"Gamebase\" отсутствует. Источники библиотеки недоступны.", "Установить");
        button->setEnabled(false);
    } else if (!curLib.exists()) {
        QPushButton* button = addRow(curRow++, Error,
                "Рабочая библиотека \"Gamebase\" отсутствует.", "Установить");
        connect(button, SIGNAL(clicked()), this, SLOT(deployLibrary()));
    } else if (!neededLib.exists()) {
        QString label = "Установлена версия рабочей библиотеки \"Gamebase\" ("
                + curLib.version.toString()
                + "). Нет доступа к источникам библиотеки, поэтому неизвестно, является ли "
                  "данная версия последней.";
        QPushButton* button = addRow(curRow++, Unknown, label, "Обновить");
        button->setEnabled(false);
    } else if (curLib.version < neededLib.version) {
        QString label = "Текушая версия рабочей библиотеки \"Gamebase\" (" + curLib.version.toString()
                + ") устарела. Имеется обновление до версии " + neededLib.version.toString() + ".";
        QPushButton* button = addRow(curRow++, Warning, label, "Обновить");
        connect(button, SIGNAL(clicked()), this, SLOT(deployLibrary()));
    } else {
        QString label = "Установлена последняя версия рабочей библиотеки \"Gamebase\" ("
                + curLib.version.toString() + ").";
        QPushButton* button = addRow(curRow++, OK, label, "Обновить");
        button->setEnabled(false);
    }
}

void MainTabForm::updateAppsState()
{
    int workingApps = 0;
    int appsToInstall = 0;
    int appsToUpdate = 0;
    foreach (const auto& app, apps) {
        if (app.cur.exists())
            workingApps++;
        if (app.latest.exists()) {
            if (app.cur.exists()) {
                appsToUpdate++;
            } else {
                appsToInstall++;
            }
        }
    }

    if (workingApps == 0 && appsToInstall == 0 && appsToUpdate == 0) {
        QPushButton* button = addRow(curRow++, Warning,
                "Не найдено приложений в рабочей папке и источниках приложений."
                "Создайте новое приложение во вкладке \"Приложения\" или укажите "
                "новый истоник приложений в настройках.", "Обновить");
        button->setEnabled(false);
    } else if (workingApps == 0 && appsToUpdate == 0) {
        QPushButton* button = addRow(curRow++, Warning,
                "Не найдено приложений в рабочей папке.", "Обновить");
        button->setEnabled(false);
    } else if (appsToUpdate == 0) {
        QPushButton* button = addRow(curRow++, OK,
                "Найдено " + QString::number(workingApps) + " приложений в рабочей папке. "
                "Обновление приложений в рабочей папке не требуется.", "Обновить");
        button->setEnabled(false);
    } else {
        QPushButton* button = addRow(curRow++, Warning,
                "Найдено " + QString::number(workingApps) + " приложений в рабочей папке. "
                "Можно обновить " + QString::number(appsToUpdate) + " приложений ("
                + appsList(true, true) + ").", "Обновить");
        connect(button, SIGNAL(clicked()), this, SLOT(updateApps()));
    }

    if (appsToInstall != 0) {
        QPushButton* button = addRow(curRow++, Warning,
                "Можно установить " + QString::number(appsToInstall) + " приложений ("
                + appsList(false, true) + ").", "Установить");
        connect(button, SIGNAL(clicked()), this, SLOT(installApps()));
    }
}

QString MainTabForm::appsList(bool curExists, bool latestExists)
{
    static const int TRIM_NUM = 4;
    QString appsList;
    int i = 0;
    for (auto it = apps.begin(); it != apps.end(); ++it) {
        const auto& app = it.value();
        if (app.cur.exists() == curExists && app.latest.exists() == latestExists) {
            ++i;
            if (i == TRIM_NUM) {
                appsList += " и др.";
                break;
            }
            if (i > 1)
                appsList += ", ";
            appsList += it.key();
        }
    }
    return appsList;
}

QString MainTabForm::nextAppToProcess()
{
    for (auto it = apps.begin(); it != apps.end(); ++it) {
        const AppInfo& app = it.value();
        bool needUpdate = app.cur.exists() && app.latest.exists();
        bool needInstall = !app.cur.exists() && app.latest.exists();
        if ((needUpdate && (action == ProcessApps || action == OnlyUpdateApps))
             || (needInstall && (action == ProcessApps || action == OnlyInstallApps)))
            return it.key();
    }
    return QString();
}

void MainTabForm::deployApp(QString appName)
{
    if (!apps.contains(appName)) {
        action = Idle;
        return;
    }
    AppInfo& app = apps[appName];
    if (!app.latest.exists()) {
        action = Idle;
        return;
    }
    parent->appsForm()->addApp(app.latest);
}

void MainTabForm::deployApps()
{
    QString appName = nextAppToProcess();
    if (appName.isEmpty()) {
        action = Idle;
        return;
    }
    deployApp(appName);
}

QPushButton* MainTabForm::addRow(int row, MainTabForm::FeatureStatus status, QString label, QString buttonLabel)
{
    QLabel* statusLabel = new QLabel();
    QPixmap image;
    QString toolTipLabel;
    switch (status) {
    case Unknown:
        image = QPixmap(":/images/icons/Help.png");
        toolTipLabel = "Неопределенное состояние";
        break;
    case Error:
        image = QPixmap(":/images/icons/Error.png");
        toolTipLabel = "Имеется проблема";
        break;
    case Warning:
        image = QPixmap(":/images/icons/Warning.png");
        toolTipLabel = "Несущественная проблема";
        break;
    case OK:
        image = QPixmap(":/images/icons/Ok.png");
        toolTipLabel = "Нормальное состояние";
        break;
    default:
        image = QPixmap(":/images/icons/Help.png");
        toolTipLabel = "Неизвестное состояние";
        break;
    }
    statusLabel->setPixmap(image);
    statusLabel->setToolTip(toolTipLabel);
    ui->stateLayout->addWidget(statusLabel, row, 0);
    stateWidgets.append(statusLabel);

    QLabel* textLabel = new QLabel(label);
    textLabel->setWordWrap(true);
    ui->stateLayout->addWidget(textLabel, row, 1);
    stateWidgets.append(textLabel);

    QPushButton* button = new QPushButton(buttonLabel);
    ui->stateLayout->addWidget(button, row, 2);
    stateWidgets.append(button);
    return button;
}

void MainTabForm::processNextAction()
{
    const auto& settings = Settings::instance();
    switch (action) {
    case Idle:
        action = CreateWorkingDir;
        if (settings.workingDir().check() != SourceStatus::OK) {
            createWorkingDir();
            return;
        }
        break;
    case CreateWorkingDir:
        action = CreateDownloadsDir;
        if (settings.downloadsDir().check() != SourceStatus::OK) {
            createDownloadsDir();
            return;
        }
        break;
    case CreateDownloadsDir:
        action = CreateOutputDir;
        if (!QDir(settings.outputPath).exists()) {
            createOutputDir();
            return;
        }
        break;
    case CreateOutputDir:
        action = DetectMSVC;
        if (settings.vcVarsPath.isEmpty() || !QFile(settings.vcVarsPath).exists()) {
            detectMSVC();
            return;
        }
        break;
    case DetectMSVC:
        action = InstallLibrary;
        if (!curLib.exists() || neededLib.version != curLib.version) {
            deployLibrary();
            return;
        }
        break;
    case InstallLibrary:
    case ProcessApps:
    {
        action = ProcessApps;
        QString appName = nextAppToProcess();
        if (appName.isEmpty()) {
            action = Idle;
            return;
        }
        deployApp(appName);
        return;
    }

    default: action = Idle; return;
    }

    processNextAction();
}

void MainTabForm::createWorkingDir()
{
    if (!QDir().mkpath(Settings::instance().workingDir().path)) {
        QMessageBox::warning(this, "Ошибка при установке",
                             "Не удалось создать рабочую папку.");
        action = Idle;
        return;
    }
    updateView();
    if (action == CreateWorkingDir)
        processNextAction();
}

void MainTabForm::createDownloadsDir()
{
    if (!QDir().mkpath(Settings::instance().downloadsDir().path)) {
        QMessageBox::warning(this, "Ошибка при установке",
                             "Не удалось создать папку для загрузок.");
        action = Idle;
        return;
    }
    updateView();
    if (action == CreateDownloadsDir)
        processNextAction();
}

void MainTabForm::createOutputDir()
{
    if (!QDir().mkpath(Settings::instance().outputPath)) {
        QMessageBox::warning(this, "Ошибка при установке",
                             "Не удалось создать папку для построенных приложений.");
        action = Idle;
        return;
    }
    updateView();
    if (action == CreateOutputDir)
        processNextAction();
}

void MainTabForm::detectMSVC()
{
    auto path = Settings::extractVCVarsPath();
    if (path.isEmpty()) {
        QMessageBox::warning(this, "Ошибка при установке",
                             "Не удалось обнаружить Microsoft Visual Studio "
                             "необходимой версии на компьютере.");
        action = Idle;
        return;
    }
    Settings::instance().vcVarsPath = path;
    parent->settingsForm()->set(Settings::instance());
    updateView();
    if (action == DetectMSVC)
        processNextAction();
}

void MainTabForm::deployLibrary()
{
    parent->librariesForm()->install(neededLib);
}

void MainTabForm::installApps()
{
    action = OnlyInstallApps;
    deployApps();
}

void MainTabForm::updateApps()
{
    action = OnlyUpdateApps;
    deployApps();
}

void MainTabForm::onLibrarySourceUpdate(LibrarySource source, const QList<Library>& libraries)
{
    if (source == Settings::instance().workingDir())
        return;
    bool needUpdate = false;
    foreach (auto library, libraries) {
        if (!neededLib.exists() || neededLib.version < library.version) {
            neededLib = library;
            needUpdate = true;
        }
        if (neededLib.version == library.version) {
            if (neededLib.source.type == LibrarySource::Server
                && library.source.type != LibrarySource::Server)
                neededLib = library;
            if (neededLib.source.type == LibrarySource::Directory
                && library.source.type == LibrarySource::DownloadsDirectory)
                neededLib = library;
            if (neededLib.state == Library::SourceCode && library.state == Library::BinaryArchive)
                neededLib = library;
        }
    }
    if (needUpdate)
        updateView();
}

void MainTabForm::onAppAdded(App app, bool success)
{
    if (success) {
        QList<App> newApps;
        newApps.append(app);
        onAppSourceUpdate(app.source, newApps);
        if (action == OnlyInstallApps)
            installApps();
        if (action == OnlyUpdateApps)
            updateApps();
        if (action == ProcessApps)
            processNextAction();
    } else {
        action = Idle;
    }
}

void MainTabForm::onAppRemoved(App app)
{
    if (app.state == App::Absent)
        return;
    bool isWorkingApp = app.source.type == AppSource::WorkingDirectory
            && app.state != App::Archived;
    if (!apps.contains(app.name))
        return;
    AppInfo& appInfo = apps[app.name];
    if (isWorkingApp)
        appInfo.cur = App::makeAbsent();
    else
        appInfo.latest = App::makeAbsent();
    if (!appInfo.cur.exists() && !appInfo.latest.exists())
        apps.remove(app.name);
    updateView();
}

void MainTabForm::onAppSourceUpdate(AppSource source, const QList<App>& newApps)
{
    bool needUpdate = false;
    foreach (const auto& app, newApps) {
        if (app.state == App::Absent)
            continue;
        bool isWorkingApp = source.type == AppSource::WorkingDirectory
                && app.state != App::Archived;
        if (apps.contains(app.name)) {
            AppInfo& appInfo = apps[app.name];
            if (isWorkingApp) {
                if (appInfo.cur != app) {
                    appInfo.cur = app;
                    if (appInfo.latest.version <= app.version)
                        appInfo.latest = App::makeAbsent();
                    needUpdate = true;
                }
            } else {
                bool isNewVersion = true;
                if (appInfo.cur.exists() && app.version <= appInfo.cur.version)
                    isNewVersion = false;
                if (appInfo.latest.exists() && app.version <= appInfo.latest.version)
                    isNewVersion = false;
                if (isNewVersion) {
                    appInfo.latest = app;
                    needUpdate = true;
                }
            }
        } else {
            App curApp = isWorkingApp ? app : App::makeAbsent();
            App latestApp = isWorkingApp ? App::makeAbsent() : app;
            apps.insert(app.name, AppInfo{ curApp, latestApp });
            needUpdate = true;
        }
    }
    if (needUpdate)
        updateView();
}

void MainTabForm::onLibraryDeployed(Library library, bool success)
{
    if (success) {
        curLib = library;
        if (action == InstallLibrary)
            processNextAction();
    } else {
        if (action == InstallLibrary) {
            QMessageBox::warning(this, "Обновление прервано",
                                 "Не удалось установить обновление текущей библиотеки.");
            action = Idle;
        }
    }
    updateView();
}

void MainTabForm::onLibraryRemoved(Library library)
{
    if (library.source.type == LibrarySource::WorkingDirectory) {
        curLib = Library::makeAbsent();
        updateView();
    }
}

void MainTabForm::on_updateStateButton_clicked()
{
    parent->updateAll();
}

void MainTabForm::on_installAllButton_clicked()
{
    processNextAction();
}
