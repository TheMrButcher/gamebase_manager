#include "appsform.h"
#include "ui_appsform.h"
#include "library.h"
#include "settings.h"
#include "files.h"
#include "mainwindow.h"
#include "appsourcemanagerlist.h"
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QInputDialog>

AppsForm::AppsForm(MainWindow *parent) :
    QWidget(parent),
    ui(new Ui::AppsForm)
{
    ui->setupUi(this);

    appsModel = new AppsTableModel(this);
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
    if (!app.validate())
        return;
    appsModel->append(app);
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
    bool ok = false;
    QString name = QInputDialog::getText(this, "Введите название приложения",
                                         "Название приложения:", QLineEdit::Normal,
                                         "", &ok, Qt::WindowFlags(), Qt::ImhLatinOnly);
    if (name.isEmpty() || !ok)
        return;

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

    Files::copyTextFile(pkgDir.absoluteFilePath("main.cpp"),
                        dstDir.absoluteFilePath("main.cpp"));

    QFile srcSolution(pkgDir.absoluteFilePath("project_template.sln"));
    srcSolution.open(QIODevice::ReadOnly | QIODevice::Text);
    QString solutionData = QString::fromUtf8(srcSolution.readAll());
    solutionData.replace("project_template", name);
    Files::writeTextFile(solutionData, dstDir.absoluteFilePath(name + ".sln"), true);

    Files::copyTextFile(pkgDir.absoluteFilePath("project_template.vcxproj"),
                        dstDir.absoluteFilePath(name + ".vcxproj"), true);
    Files::copyTextFile(pkgDir.absoluteFilePath("project_template.vcxproj.filters"),
                        dstDir.absoluteFilePath(name + ".vcxproj.filters"), true);
    Files::copyTextFile(pkgDir.absoluteFilePath("project_template.vcxproj.user.template"),
                        dstDir.absoluteFilePath(name + ".vcxproj.user"), true);
    appsModel->append(App{ AppSource{ AppSource::WorkingDirectory, workingDir.path, SourceStatus::OK },
                           App::NotConfigured, name, Version{}, containerName });
}

int AppsForm::selectedRow() const
{
    auto rows = ui->appsTable->selectionModel()->selectedRows();
    if (rows.empty())
        return -1;
    return rows[0].row();
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
    } else {
        const auto& app = appsModel->get()[row];
        ui->configureButton->setEnabled(app.checkAbility(App::Configure));
        ui->removeButton->setEnabled(app.checkAbility(App::Remove));
        ui->compressButton->setEnabled(app.checkAbility(App::Compress));
        ui->addButton->setEnabled(app.checkAbility(App::Add));
        ui->deployButton->setEnabled(app.checkAbility(App::Deploy));
    }
}

void AppsForm::on_configureButton_clicked()
{

}

void AppsForm::on_removeButton_clicked()
{
    int row = selectedRow();
    if (row == -1)
        return;
    App app = appsModel->get()[row];

    if (!app.validate())
        return;

    appsModel->removeRow(row);
    app.removeConfig();
    QDir dir(app.source.path);
    if (dir.cd(app.containerName)) {
        dir.removeRecursively();
    } else {
        dir.remove(app.containerName);
    }
}

void AppsForm::on_compressButton_clicked()
{

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

}