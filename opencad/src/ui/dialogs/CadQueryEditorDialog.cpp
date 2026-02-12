#include "CadQueryEditorDialog.h"

#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QTextStream>
#include <QVBoxLayout>


namespace opencad {
namespace ui {

CadQueryEditorDialog::CadQueryEditorDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("CadQuery Editor");
  resize(800, 600);

  auto *mainLayout = new QVBoxLayout(this);

  // Editor Area
  auto *editorLabel = new QLabel("Python/CadQuery Script:", this);
  mainLayout->addWidget(editorLabel);

  m_editor = new QTextEdit(this);
  m_editor->setFont(QFont("Consolas", 10)); // Monospace font
  // Default example
  m_editor->setText(
      "import cadquery as cq\n\nresult = cq.Workplane('XY').box(10, 10, 10)");
  mainLayout->addWidget(m_editor, 2); // Stretch factor 2

  // Console Area
  auto *consoleLabel = new QLabel("Output/Log:", this);
  mainLayout->addWidget(consoleLabel);

  m_console = new QTextEdit(this);
  m_console->setReadOnly(true);
  m_console->setFont(QFont("Consolas", 9));
  mainLayout->addWidget(m_console, 1); // Stretch factor 1

  // Buttons
  auto *btnLayout = new QHBoxLayout();

  auto *loadButton = new QPushButton("Load Script...", this);
  connect(loadButton, &QPushButton::clicked, this,
          &CadQueryEditorDialog::onLoad);
  btnLayout->addWidget(loadButton);

  auto *saveButton = new QPushButton("Save Script...", this);
  connect(saveButton, &QPushButton::clicked, this,
          &CadQueryEditorDialog::onSave);
  btnLayout->addWidget(saveButton);

  auto *clearButton = new QPushButton("Clear Log", this);
  connect(clearButton, &QPushButton::clicked, this,
          &CadQueryEditorDialog::onClear);
  btnLayout->addWidget(clearButton);

  btnLayout->addStretch();

  m_runButton = new QPushButton("Run (Generate Geometry)", this);
  m_runButton->setStyleSheet("font-weight: bold; background-color: #4CAF50; "
                             "color: white; padding: 5px 10px;");
  connect(m_runButton, &QPushButton::clicked, this,
          &CadQueryEditorDialog::onRun);
  btnLayout->addWidget(m_runButton);

  auto *closeButton = new QPushButton("Close", this);
  connect(closeButton, &QPushButton::clicked, this,
          &CadQueryEditorDialog::accept);
  btnLayout->addWidget(closeButton);

  mainLayout->addLayout(btnLayout);
}

CadQueryEditorDialog::~CadQueryEditorDialog() {}

QString CadQueryEditorDialog::getScript() const {
  return m_editor->toPlainText();
}

void CadQueryEditorDialog::setScript(const QString &script) {
  m_editor->setText(script);
}

void CadQueryEditorDialog::appendLog(const QString &message) {
  m_console->append(message);
}

void CadQueryEditorDialog::clearLog() { m_console->clear(); }

void CadQueryEditorDialog::onRun() { emit runRequested(getScript()); }

void CadQueryEditorDialog::onLoad() {
  QString fileName = QFileDialog::getOpenFileName(
      this, "Open Script", "", "Python Files (*.py);;All Files (*)");
  if (fileName.isEmpty())
    return;

  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QMessageBox::warning(this, "Error",
                         "Cannot read file: " + file.errorString());
    return;
  }

  QTextStream in(&file);
  m_editor->setText(in.readAll());
  file.close();
  appendLog("Loaded: " + fileName);
}

void CadQueryEditorDialog::onSave() {
  QString fileName = QFileDialog::getSaveFileName(
      this, "Save Script", "", "Python Files (*.py);;All Files (*)");
  if (fileName.isEmpty())
    return;

  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(this, "Error",
                         "Cannot write file: " + file.errorString());
    return;
  }

  QTextStream out(&file);
  out << m_editor->toPlainText();
  file.close();
  appendLog("Saved: " + fileName);
}

void CadQueryEditorDialog::onClear() { m_console->clear(); }

} // namespace ui
} // namespace opencad
