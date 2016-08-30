#ifndef FIRSTUSAGEDIALOG_H
#define FIRSTUSAGEDIALOG_H

#include <QDialog>

namespace Ui {
class FirstUsageDialog;
}

class FirstUsageDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FirstUsageDialog(QWidget *parent = 0);
    ~FirstUsageDialog();

private:
    Ui::FirstUsageDialog *ui;
};

#endif // FIRSTUSAGEDIALOG_H
