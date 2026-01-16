/**
 * @file SweepDialog.h
 * @brief Dialog for sweep feature parameters
 */

#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>


namespace opencad {
namespace ui {

class SweepDialog : public QDialog {
  Q_OBJECT

public:
  explicit SweepDialog(QWidget *parent = nullptr);

  QString profileSketch() const { return m_profileCombo->currentText(); }
  QString pathSketch() const { return m_pathCombo->currentText(); }
  bool solidOutput() const { return m_solidCheck->isChecked(); }
  bool closedPath() const { return m_closedPathCheck->isChecked(); }

  void setAvailableSketches(const QStringList &sketches);

private:
  QComboBox *m_profileCombo;
  QComboBox *m_pathCombo;
  QCheckBox *m_solidCheck;
  QCheckBox *m_closedPathCheck;
};

} // namespace ui
} // namespace opencad
