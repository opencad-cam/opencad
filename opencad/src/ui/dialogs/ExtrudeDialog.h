/**
 * @file ExtrudeDialog.h
 * @brief Dialog for extrude feature parameters
 */

#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>


namespace opencad {
namespace ui {

/**
 * @struct ExtrudeDialogParams
 * @brief Parameters returned from ExtrudeDialog
 */
struct ExtrudeDialogParams {
  double depth = 10.0;
  double depth2 = 0.0;
  double draftAngle = 0.0;
  bool symmetric = false; // Mid-plane
  bool reversed = false;
  bool thinFeature = false;
  double thinThickness = 1.0;
};

class ExtrudeDialog : public QDialog {
  Q_OBJECT

public:
  explicit ExtrudeDialog(QWidget *parent = nullptr);

  // Get all parameters
  ExtrudeDialogParams params() const;

  // Individual getters for convenience
  double depth() const { return m_depthSpin->value(); }
  double depth2() const { return m_depth2Spin->value(); }
  double draftAngle() const { return m_draftAngleSpin->value(); }
  bool isSymmetric() const { return m_symmetricCheck->isChecked(); }
  bool isReversed() const { return m_reversedCheck->isChecked(); }
  bool isThinFeature() const { return m_thinFeatureCheck->isChecked(); }
  double thinThickness() const { return m_thinThicknessSpin->value(); }

private slots:
  void onDirectionChanged(int index);
  void onThinFeatureToggled(bool checked);

private:
  void setupUI();

  // Basic parameters
  QDoubleSpinBox *m_depthSpin;
  QDoubleSpinBox *m_depth2Spin;
  QComboBox *m_directionCombo;
  QCheckBox *m_symmetricCheck;
  QCheckBox *m_reversedCheck;

  // Draft
  QDoubleSpinBox *m_draftAngleSpin;
  QCheckBox *m_draftOutwardCheck;

  // Thin feature
  QCheckBox *m_thinFeatureCheck;
  QDoubleSpinBox *m_thinThicknessSpin;
  QLabel *m_thinLabel;
};

} // namespace ui
} // namespace opencad
