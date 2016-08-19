#ifndef PROGRESSMANAGER_H
#define PROGRESSMANAGER_H

#include <QObject>

class QProgressDialog;

class ProgressManager : public QObject
{
    Q_OBJECT

public:
    ProgressManager(QWidget* parent);

    static ProgressManager* instance();
    static void invokeShow(QString title, QString labelAfterStart);
    static void invokeSetLabel(QString label);
    static void invokeStart(int value);
    static void invokeSetProgress(int value);

signals:
    void canceled();

public slots:
    void show(QString title, QString labelAfterStart);
    void setLabel(QString label);
    void start(int maxValue);
    void setProgress(int value);

private:
    QProgressDialog* progressDialog;
    QString labelAfterStart;
    int maxValue;
};

#endif // PROGRESSMANAGER_H
