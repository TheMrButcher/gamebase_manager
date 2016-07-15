#include "aboutwindow.h"
#include "ui_aboutwindow.h"
#include "version.h"
#include <QFile>

AboutWindow::AboutWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutWindow)
{
    ui->setupUi(this);
    Version managerVersion;
    managerVersion.read(":/common/VERSION.txt");
    ui->gamebaseLabel->setText(ui->gamebaseLabel->text() + managerVersion.toString());
    setFixedSize(sizeHint());
}

AboutWindow::~AboutWindow()
{
    delete ui;
}
