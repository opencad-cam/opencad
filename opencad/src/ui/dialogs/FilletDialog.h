/**
 * @file FilletDialog.h
 * @brief Dialog for fillet feature parameters with edge selection
 */

#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>


namespace opencad {
namespace ui {

class FilletDialog : public QDialog {
  Q_OBJECT

public:
  explicit FilletDialog(QWidget *parent = nullptr, int totalEdges = 0);

  double radius() const { return m_radiusSpin->value(); }

  // Returns edge indices (0-based), empty means all edges
  std::vector<int> selectedEdgeIndices() const;

  bool applyToAllEdges() const;

private:
  QDoubleSpinBox *m_radiusSpin;
  QLineEdit *m_edgeIndicesEdit;
  int m_totalEdges;
};

} // namespace ui
} // namespace opencad
