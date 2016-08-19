#include "appdeploysuccessdialog.h"
#include "ui_appdeploysuccessdialog.h"
#include <QDesktopServices>
#include <QProcess>
#include <QUrl>
#include <QDir>

AppDeploySuccessDialog::AppDeploySuccessDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AppDeploySuccessDialog)
{
    ui->setupUi(this);
}

AppDeploySuccessDialog::~AppDeploySuccessDialog()
{
    delete ui;
}

void AppDeploySuccessDialog::set(QString path, QString name)
{
    appPath = path;
    appName = name;
}

void AppDeploySuccessDialog::on_openDirButton_clicked()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(appPath));
}

void AppDeploySuccessDialog::on_runAppButton_clicked()
{
    QProcess::startDetached(QDir(appPath).absoluteFilePath(appName), QStringList(), appPath);
}
