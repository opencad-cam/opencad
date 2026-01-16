/**
 * @file MirrorDialog.cpp
 * @brief Mirror dialog implementation
 */

#include "MirrorDialog.h"

namespace opencad {
namespace ui {

MirrorDialog::MirrorDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Mirror");
    setMinimumWidth(280);
    
    auto* layout = new QVBoxLayout(this);
    
    // Plane selection
    auto* planeGroup = new QGroupBox("Mirror Plane");
    auto* planeLayout = new QVBoxLayout(planeGroup);
    
    m_xyRadio = new QRadioButton("XY Plane (Front)");
    m_xzRadio = new QRadioButton("XZ Plane (Top)");
    m_yzRadio = new QRadioButton("YZ Plane (Right)");
    m_yzRadio->setChecked(true);
    
    planeLayout->addWidget(m_xyRadio);
    planeLayout->addWidget(m_xzRadio);
    planeLayout->addWidget(m_yzRadio);
    layout->addWidget(planeGroup);
    
    // Options
    auto* optGroup = new QGroupBox("Options");
    auto* optLayout = new QVBoxLayout(optGroup);
    
    m_keepOriginalCheck = new QCheckBox("Keep original (fuse with mirror)");
    m_keepOriginalCheck->setChecked(true);
    optLayout->addWidget(m_keepOriginalCheck);
    layout->addWidget(optGroup);
    
    // Buttons
    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

MirrorDialog::MirrorPlane MirrorDialog::selectedPlane() const {
    if (m_xyRadio->isChecked()) return MirrorPlane::XY;
    if (m_xzRadio->isChecked()) return MirrorPlane::XZ;
    return MirrorPlane::YZ;
}

} // namespace ui
} // namespace opencad
