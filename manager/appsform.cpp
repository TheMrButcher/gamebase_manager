#include "appsform.h"
#include "ui_appsform.h"

AppsForm::AppsForm(QWidget *parent) :
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
}

AppsForm::~AppsForm()
{
    delete ui;
}
