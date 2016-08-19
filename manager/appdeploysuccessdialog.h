#ifndef APPDEPLOYSUCCESSDIALOG_H
#define APPDEPLOYSUCCESSDIALOG_H

#include <QDialog>

namespace Ui {
class AppDeploySuccessDialog;
}

class AppDeploySuccessDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AppDeploySuccessDialog(QWidget *parent = 0);
    ~AppDeploySuccessDialog();

    void set(QString path, QString name);

private slots:
    void on_openDirButton_clicked();

    void on_runAppButton_clicked();

private:
    Ui::AppDeploySuccessDialog *ui;

    QString appPath;
    QString appName;
};

#endif // APPDEPLOYSUCCESSDIALOG_H
