#include "MateDialog.h"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace opencad {
namespace ui {

MateDialog::MateDialog(
    const std::vector<std::shared_ptr<assembly::Component>> &selection,
    QWidget *parent)
    : QDialog(parent), m_selection(selection) {
  setupUi();
  setWindowTitle("Add Mate");
}

MateDialog::~MateDialog() = default;

void MateDialog::setupUi() {
  auto layout = new QVBoxLayout(this);
  auto formLayout = new QFormLayout();

  // Type Selector
  m_typeCombo = new QComboBox(this);
  m_typeCombo->addItem("Coincident",
                       static_cast<int>(assembly::ConstraintType::Coincident));
  m_typeCombo->addItem("Distance",
                       static_cast<int>(assembly::ConstraintType::Distance));
  m_typeCombo->addItem("Angle",
                       static_cast<int>(assembly::ConstraintType::Angle));
  m_typeCombo->addItem("Parallel",
                       static_cast<int>(assembly::ConstraintType::Parallel));
  m_typeCombo->addItem(
      "Perpendicular",
      static_cast<int>(assembly::ConstraintType::Perpendicular));

  // Distance/Angle Value
  m_distanceSpin = new QDoubleSpinBox(this);
  m_distanceSpin->setRange(-1000.0, 1000.0);
  m_distanceSpin->setValue(0.0);
  m_distanceSpin->setVisible(false); // Hidden for Coincident by default

  // Flip Alignment
  m_flipCheck = new QCheckBox("Flip Alignment", this);

  formLayout->addRow("Mate Type:", m_typeCombo);
  formLayout->addRow("Distance/Angle:", m_distanceSpin);
  formLayout->addRow("", m_flipCheck);

  layout->addLayout(formLayout);

  // Selection details
  QString selText = QString("Selected: %1 components").arg(m_selection.size());
  layout->addWidget(new QLabel(selText, this));

  // Buttons
  m_buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
  connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  layout->addWidget(m_buttonBox);

  // Dynamic UI updates
  connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &MateDialog::updateUiForType);
}

void MateDialog::updateUiForType() {
  auto type =
      static_cast<assembly::ConstraintType>(m_typeCombo->currentData().toInt());
  bool showValue = (type == assembly::ConstraintType::Distance ||
                    type == assembly::ConstraintType::Angle);
  m_distanceSpin->setVisible(showValue);
}

std::shared_ptr<assembly::AssemblyConstraint>
MateDialog::getConstraint() const {
  if (m_selection.size() < 2)
    return nullptr;

  auto type =
      static_cast<assembly::ConstraintType>(m_typeCombo->currentData().toInt());

  // Create constraint logic here - basic placeholder for now since
  // AssemblyConstraint ctor is simple In a real impl, we'd pass the value
  // (distance) to the constraint

  // Assuming AssemblyConstraint constructor takes (Type, C1, C2)
  auto constraint = std::make_shared<assembly::AssemblyConstraint>(
      type, m_selection[0], m_selection[1]);

  if (type == assembly::ConstraintType::Distance ||
      type == assembly::ConstraintType::Angle) {
    constraint->setValue(m_distanceSpin->value());
  }

  return constraint;
}

} // namespace ui
} // namespace opencad
