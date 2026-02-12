#ifndef OPENCAD_AI_LLMCLIENT_H
#define OPENCAD_AI_LLMCLIENT_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>


namespace opencad {
namespace ai {

class LLMClient : public QObject {
  Q_OBJECT
public:
  explicit LLMClient(QObject *parent = nullptr);
  ~LLMClient();

  /**
   * @brief Sends a prompt to the LLM to generate CadQuery code.
   * @param userPrompt The natural language description.
   */
  void generateCode(const QString &userPrompt);

  /**
   * @brief Configures the API endpoint and model.
   * @param endpoint Base URL (e.g. http://localhost:11434/v1/chat/completions)
   * @param model Model name (e.g. llama3)
   */
  void setConfiguration(const QString &endpoint, const QString &model);

signals:
  void codeGenerated(const QString &code);
  void errorOccurred(const QString &error);

private slots:
  void onNetworkReply(QNetworkReply *reply);

private:
  QNetworkAccessManager *m_manager;
  QString m_apiEndpoint;
  QString m_modelName;

  // Helper to extract code block from markdown
  QString extractCodeFromResponse(const QString &responseContent);
};

} // namespace ai
} // namespace opencad

#endif // OPENCAD_AI_LLMCLIENT_H
