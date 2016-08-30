#include "firstusagedialog.h"
#include "ui_firstusagedialog.h"
#include "version.h"

FirstUsageDialog::FirstUsageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FirstUsageDialog)
{
    ui->setupUi(this);

    Version managerVersion;
    managerVersion.read(":/common/VERSION.txt");
    ui->titleLabel->setText(ui->titleLabel->text() + managerVersion.toString());
}

FirstUsageDialog::~FirstUsageDialog()
{
    delete ui;
}
