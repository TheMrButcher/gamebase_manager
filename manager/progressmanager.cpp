#include "progressmanager.h"
#include <QProgressDialog>

ProgressManager::ProgressManager(QWidget* parent)
    : QObject(parent)
{
    progressDialog = new QProgressDialog(parent);
    progressDialog->setMinimumWidth(400);
    progressDialog->setModal(true);
    progressDialog->setRange(0, 1);
    progressDialog->setValue(1);
    progressDialog->hide();
    progressDialog->setMinimumDuration(0);

    connect(progressDialog, SIGNAL(canceled()), this, SIGNAL(canceled()));
}

void ProgressManager::invokeShow(QString title, QString labelAfterStart)
{
    QMetaObject::invokeMethod(instance(), "show",
                              Q_ARG(QString, title), Q_ARG(QString, labelAfterStart));
}

void ProgressManager::invokeStart(int value)
{
    QMetaObject::invokeMethod(instance(), "start", Q_ARG(int, value));
}

void ProgressManager::invokeSetProgress(int value)
{
    QMetaObject::invokeMethod(instance(), "setProgress", Q_ARG(int, value));
}

void ProgressManager::show(QString title, QString labelAfterStart)
{
    progressDialog->setWindowTitle(title);
    progressDialog->setRange(0, 0);
    progressDialog->setValue(0);
    progressDialog->setLabelText("Подготавливается список файлов...");
    progressDialog->show();
    this->labelAfterStart = labelAfterStart;
}

void ProgressManager::start(int maxValue)
{
    progressDialog->setRange(0, maxValue);
    progressDialog->setValue(0);
    this->maxValue = maxValue;
}

void ProgressManager::setProgress(int value)
{
    progressDialog->setValue(value);
    progressDialog->setLabelText(labelAfterStart + ": " + QString::number(value)
                                 + "/" + QString::number(maxValue));
}
