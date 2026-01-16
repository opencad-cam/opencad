/**
 * @file SplitDialog.h
 * @brief Dialog for split feature parameters
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
 * @enum SplitPlane
 * @brief Plane to split with
 */
enum class SplitPlane { XY, XZ, YZ };

/**
 * @enum SplitKeepPart
 * @brief Which parts to keep after split
 */
enum class SplitKeepPart { Both, Above, Below };

/**
 * @struct SplitDialogParams
 * @brief Parameters returned from SplitDialog
 */
struct SplitDialogParams {
  SplitPlane plane = SplitPlane::XY;
  double offset = 0.0;
  SplitKeepPart keepPart = SplitKeepPart::Both;
};

class SplitDialog : public QDialog {
  Q_OBJECT

public:
  explicit SplitDialog(QWidget *parent = nullptr);

  SplitDialogParams params() const;

  SplitPlane plane() const;
  double offset() const { return m_offsetSpin->value(); }
  SplitKeepPart keepPart() const;

private:
  void setupUI();

  QComboBox *m_planeCombo;
  QDoubleSpinBox *m_offsetSpin;
  QComboBox *m_keepPartCombo;
};

} // namespace ui
} // namespace opencad
