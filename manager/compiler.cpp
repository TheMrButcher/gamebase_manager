#include "compiler.h"
#include "progressmanager.h"
#include "settings.h"
#include <QFile>
#include <QProcess>
#include <QProcessEnvironment>
#include <QDebug>

namespace {
const QString COMPILATION_BATCH_NAME = "compile.bat";
}

Compiler::Compiler(QObject* parent)
    : QObject(parent)
{
    connect(ProgressManager::instance(), SIGNAL(canceled()), this, SLOT(cancel()));
}

bool Compiler::compile(QDir projectDir)
{
    return compile(projectDir, projectDir.dirName());
}

bool Compiler::compile(QDir projectDir, QString projectName)
{
    ProgressManager::invokeShow("Построение...", "");
    ProgressManager::invokeSetLabel("Идет построение...");
    QFile::copy(":/scripts/compile.bat", projectDir.absoluteFilePath(COMPILATION_BATCH_NAME));
    auto result = compileImpl(projectDir, projectName);
    projectDir.remove(COMPILATION_BATCH_NAME);
    if (static_cast<int>(cancelFlag) == 0) {
        ProgressManager::invokeStart(1);
        ProgressManager::invokeSetProgress(1);
    } else {
        return false;
    }
    return result;
}

void Compiler::cancel()
{
    cancelFlag.testAndSetOrdered(0, 1);
}

bool Compiler::compileImpl(QDir projectDir, QString projectName)
{
    QProcess cmdProcess;
    cmdProcess.setWorkingDirectory(projectDir.absolutePath());
    cmdProcess.setProcessChannelMode(QProcess::MergedChannels);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    auto vcVarsPath = Settings::instance().vcVarsPath;
    if (vcVarsPath.isEmpty())
        return false;
    env.insert("VISUAL_CPP_VARIABLES_PATH", vcVarsPath);
    env.insert("SOLUTION_TO_BUILD_NAME", projectName + ".sln");
    cmdProcess.setProcessEnvironment(env);

    QStringList arguments;
    arguments << "/U" << "/C" << COMPILATION_BATCH_NAME;
    cmdProcess.start("cmd.exe", arguments);
    qDebug() << "Started process";
    if (!cmdProcess.waitForStarted(5000))
        return false;
    while (cmdProcess.state() == QProcess::Running) {
        if (static_cast<int>(cancelFlag) == 1) {
            cmdProcess.kill();
            return false;
        }
        while (cmdProcess.waitForReadyRead(2000)) {
            while (cmdProcess.canReadLine()) {
                if (static_cast<int>(cancelFlag) == 1) {
                    cmdProcess.kill();
                    return false;
                }
                qDebug() << QString::fromUtf8(cmdProcess.readLine());
            }
        }
    }
    while (cmdProcess.canReadLine())
        qDebug() << QString::fromUtf8(cmdProcess.readLine());
    return cmdProcess.exitCode() == 0;
}
