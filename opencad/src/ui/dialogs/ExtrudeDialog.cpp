/**
 * @file ExtrudeDialog.cpp
 * @brief Dialog for extrude feature parameters
 */

#include "ExtrudeDialog.h"
#include <QFormLayout>
#include <QHBoxLayout>

namespace opencad {
namespace ui {

ExtrudeDialog::ExtrudeDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("Extrude");
  setMinimumWidth(320);
  setupUI();
}

void ExtrudeDialog::setupUI() {
  auto *mainLayout = new QVBoxLayout(this);

  // === Basic Parameters Group ===
  auto *basicGroup = new QGroupBox("Basic Parameters");
  auto *basicLayout = new QFormLayout(basicGroup);

  // Depth
  m_depthSpin = new QDoubleSpinBox();
  m_depthSpin->setRange(0.1, 10000.0);
  m_depthSpin->setValue(10.0);
  m_depthSpin->setDecimals(2);
  m_depthSpin->setSuffix(" mm");
  basicLayout->addRow("Depth:", m_depthSpin);

  // Direction
  m_directionCombo = new QComboBox();
  m_directionCombo->addItem("Blind (Single Direction)");
  m_directionCombo->addItem("Mid Plane (Symmetric)");
  m_directionCombo->addItem("Both Directions");
  basicLayout->addRow("Direction:", m_directionCombo);
  connect(m_directionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &ExtrudeDialog::onDirectionChanged);

  // Depth 2 (for Both Directions)
  m_depth2Spin = new QDoubleSpinBox();
  m_depth2Spin->setRange(0.1, 10000.0);
  m_depth2Spin->setValue(10.0);
  m_depth2Spin->setDecimals(2);
  m_depth2Spin->setSuffix(" mm");
  m_depth2Spin->setEnabled(false);
  basicLayout->addRow("Depth 2:", m_depth2Spin);

  // Reversed
  m_reversedCheck = new QCheckBox("Reverse Direction");
  basicLayout->addRow("", m_reversedCheck);

  m_symmetricCheck = new QCheckBox(); // Hidden, controlled by combo
  m_symmetricCheck->setVisible(false);

  mainLayout->addWidget(basicGroup);

  // === Draft Group ===
  auto *draftGroup = new QGroupBox("Draft");
  auto *draftLayout = new QFormLayout(draftGroup);

  m_draftAngleSpin = new QDoubleSpinBox();
  m_draftAngleSpin->setRange(-45.0, 45.0);
  m_draftAngleSpin->setValue(0.0);
  m_draftAngleSpin->setDecimals(2);
  m_draftAngleSpin->setSuffix(" °");
  draftLayout->addRow("Draft Angle:", m_draftAngleSpin);

  m_draftOutwardCheck = new QCheckBox("Outward");
  m_draftOutwardCheck->setChecked(true);
  draftLayout->addRow("", m_draftOutwardCheck);

  mainLayout->addWidget(draftGroup);

  // === Thin Feature Group ===
  auto *thinGroup = new QGroupBox("Thin Feature");
  auto *thinLayout = new QFormLayout(thinGroup);

  m_thinFeatureCheck = new QCheckBox("Enable Thin Feature");
  thinLayout->addRow("", m_thinFeatureCheck);
  connect(m_thinFeatureCheck, &QCheckBox::toggled, this,
          &ExtrudeDialog::onThinFeatureToggled);

  m_thinThicknessSpin = new QDoubleSpinBox();
  m_thinThicknessSpin->setRange(0.1, 1000.0);
  m_thinThicknessSpin->setValue(1.0);
  m_thinThicknessSpin->setDecimals(2);
  m_thinThicknessSpin->setSuffix(" mm");
  m_thinThicknessSpin->setEnabled(false);
  m_thinLabel = new QLabel("Thickness:");
  m_thinLabel->setEnabled(false);
  thinLayout->addRow(m_thinLabel, m_thinThicknessSpin);

  mainLayout->addWidget(thinGroup);

  // === Buttons ===
  auto *buttons =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  mainLayout->addWidget(buttons);
}

void ExtrudeDialog::onDirectionChanged(int index) {
  switch (index) {
  case 0: // Blind
    m_depth2Spin->setEnabled(false);
    m_symmetricCheck->setChecked(false);
    break;
  case 1: // Mid Plane
    m_depth2Spin->setEnabled(false);
    m_symmetricCheck->setChecked(true);
    break;
  case 2: // Both Directions
    m_depth2Spin->setEnabled(true);
    m_symmetricCheck->setChecked(false);
    break;
  }
}

void ExtrudeDialog::onThinFeatureToggled(bool checked) {
  m_thinThicknessSpin->setEnabled(checked);
  m_thinLabel->setEnabled(checked);
}

ExtrudeDialogParams ExtrudeDialog::params() const {
  ExtrudeDialogParams p;
  p.depth = m_depthSpin->value();
  p.depth2 = m_depth2Spin->value();
  p.symmetric = m_symmetricCheck->isChecked();
  p.reversed = m_reversedCheck->isChecked();
  p.thinFeature = m_thinFeatureCheck->isChecked();
  p.thinThickness = m_thinThicknessSpin->value();

  // Apply draft direction
  double draft = m_draftAngleSpin->value();
  if (!m_draftOutwardCheck->isChecked()) {
    draft = -draft;
  }
  p.draftAngle = draft;

  return p;
}

} // namespace ui
} // namespace opencad
