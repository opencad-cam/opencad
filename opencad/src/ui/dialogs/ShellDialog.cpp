/**
 * @file ShellDialog.cpp
 * @brief Shell dialog implementation
 */

#include "ShellDialog.h"

namespace opencad {
namespace ui {

ShellDialog::ShellDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Shell");
    setMinimumWidth(300);
    
    auto* layout = new QVBoxLayout(this);
    
    // Info
    auto* infoLabel = new QLabel("Select faces to remove (open), then set thickness:");
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);
    
    // Parameters
    auto* paramGroup = new QGroupBox("Parameters");
    auto* paramLayout = new QVBoxLayout(paramGroup);
    
    auto* thicknessRow = new QHBoxLayout();
    thicknessRow->addWidget(new QLabel("Wall Thickness:"));
    m_thicknessSpin = new QDoubleSpinBox();
    m_thicknessSpin->setRange(0.1, 100.0);
    m_thicknessSpin->setValue(2.0);
    m_thicknessSpin->setSuffix(" mm");
    thicknessRow->addWidget(m_thicknessSpin);
    paramLayout->addLayout(thicknessRow);
    
    m_outwardCheck = new QCheckBox("Shell outward");
    paramLayout->addWidget(m_outwardCheck);
    
    layout->addWidget(paramGroup);
    
    // Buttons
    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

} // namespace ui
} // namespace opencad
