#include "newappdialog.h"
#include "ui_newappdialog.h"
#include <QFileDialog>
#include <QRegExp>
#include <QRegExpValidator>

NewAppDialog::NewAppDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewAppDialog)
{
    ui->setupUi(this);
    QRegExp nameRegExp("[a-zA-Z0-9_]+");
    ui->nameEdit->setValidator(new QRegExpValidator(nameRegExp));

    connect(ui->useSourcesBox, SIGNAL(toggled(bool)), this, SLOT(setSourcePathEnabled(bool)));
}

NewAppDialog::~NewAppDialog()
{
    delete ui;
}

void NewAppDialog::reset()
{
    ui->nameEdit->setText("");
    ui->useSourcesBox->setChecked(false);
}

QString NewAppDialog::name() const
{
    return ui->nameEdit->text();
}

bool NewAppDialog::useSources() const
{
    return ui->useSourcesBox->isChecked();
}

QString NewAppDialog::sourcesPath() const
{
    return ui->sourcesPathEdit->text();
}

bool NewAppDialog::needCreateResources() const
{
    return ui->needCreateResourcesBox->isChecked();
}

void NewAppDialog::setSourcePathEnabled(bool value)
{
    ui->sourcesPathLabel->setEnabled(value);
    ui->sourcesPathEdit->setEnabled(value);
    ui->sourcesPathButton->setEnabled(value);
}

void NewAppDialog::on_sourcesPathButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Ввод пути к cpp-файлу",
                                                "", "Файл исходников C++ (*.cpp)");
    if (!path.isEmpty()) {
        ui->sourcesPathEdit->setText(path);
    }
}
