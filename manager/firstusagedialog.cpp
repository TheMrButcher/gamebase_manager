#include "firstusagedialog.h"
#include "ui_firstusagedialog.h"
#include "version.h"

FirstUsageDialog::FirstUsageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FirstUsageDialog)
{
    ui->setupUi(this);
    ui->titleLabel->setText(ui->titleLabel->text() + Version::selfVersion().toString());
}

FirstUsageDialog::~FirstUsageDialog()
{
    delete ui;
}
