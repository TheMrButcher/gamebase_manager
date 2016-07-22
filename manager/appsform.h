#ifndef APPSFORM_H
#define APPSFORM_H

#include "appstablemodel.h"
#include <QWidget>

namespace Ui {
class AppsForm;
}

class MainWindow;
class QItemSelection;

class AppsForm : public QWidget
{
    Q_OBJECT

public:
    explicit AppsForm(MainWindow *parent = 0);
    ~AppsForm();

    void clearAppsTable();
    void append(const QList<App>& apps);

private slots:
    void onAppAdded(App app);

    void onAppsSelectionChanged(const QItemSelection &, const QItemSelection &);

    void on_createButton_clicked();

    void on_configureButton_clicked();

    void on_removeButton_clicked();

    void on_compressButton_clicked();

    void on_addButton_clicked();

    void on_deployButton_clicked();

private:
    int selectedRow() const;
    void updateButtons();

    Ui::AppsForm *ui;
    AppsTableModel* appsModel;
};

#endif // APPSFORM_H
