#include "appconfigurationdialog.h"
#include "ui_appconfigurationdialog.h"
#include "files.h"
#include "settings.h"
#include "dimensions.h"
#include <QFileDialog>
#include <QDebug>

namespace {
App renameApp(App app, QString newName)
{
    QDir workingDir(Settings::instance().workingDir().path);
    App newApp{ app.source, app.state, newName, app.version, App::makeContainerName(workingDir, newName) };
    QDir dir = workingDir;
    if (!dir.cd(app.containerName))
        return newApp;
    if (!App::createSolution(dir, newName))
        return newApp;
    if (!workingDir.rename(app.containerName, newApp.containerName))
        return newApp;
    dir = workingDir;
    dir.cd(newApp.containerName);
    dir.remove(app.name + ".sln");
    dir.rename(app.name + ".vcxproj", newApp.name + ".vcxproj");
    dir.rename(app.name + ".vcxproj.filters", newApp.name + ".vcxproj.filters");
    dir.rename(app.name + ".vcxproj.user", newApp.name + ".vcxproj.user");
    app.removeConfig();
    newApp.configurate();
    return newApp;
}

void setStatus(QLabel* label, bool status)
{
    static QPixmap errorPixmap(":/images/icons/Error.png");
    static QPixmap okPixmap(":/images/icons/Ok.png");
    label->setPixmap(status ? okPixmap : errorPixmap);
    label->setToolTip(status ? "В норме" : "Имеется проблема");
}
}

AppConfigurationDialog::AppConfigurationDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AppConfigurationDialog)
{
    ui->setupUi(this);

    Dimensions::createDimensionsVariants(ui->dimensionsComboBox);

    connect(ui->updateButton, SIGNAL(clicked()), this, SLOT(update()));
}

AppConfigurationDialog::~AppConfigurationDialog()
{
    delete ui;
}

void AppConfigurationDialog::setApp(App app)
{
    originApp = app;
    originConfig = app.config();
    set(originApp, originConfig);
}

App AppConfigurationDialog::resultApp() const
{
    App result{ originApp.source, App::NotConfigured, originApp.name,
              Version::fromString(ui->versionEdit->text()), originApp.containerName };
    return result;
}

AppConfig AppConfigurationDialog::resultConfig() const
{
    auto config = originConfig;
    config.imagesPath = ui->imagesPath->text();
    config.designPath = ui->designPath->text();
    config.soundsPath = ui->soundsPath->text();
    config.musicPath = ui->musicPath->text();
    config.additionalFontsPath = ui->additionalFontsPath->text();
    config.isWindow = ui->windowModeCheckBox->isChecked();
    Dimensions dims = Dimensions::fromString(ui->dimensionsComboBox->currentText());
    config.width = dims.width;
    config.height = dims.height;
    return config;
}

void AppConfigurationDialog::accept()
{
    update();
    App app = resultApp();
    if (originApp.name != ui->nameEdit->text()) {
        App newApp = renameApp(app, ui->nameEdit->text());
        if (newApp.validate())
            emit appRenamed(app, newApp);
    }
    QDialog::accept();
}

void AppConfigurationDialog::reject()
{
    set(originApp, originConfig);
    update();
    QDialog::reject();
}

void AppConfigurationDialog::set(App app, const AppConfig& config)
{
    ui->nameEdit->setText(app.name);
    ui->versionEdit->setText(app.version.toString());
    ui->imagesPath->setText(config.imagesPath);
    ui->designPath->setText(config.designPath);
    ui->soundsPath->setText(config.soundsPath);
    ui->musicPath->setText(config.musicPath);
    ui->additionalFontsPath->setText(config.additionalFontsPath);
    ui->windowModeCheckBox->setChecked(config.isWindow);
    ui->dimensionsComboBox->setCurrentText(Dimensions::toString(config.width, config.height));
    updateStatuses(app);
}

void AppConfigurationDialog::update()
{
    auto app = resultApp();
    app.setConfig(resultConfig());
    app.configurate();
    emit appUpdated(app);
    updateStatuses(app);
}

void AppConfigurationDialog::updateStatuses(App app)
{
    bool hasVersionFile = false;
    bool hasConfig = false;
    bool isMainCppOK = app.isMainCppOK();
    bool hasManagerProject = false;
    QDir dir(app.source.path);
    if (dir.cd(app.containerName)) {
        Version version;
        hasVersionFile = version.read(dir.absoluteFilePath("VERSION.txt"));
        hasConfig = dir.exists(Files::APP_CONFIG_NAME);
        if (hasConfig)
            hasConfig = app.copyConfig();
        hasManagerProject = dir.exists(Files::APP_PROJECT_NAME);
    }

    setStatus(ui->versionStatus, hasVersionFile);
    setStatus(ui->configStatus, hasConfig);
    setStatus(ui->mainCppStatus, isMainCppOK);
    setStatus(ui->managerProjectStatus, hasManagerProject);
}

void AppConfigurationDialog::on_chooseImagesPathButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Ввод пути к папке с изображениями",
                                                     Settings::instance().workingDir().path);
    if (!path.isEmpty())
        ui->imagesPath->setText(path);
}

void AppConfigurationDialog::on_chooseDesignPathButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Ввод пути к папке с дизайном",
                                                     Settings::instance().workingDir().path);
    if (!path.isEmpty())
        ui->designPath->setText(path);
}

void AppConfigurationDialog::on_chooseSoundsPathButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Ввод пути к папке со звуками",
                                                     Settings::instance().workingDir().path);
    if (!path.isEmpty())
        ui->soundsPath->setText(path);
}

void AppConfigurationDialog::on_chooseMusicPathButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Ввод пути к папке с музыкой",
                                                     Settings::instance().workingDir().path);
    if (!path.isEmpty())
        ui->musicPath->setText(path);
}

void AppConfigurationDialog::on_chooseAdditionalFontsPathButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Ввод пути к папке с дополнительными шрифтами",
                                                     Settings::instance().workingDir().path);
    if (!path.isEmpty())
        ui->additionalFontsPath->setText(path);
}
