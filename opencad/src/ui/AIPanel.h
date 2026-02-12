#pragma once

#include <QDockWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

namespace opencad {
namespace ui {

class AIPanel : public QDockWidget {
  Q_OBJECT

public:
  explicit AIPanel(QWidget *parent = nullptr);
  ~AIPanel();

  void appendLog(const QString &message);
  void clearLog();

signals:
  void runRequested(const QString &prompt);

private slots:
  void onRunClicked();

private:
  QLineEdit *m_promptInput;
  QPushButton *m_runButton;
  QTextEdit *m_logOutput;
};

} // namespace ui
} // namespace opencad
