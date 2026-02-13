#include "MateDialog.h"
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
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

void MateDialog::updateUiForType() {
  auto type =
      static_cast<assembly::ConstraintType>(m_typeCombo->currentData().toInt());

  bool showDistAngle = (type == assembly::ConstraintType::Distance ||
                        type == assembly::ConstraintType::Angle);
  m_distanceSpin->setVisible(showDistAngle);
  // Note: The label "Distance/Angle:" will still be visible if we don't hide
  // it. For this MVP, we'll accept the label being there or improve it later.

  bool showGear = (type == assembly::ConstraintType::Gear);
  if (m_ratioSpin)
    m_ratioSpin->setVisible(showGear);
  if (m_ratioLabel)
    m_ratioLabel->setVisible(showGear);

  bool showScrew = (type == assembly::ConstraintType::Screw);
  if (m_pitchSpin)
    m_pitchSpin->setVisible(showScrew);
  if (m_pitchLabel)
    m_pitchLabel->setVisible(showScrew);

  // Flip is valid for most, maybe not Lock?
  m_flipCheck->setVisible(type != assembly::ConstraintType::Lock);
}

std::shared_ptr<assembly::AssemblyConstraint>
MateDialog::getConstraint() const {
  if (m_selection.size() < 2)
    return nullptr;

  auto type =
      static_cast<assembly::ConstraintType>(m_typeCombo->currentData().toInt());

  auto constraint = std::make_shared<assembly::AssemblyConstraint>(
      type, m_selection[0], m_selection[1]);

  if (type == assembly::ConstraintType::Distance ||
      type == assembly::ConstraintType::Angle) {
    constraint->setValue(m_distanceSpin->value());
  } else if (type == assembly::ConstraintType::Gear) {
    constraint->setRatio(m_ratioSpin->value());
  } else if (type == assembly::ConstraintType::Screw) {
    constraint->setPitch(m_pitchSpin->value());
  }

  constraint->setFlipped(m_flipCheck->isChecked());

  return constraint;
}

// Re-implementing setupUi to be safe about labels
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
  m_typeCombo->addItem("Concentric",
                       static_cast<int>(assembly::ConstraintType::Concentric));
  m_typeCombo->addItem("Tangent",
                       static_cast<int>(assembly::ConstraintType::Tangent));
  m_typeCombo->addItem("Lock (Rigid)",
                       static_cast<int>(assembly::ConstraintType::Lock));
  m_typeCombo->addItem("Gear",
                       static_cast<int>(assembly::ConstraintType::Gear));
  m_typeCombo->addItem("Screw",
                       static_cast<int>(assembly::ConstraintType::Screw));

  // Distance/Angle Value
  m_distanceSpin = new QDoubleSpinBox(this);
  m_distanceSpin->setRange(-1000.0, 1000.0);
  m_distanceSpin->setValue(0.0);

  // Ratio
  m_ratioSpin = new QDoubleSpinBox(this);
  m_ratioSpin->setRange(0.001, 1000.0);
  m_ratioSpin->setValue(1.0);
  m_ratioLabel = new QLabel("Ratio:", this);

  // Pitch
  m_pitchSpin = new QDoubleSpinBox(this);
  m_pitchSpin->setRange(0.001, 1000.0);
  m_pitchSpin->setValue(1.0);
  m_pitchLabel = new QLabel("Pitch:", this);

  // Flip
  m_flipCheck = new QCheckBox("Flip Alignment", this);

  formLayout->addRow("Mate Type:", m_typeCombo);
  formLayout->addRow("Distance/Angle:", m_distanceSpin);
  formLayout->addRow(m_ratioLabel, m_ratioSpin);
  formLayout->addRow(m_pitchLabel, m_pitchSpin);
  formLayout->addRow("", m_flipCheck);

  layout->addLayout(formLayout);

  // Selection details
  QString selText = QString("Selected: %1 components").arg(m_selection.size());
  layout->addWidget(new QLabel(selText, this));

  // Buttons
  m_buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply,
      this);
  connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(m_buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
          this, &MateDialog::onPreviewClicked);

  // Set Apply button text to "Preview / Apply"
  m_buttonBox->button(QDialogButtonBox::Apply)->setText("Preview");

  layout->addWidget(m_buttonBox);

  // Dynamic UI updates
  connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &MateDialog::updateUiForType);

  // Initial UI Update
  updateUiForType();
}

void MateDialog::onPreviewClicked() { emit previewRequested(); }

} // namespace ui
} // namespace opencad
