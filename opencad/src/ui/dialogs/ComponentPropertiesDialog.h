#pragma once

#include "assembly/Component.h"
#include <QDialog>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <memory>


namespace opencad {
namespace ui {

class ComponentPropertiesDialog : public QDialog {
  Q_OBJECT

public:
  explicit ComponentPropertiesDialog(
      std::shared_ptr<assembly::Component> component,
      QWidget *parent = nullptr);

private slots:
  void onApply();
  void onOk();

private:
  std::shared_ptr<assembly::Component> m_component;

  QLineEdit *m_nameEdit;
  QDoubleSpinBox *m_posX;
  QDoubleSpinBox *m_posY;
  QDoubleSpinBox *m_posZ;
  // Rotation could be Euler or Axis-Angle. For simplicity, let's just do
  // Position first or Euler if easy. Let's stick to Name and Position for now
  // to be safe.
};

} // namespace ui
} // namespace opencad
