#ifndef NEWAPPDIALOG_H
#define NEWAPPDIALOG_H

#include <QDialog>

namespace Ui {
class NewAppDialog;
}

class NewAppDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewAppDialog(QWidget *parent = 0);
    ~NewAppDialog();

    void reset();
    QString name() const;
    bool useSources() const;
    QString sourcesPath() const;
    bool needCreateResources() const;

private slots:
    void setSourcePathEnabled(bool value);

    void on_sourcesPathButton_clicked();

private:
    Ui::NewAppDialog *ui;
};

#endif // NEWAPPDIALOG_H
