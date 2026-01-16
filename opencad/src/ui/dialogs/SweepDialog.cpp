/**
 * @file SweepDialog.cpp
 * @brief Sweep dialog implementation
 */

#include "SweepDialog.h"
#include <QHBoxLayout>

namespace opencad {
namespace ui {

SweepDialog::SweepDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle("Sweep");
  setMinimumWidth(350);

  auto *layout = new QVBoxLayout(this);

  // Info
  auto *infoLabel = new QLabel(
      "Sweep creates a solid by sweeping a profile along a path.\n"
      "Select the profile sketch and path sketch from your existing sketches.");
  infoLabel->setWordWrap(true);
  layout->addWidget(infoLabel);

  // Profile selection
  auto *profileGroup = new QGroupBox("Profile (Cross Section)");
  auto *profileLayout = new QVBoxLayout(profileGroup);

  auto *profileRow = new QHBoxLayout();
  profileRow->addWidget(new QLabel("Sketch:"));
  m_profileCombo = new QComboBox();
  m_profileCombo->setMinimumWidth(200);
  profileRow->addWidget(m_profileCombo);
  profileLayout->addLayout(profileRow);

  layout->addWidget(profileGroup);

  // Path selection
  auto *pathGroup = new QGroupBox("Path (Sweep Direction)");
  auto *pathLayout = new QVBoxLayout(pathGroup);

  auto *pathRow = new QHBoxLayout();
  pathRow->addWidget(new QLabel("Sketch:"));
  m_pathCombo = new QComboBox();
  m_pathCombo->setMinimumWidth(200);
  pathRow->addWidget(m_pathCombo);
  pathLayout->addLayout(pathRow);

  layout->addWidget(pathGroup);

  // Options
  auto *optGroup = new QGroupBox("Options");
  auto *optLayout = new QVBoxLayout(optGroup);

  m_solidCheck = new QCheckBox("Create solid (uncheck for surface)");
  m_solidCheck->setChecked(true);
  optLayout->addWidget(m_solidCheck);

  m_closedPathCheck = new QCheckBox("Closed path (path forms a loop)");
  m_closedPathCheck->setChecked(false);
  m_closedPathCheck->setToolTip(
      "Enable for sweep along circular or closed curves");
  optLayout->addWidget(m_closedPathCheck);

  layout->addWidget(optGroup);

  // Buttons
  auto *buttons =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  layout->addWidget(buttons);
}

void SweepDialog::setAvailableSketches(const QStringList &sketches) {
  m_profileCombo->clear();
  m_pathCombo->clear();
  m_profileCombo->addItems(sketches);
  m_pathCombo->addItems(sketches);
}

} // namespace ui
} // namespace opencad
