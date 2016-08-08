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
    static void invokeStart(int value);
    static void invokeSetProgress(int value);

signals:
    void canceled();

public slots:
    void show(QString title, QString labelBeforeStart, QString labelAfterStart);
    void start(int maxValue);
    void setProgress(int value);

private:
    QProgressDialog* progressDialog;
    QString labelAfterStart;
    int maxValue;
};

#endif // PROGRESSMANAGER_H
