#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    about = new AboutWindow(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_aboutAction_triggered()
{
    about->show();
}
