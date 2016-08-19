#include "appsform.h"
#include "ui_appsform.h"
#include "library.h"
#include "settings.h"
#include "files.h"
#include "mainwindow.h"
#include "appsourcemanagerlist.h"
#include "newappdialog.h"
#include "appconfigurationdialog.h"
#include "appcompressiondialog.h"
#include "appcompressor.h"
#include "appdeployer.h"
#include "appdeploysuccessdialog.h"
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QInputDialog>
#include <QUrl>
#include <QDesktopServices>
#include <QThreadPool>

AppsForm::AppsForm(MainWindow *parent) :
    QWidget(parent),
    ui(new Ui::AppsForm)
{
    ui->setupUi(this);

    appsModel = new AppsTableModel(this);
    newAppDialog = new NewAppDialog(this);
    newAppDialog->hide();
    configDialog = new AppConfigurationDialog(this);
    configDialog->hide();
    compressionDialog = new AppCompressionDialog(this);
    compressionDialog->hide();
    deploySuccessDialog = new AppDeploySuccessDialog(this);
    deploySuccessDialog->hide();
    ui->appsTable->setModel(appsModel);
    ui->appsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->appsTable->setColumnWidth(1, 150);
    ui->appsTable->setColumnWidth(2, 150);
    ui->appsTable->setColumnWidth(3, 50);
    ui->appsTable->setColumnWidth(4, 50);

    connect(ui->updateButton, SIGNAL(clicked()), parent, SLOT(updateAppSources()));
    connect(ui->appsTable->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(onAppsSelectionChanged(QItemSelection,QItemSelection)));
    connect(AppSourceManagerList::instance(), SIGNAL(finishedAdd(App)),
            this, SLOT(onAppAdded(App)));

    connect(configDialog, SIGNAL(appUpdated(App)), this, SLOT(onAppUpdate(App)));
    connect(configDialog, SIGNAL(appRenamed(App,App)), this, SLOT(onAppRename(App,App)));
}

AppsForm::~AppsForm()
{
    delete ui;
}

void AppsForm::clearAppsTable()
{
    appsModel->set(QList<App>());
    ui->appsTable->selectionModel()->clearSelection();
}

void AppsForm::append(const QList<App>& apps)
{
    foreach (const auto& app, apps)
        appsModel->append(app);
}

void AppsForm::onAppAdded(App app)
{
    if (!app.exists())
        return;
    if (app.checkAbility(App::Configure))
        app.configurate();
    app.validate();
    appsModel->append(app);
}

void AppsForm::onAppUpdate(App app)
{
    App newApp = app;
    newApp.validate();
    appsModel->replace(app.source, app.containerName, newApp);
}

void AppsForm::onAppRename(App oldApp, App newApp)
{
    appsModel->replace(oldApp.source, oldApp.containerName, newApp);
}

void AppsForm::onTempAppAdded(App app)
{
    if (!app.exists()) {
        QMessageBox::warning(this, "Ошибка при создании временной папки",
                             "Невозможно создать папку для построения приложения");
        return;
    }

    QDir dir;
    auto outputPath = Settings::instance().outputPath;
    if (!dir.cd(outputPath)) {
        auto answer = QMessageBox::question(this, "Создание папки для построенных приложений",
                                            "Папка для построенных приложений отсутствует. Создать?");
        if (answer != QMessageBox::Yes)
            return;
        dir.mkpath(outputPath);
        if (!dir.cd(outputPath))
            return;
    }

    QString containerName = app.name + "_" + app.version.version.join('_');
    containerName = App::makeContainerName(dir, containerName);
    if (containerName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка при построении приложения",
                             "Невозможно создать папку для построенного приложения");
        return;
    }

    auto deployer = new AppDeployer(app, dir.absoluteFilePath(containerName));
    connect(deployer, SIGNAL(finishedDeploy(App,QString,bool)),
            this, SLOT(onAppDeployed(App,QString,bool)));
    QThreadPool::globalInstance()->start(deployer);
}

void AppsForm::onAppDeployed(App app, QString path, bool success)
{
    removeApp(app);
    if (success) {
        deploySuccessDialog->set(path, app.name + ".exe");
        deploySuccessDialog->exec();
    } else {
        QMessageBox::warning(this, "Ошибка при построении приложения",
                             "Произошла ошибка при построении приложения. "
                             "Проверьте, что приложение правильно сконфигурирвано "
                             "и может быть скомпилировано на данном компьютере.");
    }
}

void AppsForm::onAppsSelectionChanged(const QItemSelection&, const QItemSelection&)
{
    updateButtons();
}

void AppsForm::on_createButton_clicked()
{
    auto workingDir = Settings::instance().workingDir();
    if (!Library::fromFileSystem(workingDir).exists()) {
        QMessageBox::warning(this, "Библиотека Gamebase не установлена",
                             "Для создания проекта необходима библиотека Gamebase. "
                             "Пожалуйста, перейдите во вкладку \"Библиотеки\" и "
                             "установите библиотеку Gamebase.");
        return;
    }
    QDir dir(workingDir.path);
    auto pkgDir = dir;
    pkgDir.cd(Files::DEPLOYED_ROOT_DIR_NAME);
    pkgDir.cd(Files::PACKAGE_DIR_NAME);
    pkgDir.cd("project_template");

    newAppDialog->reset();
    bool ok = newAppDialog->exec() == QDialog::Accepted;
    if (newAppDialog->name().isEmpty() || !ok)
        return;

    QString name = newAppDialog->name();
    QString containerName = App::makeContainerName(dir, name);
    bool success = !containerName.isEmpty() && dir.mkdir(containerName);

    if (!success) {
        QMessageBox::warning(this, "Ошибка при создании приложения",
                             "Невозможно создать приложение с указанным именем. "
                             "Пожалуйста, проверьте, что в рабочей папке нет файлов или папок "
                             "с таким именем, а также отсутствие в имени специальных символов.");
        return;
    }

    auto dstDir = dir;
    dstDir.cd(containerName);

    QString mainCppSourcePath;
    if (newAppDialog->useSources())
        mainCppSourcePath = QDir().absoluteFilePath(newAppDialog->sourcesPath());
    else
        mainCppSourcePath = pkgDir.absoluteFilePath("main.cpp");
    Files::copyTextFile(mainCppSourcePath,
                        dstDir.absoluteFilePath("main.cpp"));

    App::createSolution(dstDir, name);

    Files::copyTextFile(pkgDir.absoluteFilePath("project_template.vcxproj"),
                        dstDir.absoluteFilePath(name + ".vcxproj"), true);
    Files::copyTextFile(pkgDir.absoluteFilePath("project_template.vcxproj.filters"),
                        dstDir.absoluteFilePath(name + ".vcxproj.filters"), true);
    Files::copyTextFile(pkgDir.absoluteFilePath("project_template.vcxproj.user.template"),
                        dstDir.absoluteFilePath(name + ".vcxproj.user"), true);
    App app{ AppSource{ AppSource::WorkingDirectory, workingDir.path, SourceStatus::OK },
                           App::NotConfigured, name, Version{}, containerName };
    app.configurate();
    app.validate();
    appsModel->append(app);
}

int AppsForm::selectedRow() const
{
    auto rows = ui->appsTable->selectionModel()->selectedRows();
    if (rows.empty())
        return -1;
    return rows[0].row();
}

void AppsForm::removeApp(App app)
{
    app.removeConfig();
    QDir dir(app.source.path);
    if (dir.cd(app.containerName)) {
        dir.removeRecursively();
    } else {
        dir.remove(app.containerName);
    }
}

void AppsForm::updateButtons()
{
    int row = selectedRow();
    if (row == -1) {
        ui->configureButton->setEnabled(false);
        ui->removeButton->setEnabled(false);
        ui->compressButton->setEnabled(false);
        ui->addButton->setEnabled(false);
        ui->deployButton->setEnabled(false);
        ui->openSolutionButton->setEnabled(false);
        ui->openDirButton->setEnabled(false);
    } else {
        const auto& app = appsModel->get()[row];
        ui->configureButton->setEnabled(app.checkAbility(App::Configure));
        ui->removeButton->setEnabled(app.checkAbility(App::Remove));
        ui->compressButton->setEnabled(app.checkAbility(App::Compress));
        ui->addButton->setEnabled(app.checkAbility(App::Add));
        ui->deployButton->setEnabled(app.checkAbility(App::Deploy));
        ui->openSolutionButton->setEnabled(app.checkAbility(App::OpenSolution));
        ui->openDirButton->setEnabled(app.checkAbility(App::OpenDirectory));
    }
}

void AppsForm::on_configureButton_clicked()
{
    int row = selectedRow();
    if (row == -1)
        return;
    App app = appsModel->get()[row];

    if (!app.validate())
        return;

    configDialog->setApp(app);
    configDialog->show();
}

void AppsForm::on_removeButton_clicked()
{
    int row = selectedRow();
    if (row == -1)
        return;
    App app = appsModel->get()[row];

    if (!app.validate())
        return;

    auto answer = QMessageBox::question(this, "Удаление приложения",
                                        "Вы уверены, что хотите удалить выбранное приложение?");
    if (answer != QMessageBox::Yes)
        return;

    appsModel->removeRow(row);
    removeApp(app);
}

void AppsForm::on_compressButton_clicked()
{
    int row = selectedRow();
    if (row == -1)
        return;
    App app = appsModel->get()[row];

    compressionDialog->set(app);
    if (compressionDialog->exec() != QDialog::Accepted)
        return;

    AppSource newAppSource{ AppSource::Directory, compressionDialog->path(), SourceStatus::Unknown };
    if (newAppSource.check() != SourceStatus::OK) {
        if (QDir().mkpath(compressionDialog->path()) || newAppSource.check() != SourceStatus::OK) {
            QMessageBox::warning(this, "Ошибка при сжатии приложения",
                                 "Невозможно создать выбранную для архива папку");
            return;
        }
    }
    if (compressionDialog->path() == Settings::instance().workingDir().path)
        newAppSource.type = AppSource::WorkingDirectory;
    App newApp{ newAppSource, App::Archived, app.name, app.version, compressionDialog->name() + ".zip" };

    auto compressor = new AppCompressor(app, newApp);
    bool isAppSource = false;
    foreach (const auto& source, Settings::instance().appSources)
        if (source == newAppSource)
            isAppSource = true;
    if (isAppSource)
        connect(compressor, SIGNAL(finishedCompress(App)),
                this, SLOT(onAppAdded(App)));
    QThreadPool::globalInstance()->start(compressor);
}

void AppsForm::on_addButton_clicked()
{
    int row = selectedRow();
    if (row == -1)
        return;
    App app = appsModel->get()[row];

    auto workingDir = Settings::instance().workingDir();
    if (workingDir.check() != SourceStatus::OK) {
        auto answer = QMessageBox::question(this, "Создание рабочей папки",
                                            "Рабочая папка отсутствует. Создать?");
        if (answer != QMessageBox::Yes)
            return;
        QDir dir;
        dir.mkpath(workingDir.path);
        if (workingDir.check() != SourceStatus::OK)
            return;
    }

    AppSourceManagerList::instance()->addToWorkingDir(app);
}

void AppsForm::on_deployButton_clicked()
{
    int row = selectedRow();
    if (row == -1)
        return;
    App app = appsModel->get()[row];

    QString tempAppContainerName = App::makeContainerName(
                QDir(app.source.path),
                app.containerName + "_temp");
    if (tempAppContainerName.isEmpty()) {
        QMessageBox::warning(this, "Ошибка при создании временной папки",
                             "Невозможно создать папку для построения приложения");
        return;
    }

    App tempApp{ app.source, App::NotConfigured, app.name, app.version, tempAppContainerName };
    auto compressor = new AppCompressor(app, tempApp);
    connect(compressor, SIGNAL(finishedCompress(App)),
            this, SLOT(onTempAppAdded(App)));
    QThreadPool::globalInstance()->start(compressor);
}

void AppsForm::on_openSolutionButton_clicked()
{
    int row = selectedRow();
    if (row == -1)
        return;
    App app = appsModel->get()[row];

    QDir dir(app.source.path);
    if (!dir.cd(app.containerName))
        return;
    QString solutionPath = dir.absoluteFilePath(app.name + ".sln");
    QDesktopServices::openUrl(QUrl::fromLocalFile(solutionPath));
}

void AppsForm::on_openDirButton_clicked()
{
    int row = selectedRow();
    if (row == -1)
        return;
    App app = appsModel->get()[row];

    QString path = app.source.path;
    if (app.state == App::NotConfigured || app.state == App::Full) {
        QDir dir(app.source.path);
        path = dir.absoluteFilePath(app.containerName);
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}
