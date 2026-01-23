/**
 * @file ProfileSelectionPanel.h
 * @brief Panel for profile selection during Extrude/Cut/Revolve operations
 *
 * OpenCAD - Modular CAD/CAE Platform
 */

#pragma once

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

namespace opencad {
namespace ui {

/**
 * @brief Panel for selecting closed profiles from sketch
 */
class ProfileSelectionPanel : public QWidget {
  Q_OBJECT

public:
  explicit ProfileSelectionPanel(QWidget *parent = nullptr);
  ~ProfileSelectionPanel() override = default;

  /// Update the list of available profiles
  void updateProfileList(const QStringList &profiles);

  /// Set the currently selected profile index
  void setProfileIndex(int index);

  /// Get the currently selected profile index (-1 if none)
  int selectedProfile() const;

  /// Clear profile selection
  void clearSelection();

  /// Set operation title (e.g. "Extrude", "Cut")
  void setOperationTitle(const QString &title);

signals:
  /// Emitted when user selects a profile from combo
  void profileSelected(int index);

  /// Emitted when user clicks Apply
  void applyClicked();

  /// Emitted when user clicks Cancel
  void cancelClicked();

private:
  void setupUI();

  QLabel *m_titleLabel;
  QLabel *m_descriptionLabel;
  QComboBox *m_profileCombo;
  QPushButton *m_applyButton;
  QPushButton *m_cancelButton;
};

} // namespace ui
} // namespace opencad
