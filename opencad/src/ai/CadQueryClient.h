#pragma once

#include <QObject>
#include <QProcess>
#include <QString>
#include <QVariantMap>

namespace opencad {
namespace ai {

class CadQueryClient : public QObject {
  Q_OBJECT
public:
  explicit CadQueryClient(QObject *parent = nullptr);
  ~CadQueryClient();

  /**
   * @brief Executes a CadQuery script and exports the result to STEP.
   * @param scriptContent The Python script content.
   * @param outputStepPath The desired output path for the STEP file.
   * @return true if process started successfully.
   */
  bool runScript(const QString &scriptContent, const QString &outputStepPath);

signals:
  void executionFinished(bool success, const QString &message,
                         const QString &outputStepPath);

private:
  QString m_pythonScriptPath;
  QString m_tempDir;

  void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

} // namespace ai
} // namespace opencad
