/**
 * @file DomeDialog.cpp
 * @brief Dialog for dome feature parameters
 */

#include "DomeDialog.h"
#include <QFormLayout>

namespace opencad {
namespace ui {

DomeDialog::DomeDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("Dome");
  setMinimumWidth(300);
  setupUI();
}

void DomeDialog::setupUI() {
  auto *mainLayout = new QVBoxLayout(this);

  // === Dome Parameters ===
  auto *domeGroup = new QGroupBox("Dome Parameters");
  auto *domeLayout = new QFormLayout(domeGroup);

  // Height
  m_heightSpin = new QDoubleSpinBox();
  m_heightSpin->setRange(0.1, 10000.0);
  m_heightSpin->setValue(10.0);
  m_heightSpin->setDecimals(2);
  m_heightSpin->setSuffix(" mm");
  domeLayout->addRow("Height:", m_heightSpin);

  // Type
  m_typeCombo = new QComboBox();
  m_typeCombo->addItem("Spherical", static_cast<int>(DomeType::Spherical));
  m_typeCombo->addItem("Elliptical", static_cast<int>(DomeType::Elliptical));
  domeLayout->addRow("Type:", m_typeCombo);
  connect(m_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &DomeDialog::onTypeChanged);

  // Elliptic Ratio (for elliptical dome)
  m_ellipticRatioSpin = new QDoubleSpinBox();
  m_ellipticRatioSpin->setRange(0.1, 10.0);
  m_ellipticRatioSpin->setValue(1.0);
  m_ellipticRatioSpin->setDecimals(2);
  m_ellipticRatioSpin->setEnabled(false);
  m_ellipticLabel = new QLabel("Width/Height Ratio:");
  m_ellipticLabel->setEnabled(false);
  domeLayout->addRow(m_ellipticLabel, m_ellipticRatioSpin);

  mainLayout->addWidget(domeGroup);

  // === Direction ===
  auto *dirGroup = new QGroupBox("Direction");
  auto *dirLayout = new QFormLayout(dirGroup);

  m_reversedCheck = new QCheckBox("Dome Inward (Concave)");
  dirLayout->addRow("", m_reversedCheck);

  mainLayout->addWidget(dirGroup);

  // === Buttons ===
  auto *buttons =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  mainLayout->addWidget(buttons);
}

void DomeDialog::onTypeChanged(int index) {
  bool isElliptical = (index == 1);
  m_ellipticRatioSpin->setEnabled(isElliptical);
  m_ellipticLabel->setEnabled(isElliptical);
}

DomeType DomeDialog::type() const {
  return static_cast<DomeType>(m_typeCombo->currentData().toInt());
}

DomeDialogParams DomeDialog::params() const {
  DomeDialogParams p;
  p.height = height();
  p.type = type();
  p.ellipticRatio = ellipticRatio();
  p.reversed = isReversed();
  return p;
}

} // namespace ui
} // namespace opencad
