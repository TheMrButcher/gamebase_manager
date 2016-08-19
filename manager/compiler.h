#ifndef COMPILER_H
#define COMPILER_H

#include <QObject>
#include <QDir>
#include <QString>
#include <QAtomicInt>

class Compiler : public QObject
{
    Q_OBJECT

public:
    Compiler(QObject* parent);

    bool compile(QDir projectDir);
    bool compile(QDir projectDir, QString projectName);

public slots:
    void cancel();

private:
    bool compileImpl(QDir projectDir, QString projectName);

    QAtomicInt cancelFlag;
};

#endif // COMPILER_H
