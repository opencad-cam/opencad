#pragma once

#include "assembly/AssemblyConstraint.h"
#include "assembly/Component.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QLabel>
#include <memory>
#include <vector>

class QDialogButtonBox;

namespace opencad {
namespace ui {

class MateDialog : public QDialog {
  Q_OBJECT

public:
  explicit MateDialog(
      const std::vector<std::shared_ptr<assembly::Component>> &selection,
      QWidget *parent = nullptr);
  ~MateDialog() override;

  std::shared_ptr<assembly::AssemblyConstraint> getConstraint() const;

signals:
  void previewRequested();

private:
  void setupUi();
  void updateUiForType();
  void onPreviewClicked();

  std::vector<std::shared_ptr<assembly::Component>> m_selection;

  QComboBox *m_typeCombo;
  QDoubleSpinBox *m_distanceSpin;
  QCheckBox *m_flipCheck;
  QDialogButtonBox *m_buttonBox;

  // New fields
  QDoubleSpinBox *m_ratioSpin;
  QDoubleSpinBox *m_pitchSpin;
  QLabel *m_ratioLabel;
  QLabel *m_pitchLabel;
};

} // namespace ui
} // namespace opencad
