/**
 * @file ChamferDialog.cpp
 * @brief Chamfer dialog implementation
 */

#include "ChamferDialog.h"

namespace opencad {
namespace ui {

ChamferDialog::ChamferDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Chamfer");
    setMinimumWidth(320);
    
    auto* layout = new QVBoxLayout(this);
    
    // Mode selection
    auto* modeGroup = new QGroupBox("Chamfer Type");
    auto* modeLayout = new QVBoxLayout(modeGroup);
    
    m_symmetricRadio = new QRadioButton("Symmetric (equal distances)");
    m_asymmetricRadio = new QRadioButton("Asymmetric (two distances)");
    m_angleRadio = new QRadioButton("Distance + Angle");
    m_symmetricRadio->setChecked(true);
    
    modeLayout->addWidget(m_symmetricRadio);
    modeLayout->addWidget(m_asymmetricRadio);
    modeLayout->addWidget(m_angleRadio);
    layout->addWidget(modeGroup);
    
    connect(m_symmetricRadio, &QRadioButton::toggled, this, &ChamferDialog::onModeChanged);
    connect(m_asymmetricRadio, &QRadioButton::toggled, this, &ChamferDialog::onModeChanged);
    connect(m_angleRadio, &QRadioButton::toggled, this, &ChamferDialog::onModeChanged);
    
    // Parameters
    auto* paramGroup = new QGroupBox("Parameters");
    auto* paramLayout = new QVBoxLayout(paramGroup);
    
    // Distance 1
    auto* dist1Row = new QHBoxLayout();
    dist1Row->addWidget(new QLabel("Distance 1:"));
    m_distance1Spin = new QDoubleSpinBox();
    m_distance1Spin->setRange(0.1, 1000.0);
    m_distance1Spin->setValue(1.0);
    m_distance1Spin->setSuffix(" mm");
    dist1Row->addWidget(m_distance1Spin);
    paramLayout->addLayout(dist1Row);
    
    // Distance 2
    auto* dist2Row = new QHBoxLayout();
    m_distance2Label = new QLabel("Distance 2:");
    dist2Row->addWidget(m_distance2Label);
    m_distance2Spin = new QDoubleSpinBox();
    m_distance2Spin->setRange(0.1, 1000.0);
    m_distance2Spin->setValue(1.0);
    m_distance2Spin->setSuffix(" mm");
    dist2Row->addWidget(m_distance2Spin);
    paramLayout->addLayout(dist2Row);
    
    // Angle
    auto* angleRow = new QHBoxLayout();
    m_angleLabel = new QLabel("Angle:");
    angleRow->addWidget(m_angleLabel);
    m_angleSpin = new QDoubleSpinBox();
    m_angleSpin->setRange(1.0, 89.0);
    m_angleSpin->setValue(45.0);
    m_angleSpin->setSuffix("°");
    angleRow->addWidget(m_angleSpin);
    paramLayout->addLayout(angleRow);
    
    layout->addWidget(paramGroup);
    
    // Initial state
    onModeChanged();
    
    // Buttons
    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void ChamferDialog::onModeChanged() {
    bool symmetric = m_symmetricRadio->isChecked();
    bool asymmetric = m_asymmetricRadio->isChecked();
    bool angle = m_angleRadio->isChecked();
    
    m_distance2Label->setVisible(asymmetric);
    m_distance2Spin->setVisible(asymmetric);
    m_angleLabel->setVisible(angle);
    m_angleSpin->setVisible(angle);
}

} // namespace ui
} // namespace opencad
