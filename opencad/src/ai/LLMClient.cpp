#include "LLMClient.h"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>

namespace opencad {
namespace ai {

LLMClient::LLMClient(QObject *parent) : QObject(parent) {
  m_manager = new QNetworkAccessManager(this);
  connect(m_manager, &QNetworkAccessManager::finished, this,
          &LLMClient::onNetworkReply);

  // Default configuration (Ollama)
  m_apiEndpoint = "http://localhost:11434/v1/chat/completions";
  m_modelName = "opencad-parser"; // Strict domain parser model
}

LLMClient::~LLMClient() {}

void LLMClient::setConfiguration(const QString &endpoint,
                                 const QString &model) {
  m_apiEndpoint = endpoint;
  m_modelName = model;
}

void LLMClient::generateCode(const QString &userPrompt) {
  QUrl url(m_apiEndpoint);
  QNetworkRequest request(url);
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

  // System prompt is baked into the cadquery-coder Modelfile.
  // Only send the user message here.
  QJsonArray messages;
  QJsonObject userMsg;
  userMsg["role"] = "user";
  userMsg["content"] = userPrompt;
  messages.append(userMsg);

  QJsonObject payload;
  payload["model"] = m_modelName;
  payload["messages"] = messages;
  payload["stream"] = false;
  payload["temperature"] = 0;

  QJsonDocument doc(payload);
  qDebug() << "LLMClient: Sending request to" << m_apiEndpoint;
  m_manager->post(request, doc.toJson());
}

void LLMClient::onNetworkReply(QNetworkReply *reply) {
  qDebug() << "LLMClient: Received reply from" << reply->url().toString();

  if (reply->error() != QNetworkReply::NoError) {
    qCritical() << "LLMClient: Network Error:" << reply->errorString();
    emit errorOccurred("Network Error: " + reply->errorString());
    reply->deleteLater();
    return;
  }

  QByteArray data = reply->readAll();
  QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
  QJsonObject jsonObj = jsonDoc.object();

  qDebug() << "LLMClient: Response Body:" << data;

  if (jsonObj.contains("error")) {
    // OpenAI/Ollama API error format
    QJsonObject errObj = jsonObj["error"].toObject();
    emit errorOccurred("API Error: " + errObj["message"].toString());
    reply->deleteLater();
    return;
  }

  // Extract content
  // Format: choices[0].message.content
  if (jsonObj.contains("choices") && jsonObj["choices"].isArray()) {
    QJsonArray choices = jsonObj["choices"].toArray();
    if (!choices.isEmpty()) {
      QJsonObject firstChoice = choices[0].toObject();
      QJsonObject message = firstChoice["message"].toObject();
      QString content = message["content"].toString();

      QString extractedCode = extractCodeFromResponse(content);
      emit codeGenerated(extractedCode);
    } else {
      emit errorOccurred("Empty choices in response.");
    }
  } else {
    emit errorOccurred("Invalid JSON response format.");
  }

  reply->deleteLater();
}

QString LLMClient::extractCodeFromResponse(const QString &responseContent) {
  QString content = responseContent.trimmed();

  // 1. Try to find the first complete markdown code block (```...```)
  int startIdx = content.indexOf("```");
  if (startIdx != -1) {
    int endIdx = content.lastIndexOf("```");
    if (endIdx > startIdx) {
      // Extract what's BETWEEN the triple backticks
      content = content.mid(startIdx + 3, endIdx - startIdx - 3);
    } else {
      // Only one set of backticks? Strip them and continue
      content = content.mid(startIdx + 3);
    }
  }

  // 2. Remove language tag if present
  QString trimmed = content.trimmed();
  if (trimmed.toLower().startsWith("python")) {
    if (trimmed.contains('\n')) {
      content = trimmed.section('\n', 1);
    } else {
      content = trimmed.mid(6);
    }
  } else if (trimmed.toLower().startsWith("json")) {
    if (trimmed.contains('\n')) {
      content = trimmed.section('\n', 1);
    } else {
      content = trimmed.mid(4);
    }
  }

  // 3. Final cleanup
  content = content.trimmed();
  if (content.endsWith("```")) {
    content.chop(3);
  }
  content = content.trimmed();

  // 4. Check if it is JSON (starts with { and ends with })
  if (content.startsWith("{") && content.endsWith("}")) {
    // Wrap it in the python generator logic
    QString jsonConfig = content;
    // Escape quotes in JSON just in case, though triple quotes usually handle
    // it Actually, triple quotes """ ... """ are safe for " unless it contains
    // """

    QString wrapper = R"(
import sys
import os
import json
import cadquery as cq

# Ensure generic generator script is found
# We know where run_cq.py is (usually scripts/cadquery), and machine_generator.py is there too.
# But we are running a temp script. 
# Let's add the hardcoded path for safety in this environment
generator_path = "c:/Projects/opencadandsimulation/opencad/scripts/cadquery"
if generator_path not in sys.path:
    sys.path.append(generator_path)

try:
    import json_to_dsl
except ImportError:
    print("Error: json_to_dsl.py not found in " + generator_path)
    raise

config_json = r"""
%1
"""

try:
    # Parse the JSON from the AI
    config = json.loads(config_json)
    
    # Use the bridge to generate CadQuery object
    result = json_to_dsl.build(config)
    
    if result is None:
        raise Exception("json_to_dsl returned None")

except Exception as e:
    print(f"Generator Error: {e}")
    raise
)";
    return wrapper.arg(jsonConfig);
  }

  return content;
}

} // namespace ai
} // namespace opencad
