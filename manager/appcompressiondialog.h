#ifndef APPCOMPRESSIONDIALOG_H
#define APPCOMPRESSIONDIALOG_H

#include "app.h"
#include <QDialog>

namespace Ui {
class AppCompressionDialog;
}

class QItemSelection;

class AppCompressionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AppCompressionDialog(QWidget *parent = 0);
    ~AppCompressionDialog();

    void set(const App& app);
    QString name() const;
    QString path() const;

private slots:
    void onDestSelectionChanged(const QItemSelection&, const QItemSelection&);

    void on_selectButton_clicked();

    void on_setPathButton_clicked();

private:
    Ui::AppCompressionDialog *ui;
};

#endif // APPCOMPRESSIONDIALOG_H
