#include "AIChatPanel.h"
#include <QDateTime>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollBar>

namespace opencad {
namespace ui {

AIChatPanel::AIChatPanel(QWidget *parent) : QWidget(parent) { setupUI(); }

void AIChatPanel::setupUI() {
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(4, 4, 4, 4);
  layout->setSpacing(4);

  // Chat History
  m_chatHistory = new QTextBrowser(this);
  m_chatHistory->setOpenExternalLinks(true);
  m_chatHistory->setStyleSheet("background-color: #1e1e1e; color: #d4d4d4; "
                               "border: none; font-size: 13px;");
  layout->addWidget(m_chatHistory);

  // Status Label
  m_statusLabel = new QLabel("Ready", this);
  m_statusLabel->setStyleSheet(
      "color: #888; font-style: italic; font-size: 11px;");
  layout->addWidget(m_statusLabel);

  // Input Area
  auto *inputLayout = new QHBoxLayout();
  m_inputField = new QLineEdit(this);
  m_inputField->setPlaceholderText("Ask AI to design something...");
  m_inputField->setStyleSheet("background-color: #252526; color: #eee; border: "
                              "1px solid #3e3e42; padding: 4px;");

  m_sendButton = new QPushButton("Send", this);
  m_sendButton->setStyleSheet("background-color: #0e639c; color: white; "
                              "border: none; padding: 5px 10px;");

  inputLayout->addWidget(m_inputField);
  inputLayout->addWidget(m_sendButton);
  layout->addLayout(inputLayout);

  // Connect signals
  connect(m_sendButton, &QPushButton::clicked, this, &AIChatPanel::onSend);
  connect(m_inputField, &QLineEdit::returnPressed, this, &AIChatPanel::onSend);

  // Initial message
  addMessage(
      "System",
      "AI Agent initialized. You can ask me to generate parametric 3D models.");
}

void AIChatPanel::addMessage(const QString &sender, const QString &message,
                             bool isUser) {
  QString color =
      isUser ? "#4ec9b0" : "#ce9178"; // Blue-ish for user, Orange-ish for AI
  QString timestamp = QDateTime::currentDateTime().toString("HH:mm");

  QString html =
      QString("<div style='margin-bottom: 8px;'>"
              "<span style='color: #555; font-size: 10px;'>[%1]</span> "
              "<b style='color: %2;'>%3:</b><br>"
              "<span style='white-space: pre-wrap;'>%4</span>"
              "</div>")
          .arg(timestamp)
          .arg(color)
          .arg(sender)
          .arg(message.toHtmlEscaped().replace("\n", "<br>"));

  m_chatHistory->append(html);
  m_chatHistory->verticalScrollBar()->setValue(
      m_chatHistory->verticalScrollBar()->maximum());
}

void AIChatPanel::setStatus(const QString &status) {
  m_statusLabel->setText(status);
}

void AIChatPanel::onSend() {
  QString text = m_inputField->text().trimmed();
  if (text.isEmpty())
    return;

  addMessage("User", text, true);
  m_inputField->clear();
  setStatus("Thinking...");

  emit promptSubmitted(text);
}

} // namespace ui
} // namespace opencad
