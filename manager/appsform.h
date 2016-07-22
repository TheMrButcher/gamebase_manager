#ifndef APPSFORM_H
#define APPSFORM_H

#include "appstablemodel.h"
#include <QWidget>

namespace Ui {
class AppsForm;
}

class AppsForm : public QWidget
{
    Q_OBJECT

public:
    explicit AppsForm(QWidget *parent = 0);
    ~AppsForm();

private:
    Ui::AppsForm *ui;
    AppsTableModel* appsModel;
};

#endif // APPSFORM_H
