#include "CadQueryClient.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTemporaryFile>

namespace opencad {
namespace ai {

CadQueryClient::CadQueryClient(QObject *parent) : QObject(parent) {
  // Dynamic path resolution
  QDir appDir(QCoreApplication::applicationDirPath());

  // ../../scripts/cadquery/run_cq.py relative to bin/
  QString relativePath =
      appDir.absoluteFilePath("../../scripts/cadquery/run_cq.py");

  if (QFile::exists(relativePath)) {
    m_pythonScriptPath = relativePath;
  } else {
    // Fallback or dev path
    m_pythonScriptPath =
        "c:/Projects/opencadandsimulation/opencad/scripts/cadquery/run_cq.py";
  }

  m_tempDir = appDir.absoluteFilePath("../../temp_cq_output");
  QDir().mkpath(m_tempDir);
}

CadQueryClient::~CadQueryClient() {}

bool CadQueryClient::runScript(const QString &scriptContent,
                               const QString &outputStepPath) {
  if (!QFile::exists(m_pythonScriptPath)) {
    emit executionFinished(
        false, "Python script wrapper not found at: " + m_pythonScriptPath, "");
    return false;
  }

  // Write script content to a temp file
  QString tempScriptPath = m_tempDir + "/temp_script.py";
  QFile scriptFile(tempScriptPath);
  if (!scriptFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
    emit executionFinished(false, "Failed to write temporary script file.", "");
    return false;
  }
  scriptFile.write(scriptContent.toUtf8());
  scriptFile.close();

  // Clean up previous output to avoid false positives
  if (QFile::exists(outputStepPath)) {
    QFile::remove(outputStepPath);
  }

  QProcess *process = new QProcess(this);
  QStringList args;
  args << m_pythonScriptPath;
  args << "--script" << tempScriptPath;
  args << "--output" << outputStepPath;

  qDebug() << "CadQueryClient: Starting Python process...";
  qDebug() << "  Command: python" << args.join(" ");

  // Connect signals to capture output for debugging
  connect(process, &QProcess::readyReadStandardOutput, [process]() {
    qDebug() << "  [Python Stdout]:" << process->readAllStandardOutput();
  });
  connect(process, &QProcess::readyReadStandardError, [process]() {
    qDebug() << "  [Python Stderr]:" << process->readAllStandardError();
  });

  connect(process,
          QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this,
          [this, process, outputStepPath](int exitCode,
                                          QProcess::ExitStatus exitStatus) {
            qDebug() << "CadQueryClient: Process finished with exit code"
                     << exitCode;

            // On Windows, cadquery-ocp sometimes crashes on exit (Heap
            // Corruption 0xC0000374) even if the script ran successfully. We
            // check if the output file exists.
            bool fileExists = QFile::exists(outputStepPath);

            // If file exists, we consider it a success regardless of exit code
            // (unless it's 0 which is also success)
            if (exitCode == 0 || fileExists) {
              if (exitCode != 0) {
                qDebug() << "CadQueryClient: Process had non-zero exit code "
                            "but file exists. Treating as success.";
              }
              emit executionFinished(true, "Execution successful.",
                                     outputStepPath);
            } else {
              QString err =
                  "Process failed with exit code " + QString::number(exitCode);
              emit executionFinished(false, "CadQuery Error:\n" + err, "");
            }
            process->deleteLater();
          });

  process->start("python", args);

  if (!process->waitForStarted()) {
    qDebug() << "CadQueryClient: Failed to start process!";
    emit executionFinished(
        false, "Failed to start Python process. Is python in PATH?", "");
    delete process;
    return false;
  }

  return true;
}

} // namespace ai
} // namespace opencad
