#ifndef LIBRARIESFORM_H
#define LIBRARIESFORM_H

#include "librariestablemodel.h"
#include <QWidget>

namespace Ui {
class LibrariesForm;
}

class LibrariesForm : public QWidget
{
    Q_OBJECT

public:
    explicit LibrariesForm(QWidget *parent = 0);
    ~LibrariesForm();

public slots:
    void update();

private:
    Ui::LibrariesForm *ui;
    LibrariesTableModel* librariesModel;
};

#endif // LIBRARIESFORM_H
