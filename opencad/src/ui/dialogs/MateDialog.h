#pragma once

#include "assembly/AssemblyConstraint.h"
#include "assembly/Component.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDoubleSpinBox>
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

private:
  void setupUi();
  void updateUiForType();

  std::vector<std::shared_ptr<assembly::Component>> m_selection;

  QComboBox *m_typeCombo;
  QDoubleSpinBox *m_distanceSpin;
  QCheckBox *m_flipCheck;
  QDialogButtonBox *m_buttonBox;
};

} // namespace ui
} // namespace opencad
