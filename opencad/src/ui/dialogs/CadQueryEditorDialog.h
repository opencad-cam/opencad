#pragma once

#include <QDialog>
#include <QPushButton>
#include <QTextEdit>


namespace opencad {
namespace ui {

class CadQueryEditorDialog : public QDialog {
  Q_OBJECT

public:
  explicit CadQueryEditorDialog(QWidget *parent = nullptr);
  ~CadQueryEditorDialog();

  QString getScript() const;
  void setScript(const QString &script);
  void appendLog(const QString &message);
  void clearLog();

signals:
  void runRequested(const QString &script);

private slots:
  void onRun();
  void onLoad();
  void onSave();
  void onClear();

private:
  QTextEdit *m_editor;
  QTextEdit *m_console;
  QPushButton *m_runButton;
};

} // namespace ui
} // namespace opencad
