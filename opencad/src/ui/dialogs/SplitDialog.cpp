/**
 * @file SplitDialog.cpp
 * @brief Dialog for split feature parameters
 */

#include "SplitDialog.h"
#include <QFormLayout>

namespace opencad {
namespace ui {

SplitDialog::SplitDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("Split");
  setMinimumWidth(300);
  setupUI();
}

void SplitDialog::setupUI() {
  auto *mainLayout = new QVBoxLayout(this);

  // === Split Parameters ===
  auto *splitGroup = new QGroupBox("Split Parameters");
  auto *splitLayout = new QFormLayout(splitGroup);

  // Plane selection
  m_planeCombo = new QComboBox();
  m_planeCombo->addItem("XY Plane (Horizontal)",
                        static_cast<int>(SplitPlane::XY));
  m_planeCombo->addItem("XZ Plane (Front)", static_cast<int>(SplitPlane::XZ));
  m_planeCombo->addItem("YZ Plane (Side)", static_cast<int>(SplitPlane::YZ));
  splitLayout->addRow("Split Plane:", m_planeCombo);

  // Offset
  m_offsetSpin = new QDoubleSpinBox();
  m_offsetSpin->setRange(-10000.0, 10000.0);
  m_offsetSpin->setValue(0.0);
  m_offsetSpin->setDecimals(2);
  m_offsetSpin->setSuffix(" mm");
  splitLayout->addRow("Plane Offset:", m_offsetSpin);

  mainLayout->addWidget(splitGroup);

  // === Result Options ===
  auto *resultGroup = new QGroupBox("Result Options");
  auto *resultLayout = new QFormLayout(resultGroup);

  m_keepPartCombo = new QComboBox();
  m_keepPartCombo->addItem("Keep Both Parts",
                           static_cast<int>(SplitKeepPart::Both));
  m_keepPartCombo->addItem("Keep Above Only",
                           static_cast<int>(SplitKeepPart::Above));
  m_keepPartCombo->addItem("Keep Below Only",
                           static_cast<int>(SplitKeepPart::Below));
  resultLayout->addRow("Keep:", m_keepPartCombo);

  mainLayout->addWidget(resultGroup);

  // === Buttons ===
  auto *buttons =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  mainLayout->addWidget(buttons);
}

SplitPlane SplitDialog::plane() const {
  return static_cast<SplitPlane>(m_planeCombo->currentData().toInt());
}

SplitKeepPart SplitDialog::keepPart() const {
  return static_cast<SplitKeepPart>(m_keepPartCombo->currentData().toInt());
}

SplitDialogParams SplitDialog::params() const {
  SplitDialogParams p;
  p.plane = plane();
  p.offset = offset();
  p.keepPart = keepPart();
  return p;
}

} // namespace ui
} // namespace opencad
