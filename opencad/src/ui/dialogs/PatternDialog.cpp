/**
 * @file PatternDialog.cpp
 * @brief Pattern dialog implementation
 */

#include "PatternDialog.h"
#include <QCheckBox>

namespace opencad {
namespace ui {

PatternDialog::PatternDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Pattern");
    setMinimumWidth(350);
    
    auto* layout = new QVBoxLayout(this);
    
    // Type selection
    auto* typeGroup = new QGroupBox("Pattern Type");
    auto* typeLayout = new QHBoxLayout(typeGroup);
    m_linearRadio = new QRadioButton("Linear");
    m_circularRadio = new QRadioButton("Circular");
    m_linearRadio->setChecked(true);
    typeLayout->addWidget(m_linearRadio);
    typeLayout->addWidget(m_circularRadio);
    layout->addWidget(typeGroup);
    
    connect(m_linearRadio, &QRadioButton::toggled, this, &PatternDialog::onTypeChanged);
    
    // Stacked widget for parameters
    m_stackedWidget = new QStackedWidget();
    
    // Linear page
    auto* linearPage = new QWidget();
    auto* linearLayout = new QVBoxLayout(linearPage);
    
    auto* dirXGroup = new QGroupBox("Direction 1 (X)");
    auto* dirXLayout = new QVBoxLayout(dirXGroup);
    auto* countXRow = new QHBoxLayout();
    countXRow->addWidget(new QLabel("Count:"));
    m_countXSpin = new QSpinBox();
    m_countXSpin->setRange(1, 100);
    m_countXSpin->setValue(3);
    countXRow->addWidget(m_countXSpin);
    dirXLayout->addLayout(countXRow);
    
    auto* spacingXRow = new QHBoxLayout();
    spacingXRow->addWidget(new QLabel("Spacing:"));
    m_spacingXSpin = new QDoubleSpinBox();
    m_spacingXSpin->setRange(0.1, 1000.0);
    m_spacingXSpin->setValue(20.0);
    m_spacingXSpin->setSuffix(" mm");
    spacingXRow->addWidget(m_spacingXSpin);
    dirXLayout->addLayout(spacingXRow);
    linearLayout->addWidget(dirXGroup);
    
    auto* dirYGroup = new QGroupBox("Direction 2 (Y)");
    auto* dirYLayout = new QVBoxLayout(dirYGroup);
    auto* countYRow = new QHBoxLayout();
    countYRow->addWidget(new QLabel("Count:"));
    m_countYSpin = new QSpinBox();
    m_countYSpin->setRange(1, 100);
    m_countYSpin->setValue(1);
    countYRow->addWidget(m_countYSpin);
    dirYLayout->addLayout(countYRow);
    
    auto* spacingYRow = new QHBoxLayout();
    spacingYRow->addWidget(new QLabel("Spacing:"));
    m_spacingYSpin = new QDoubleSpinBox();
    m_spacingYSpin->setRange(0.1, 1000.0);
    m_spacingYSpin->setValue(20.0);
    m_spacingYSpin->setSuffix(" mm");
    spacingYRow->addWidget(m_spacingYSpin);
    dirYLayout->addLayout(spacingYRow);
    linearLayout->addWidget(dirYGroup);
    
    m_stackedWidget->addWidget(linearPage);
    
    // Circular page
    auto* circularPage = new QWidget();
    auto* circularLayout = new QVBoxLayout(circularPage);
    
    auto* circGroup = new QGroupBox("Circular Pattern");
    auto* circGroupLayout = new QVBoxLayout(circGroup);
    
    auto* countRow = new QHBoxLayout();
    countRow->addWidget(new QLabel("Count:"));
    m_circularCountSpin = new QSpinBox();
    m_circularCountSpin->setRange(2, 100);
    m_circularCountSpin->setValue(6);
    countRow->addWidget(m_circularCountSpin);
    circGroupLayout->addLayout(countRow);
    
    auto* angleRow = new QHBoxLayout();
    angleRow->addWidget(new QLabel("Total Angle:"));
    m_totalAngleSpin = new QDoubleSpinBox();
    m_totalAngleSpin->setRange(1.0, 360.0);
    m_totalAngleSpin->setValue(360.0);
    m_totalAngleSpin->setSuffix("°");
    angleRow->addWidget(m_totalAngleSpin);
    circGroupLayout->addLayout(angleRow);
    
    m_equalSpacingCheck = new QCheckBox("Equal spacing");
    m_equalSpacingCheck->setChecked(true);
    circGroupLayout->addWidget(m_equalSpacingCheck);
    
    circularLayout->addWidget(circGroup);
    circularLayout->addStretch();
    
    m_stackedWidget->addWidget(circularPage);
    layout->addWidget(m_stackedWidget);
    
    // Buttons
    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void PatternDialog::onTypeChanged() {
    m_stackedWidget->setCurrentIndex(m_linearRadio->isChecked() ? 0 : 1);
}

} // namespace ui
} // namespace opencad
