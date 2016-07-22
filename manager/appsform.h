#ifndef APPSFORM_H
#define APPSFORM_H

#include "appstablemodel.h"
#include <QWidget>

namespace Ui {
class AppsForm;
}

class MainWindow;

class AppsForm : public QWidget
{
    Q_OBJECT

public:
    explicit AppsForm(MainWindow *parent = 0);
    ~AppsForm();

    void clearAppsTable();
    void append(const QList<App>& apps);

private slots:
    void on_createButton_clicked();

private:
    Ui::AppsForm *ui;
    AppsTableModel* appsModel;
};

#endif // APPSFORM_H
