#include "AIPanel.h"

namespace opencad {
namespace ui {

AIPanel::AIPanel(QWidget *parent) : QDockWidget("AI Control", parent) {
  setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);

  QWidget *content = new QWidget(this);
  QVBoxLayout *layout = new QVBoxLayout(content);

  // Prompt Input
  m_promptInput = new QLineEdit(content);
  m_promptInput->setPlaceholderText("Enter AI command (e.g., 'segment all')...");
  layout->addWidget(m_promptInput);

  // Run Button
  m_runButton = new QPushButton("Run AI Segmentation", content);
  layout->addWidget(m_runButton);

  // Log Output
  m_logOutput = new QTextEdit(content);
  m_logOutput->setReadOnly(true);
  layout->addWidget(m_logOutput);

  setWidget(content);

  connect(m_runButton, &QPushButton::clicked, this, &AIPanel::onRunClicked);
  connect(m_promptInput, &QLineEdit::returnPressed, this, &AIPanel::onRunClicked);
}

AIPanel::~AIPanel() {}

void AIPanel::appendLog(const QString &message) {
  m_logOutput->append(message);
}

void AIPanel::clearLog() {
  m_logOutput->clear();
}

void AIPanel::onRunClicked() {
  QString prompt = m_promptInput->text();
  emit runRequested(prompt);
}

} // namespace ui
} // namespace opencad
