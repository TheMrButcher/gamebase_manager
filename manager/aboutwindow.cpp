#include "aboutwindow.h"
#include "ui_aboutwindow.h"
#include "version.h"
#include <QFile>

AboutWindow::AboutWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutWindow)
{
    ui->setupUi(this);
    ui->gamebaseLabel->setText(
                ui->gamebaseLabel->text() + Version::read(":/common/VERSION.txt").toString());
    setFixedSize(sizeHint());
}

AboutWindow::~AboutWindow()
{
    delete ui;
}
