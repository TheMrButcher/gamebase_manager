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
    enum class BuildType {
        Debug,
        Release
    };

    Compiler(QObject* parent);

    bool compile(QDir projectDir, BuildType buildType);
    bool compile(QDir projectDir, QString projectName, BuildType buildType);

public slots:
    void cancel();

private:
    bool compileImpl(QDir projectDir, QString projectName, QString buildType);

    QAtomicInt cancelFlag;
};

#endif // COMPILER_H
