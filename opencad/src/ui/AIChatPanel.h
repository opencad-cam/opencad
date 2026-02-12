#pragma once

#include <QLabel> // Added
#include <QLineEdit>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QWidget>

namespace opencad {
namespace ui {

class AIChatPanel : public QWidget {
  Q_OBJECT

public:
  explicit AIChatPanel(QWidget *parent = nullptr);
  ~AIChatPanel() override = default;

  // Add a message to the chat history
  void addMessage(const QString &sender, const QString &message,
                  bool isUser = false);

  // Set status (e.g. "Thinking...", "Ready")
  void setStatus(const QString &status);

signals:
  void promptSubmitted(const QString &prompt);

private:
  void setupUI();
  void onSend();

  QTextBrowser *m_chatHistory;
  QLineEdit *m_inputField;
  QPushButton *m_sendButton;
  QLabel *m_statusLabel;
};

} // namespace ui
} // namespace opencad
