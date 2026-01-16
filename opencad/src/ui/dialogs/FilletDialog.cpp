/**
 * @file FilletDialog.cpp
 * @brief Fillet dialog implementation with edge selection
 */

#include "FilletDialog.h"
#include <QHBoxLayout>

namespace opencad {
namespace ui {

FilletDialog::FilletDialog(QWidget *parent, int totalEdges)
    : QDialog(parent), m_totalEdges(totalEdges) {
  setWindowTitle("Fillet");
  setMinimumWidth(350);

  auto *layout = new QVBoxLayout(this);

  // Info label
  QString infoText =
      QString("Total edges in shape: %1\n\n"
              "To fillet specific edges, enter edge numbers (1-%1)\n"
              "separated by commas. Leave empty for ALL edges.")
          .arg(totalEdges);
  auto *infoLabel = new QLabel(infoText);
  infoLabel->setWordWrap(true);
  layout->addWidget(infoLabel);

  // Edge indices input
  auto *edgeRow = new QHBoxLayout();
  edgeRow->addWidget(new QLabel("Edge numbers:"));
  m_edgeIndicesEdit = new QLineEdit();
  m_edgeIndicesEdit->setPlaceholderText("e.g. 1,3,5 or leave empty for all");
  edgeRow->addWidget(m_edgeIndicesEdit);
  layout->addLayout(edgeRow);

  // Radius input
  auto *radiusRow = new QHBoxLayout();
  radiusRow->addWidget(new QLabel("Radius:"));
  m_radiusSpin = new QDoubleSpinBox();
  m_radiusSpin->setRange(0.1, 1000.0);
  m_radiusSpin->setValue(2.0);
  m_radiusSpin->setSuffix(" mm");
  m_radiusSpin->setDecimals(2);
  radiusRow->addWidget(m_radiusSpin);
  layout->addLayout(radiusRow);

  // Buttons
  auto *buttons =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
  layout->addWidget(buttons);
}

std::vector<int> FilletDialog::selectedEdgeIndices() const {
  std::vector<int> indices;
  QString text = m_edgeIndicesEdit->text().trimmed();

  if (text.isEmpty()) {
    return indices; // Empty = all edges
  }

  QStringList parts = text.split(',', Qt::SkipEmptyParts);
  for (const QString &part : parts) {
    bool ok;
    int num = part.trimmed().toInt(&ok);
    if (ok && num >= 1 && num <= m_totalEdges) {
      indices.push_back(num - 1); // Convert to 0-based index
    }
  }
  return indices;
}

bool FilletDialog::applyToAllEdges() const {
  return m_edgeIndicesEdit->text().trimmed().isEmpty();
}

} // namespace ui
} // namespace opencad
