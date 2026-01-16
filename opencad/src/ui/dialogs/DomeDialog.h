/**
 * @file DomeDialog.h
 * @brief Dialog for dome feature parameters
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
 * @enum DomeType
 * @brief Type of dome geometry
 */
enum class DomeType { Spherical, Elliptical };

/**
 * @struct DomeDialogParams
 * @brief Parameters returned from DomeDialog
 */
struct DomeDialogParams {
  double height = 10.0;
  DomeType type = DomeType::Spherical;
  double ellipticRatio = 1.0;
  bool reversed = false;
};

class DomeDialog : public QDialog {
  Q_OBJECT

public:
  explicit DomeDialog(QWidget *parent = nullptr);

  DomeDialogParams params() const;

  double height() const { return m_heightSpin->value(); }
  DomeType type() const;
  double ellipticRatio() const { return m_ellipticRatioSpin->value(); }
  bool isReversed() const { return m_reversedCheck->isChecked(); }

private slots:
  void onTypeChanged(int index);

private:
  void setupUI();

  QDoubleSpinBox *m_heightSpin;
  QComboBox *m_typeCombo;
  QDoubleSpinBox *m_ellipticRatioSpin;
  QLabel *m_ellipticLabel;
  QCheckBox *m_reversedCheck;
};

} // namespace ui
} // namespace opencad
