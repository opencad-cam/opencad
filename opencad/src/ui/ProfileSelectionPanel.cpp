/**
 * @file ProfileSelectionPanel.cpp
 * @brief Implementation of ProfileSelectionPanel
 *
 * OpenCAD - Modular CAD/CAE Platform
 */

#include "ProfileSelectionPanel.h"
#include <QDebug>

namespace opencad {
namespace ui {

ProfileSelectionPanel::ProfileSelectionPanel(QWidget *parent)
    : QWidget(parent) {
  setupUI();
}

void ProfileSelectionPanel::setupUI() {
  auto *layout = new QVBoxLayout(this);
  layout->setContentsMargins(10, 10, 10, 10);
  layout->setSpacing(10);

  // Title
  m_titleLabel = new QLabel("📐 Profile Selection", this);
  m_titleLabel->setStyleSheet(
      "QLabel { font-size: 14px; font-weight: bold; color: #333; }");
  layout->addWidget(m_titleLabel);

  // Description
  m_descriptionLabel =
      new QLabel("Select a closed profile from the sketch.", this);
  m_descriptionLabel->setWordWrap(true);
  m_descriptionLabel->setStyleSheet("QLabel { color: #666; }");
  layout->addWidget(m_descriptionLabel);

  // Profile combo
  layout->addWidget(new QLabel("Profile:", this));
  m_profileCombo = new QComboBox(this);
  m_profileCombo->addItem("No profiles available");
  m_profileCombo->setEnabled(false);
  layout->addWidget(m_profileCombo);

  // Spacer
  layout->addStretch();

  // Buttons
  auto *buttonLayout = new QHBoxLayout();

  m_cancelButton = new QPushButton("Cancel", this);
  m_cancelButton->setStyleSheet(
      "QPushButton { padding: 8px 16px; border-radius: 4px; }");
  buttonLayout->addWidget(m_cancelButton);

  m_applyButton = new QPushButton("Apply (Enter)", this);
  m_applyButton->setStyleSheet(
      "QPushButton { background-color: #4CAF50; color: white; padding: 8px "
      "16px; font-weight: bold; border-radius: 4px; } "
      "QPushButton:hover { background-color: #45a049; }");
  m_applyButton->setDefault(true);
  buttonLayout->addWidget(m_applyButton);

  layout->addLayout(buttonLayout);

  // Connections
  connect(m_profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, [this](int index) {
            if (m_profileCombo->isEnabled() && index >= 0) {
              emit profileSelected(index);
            }
          });

  connect(m_applyButton, &QPushButton::clicked, this, [this]() {
    qDebug() << "ProfileSelectionPanel: Apply clicked";
    emit applyClicked();
  });

  connect(m_cancelButton, &QPushButton::clicked, this, [this]() {
    qDebug() << "ProfileSelectionPanel: Cancel clicked";
    emit cancelClicked();
  });
}

void ProfileSelectionPanel::updateProfileList(const QStringList &profiles) {
  m_profileCombo->clear();
  if (profiles.isEmpty()) {
    m_profileCombo->addItem("No profiles available");
    m_profileCombo->setEnabled(false);
  } else {
    for (int i = 0; i < profiles.size(); ++i) {
      m_profileCombo->addItem(QString("Profile %1").arg(i + 1));
    }
    m_profileCombo->setEnabled(true);
    // Auto-select first profile
    m_profileCombo->setCurrentIndex(0);
  }
}

void ProfileSelectionPanel::setProfileIndex(int index) {
  if (m_profileCombo->isEnabled() && index >= 0 &&
      index < m_profileCombo->count()) {
    m_profileCombo->setCurrentIndex(index);
  }
}

int ProfileSelectionPanel::selectedProfile() const {
  return m_profileCombo->isEnabled() ? m_profileCombo->currentIndex() : -1;
}

void ProfileSelectionPanel::clearSelection() {
  m_profileCombo->clear();
  m_profileCombo->addItem("No profiles available");
  m_profileCombo->setEnabled(false);
}

void ProfileSelectionPanel::setOperationTitle(const QString &title) {
  m_titleLabel->setText(QString("📐 %1 - Profile Selection").arg(title));
}

} // namespace ui
} // namespace opencad
