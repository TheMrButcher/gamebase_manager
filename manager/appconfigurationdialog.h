#ifndef APPCONFIGURATIONDIALOG_H
#define APPCONFIGURATIONDIALOG_H

#include "app.h"
#include "appconfig.h"
#include <QDialog>

namespace Ui {
class AppConfigurationDialog;
}

class AppConfigurationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AppConfigurationDialog(QWidget *parent = 0);
    ~AppConfigurationDialog();

    void setApp(App app);
    App resultApp() const;
    AppConfig resultConfig() const;

signals:
    void appUpdated(App app);
    void appRenamed(App oldApp, App newApp);

public slots:
    virtual void accept() override;
    virtual void reject() override;

private slots:
    void update();

    void on_chooseImagesPathButton_clicked();

    void on_chooseDesignPathButton_clicked();

    void on_chooseSoundsPathButton_clicked();

    void on_chooseMusicPathButton_clicked();

    void on_chooseAdditionalFontsPathButton_clicked();

private:
    void set(App app, const AppConfig& config);
    void updateStatuses(App app);

    Ui::AppConfigurationDialog *ui;
    App originApp;
    AppConfig originConfig;
};

#endif // APPCONFIGURATIONDIALOG_H
