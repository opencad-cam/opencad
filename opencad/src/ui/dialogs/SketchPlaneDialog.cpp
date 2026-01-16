/**
 * @file SketchPlaneDialog.cpp
 * @brief Dialog for selecting sketch plane
 */

#include "SketchPlaneDialog.h"

#include <QHBoxLayout>
#include <QGroupBox>
#include <QIcon>

namespace opencad {
namespace ui {

SketchPlaneDialog::SketchPlaneDialog(QWidget* parent, bool hasSolid)
    : QDialog(parent)
{
    setWindowTitle("Select Sketch Plane");
    setModal(true);
    setMinimumWidth(350);
    setupUI(hasSolid);
}

void SketchPlaneDialog::setupUI(bool hasSolid) {
    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Title
    auto* titleLabel = new QLabel("<b>Select a plane for your sketch:</b>");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    // Standard planes group
    auto* planesGroup = new QGroupBox("Standard Planes");
    auto* planesLayout = new QHBoxLayout(planesGroup);
    planesLayout->setSpacing(10);
    
    // XY Plane (Top)
    m_btnXY = new QPushButton("📐 XY\n(Top)");
    m_btnXY->setFixedSize(80, 60);
    m_btnXY->setToolTip("Top plane - Z = 0");
    connect(m_btnXY, &QPushButton::clicked, this, &SketchPlaneDialog::onXYSelected);
    planesLayout->addWidget(m_btnXY);
    
    // XZ Plane (Front)
    m_btnXZ = new QPushButton("📐 XZ\n(Front)");
    m_btnXZ->setFixedSize(80, 60);
    m_btnXZ->setToolTip("Front plane - Y = 0");
    connect(m_btnXZ, &QPushButton::clicked, this, &SketchPlaneDialog::onXZSelected);
    planesLayout->addWidget(m_btnXZ);
    
    // YZ Plane (Right)
    m_btnYZ = new QPushButton("📐 YZ\n(Right)");
    m_btnYZ->setFixedSize(80, 60);
    m_btnYZ->setToolTip("Right plane - X = 0");
    connect(m_btnYZ, &QPushButton::clicked, this, &SketchPlaneDialog::onYZSelected);
    planesLayout->addWidget(m_btnYZ);
    
    mainLayout->addWidget(planesGroup);
    
    // Face selection (only if solid exists)
    if (hasSolid) {
        auto* faceGroup = new QGroupBox("From Model");
        auto* faceLayout = new QVBoxLayout(faceGroup);
        
        m_btnFace = new QPushButton("🎯 Select Face from Model...");
        m_btnFace->setMinimumHeight(40);
        m_btnFace->setToolTip("Click on a flat face of the 3D model");
        connect(m_btnFace, &QPushButton::clicked, this, &SketchPlaneDialog::onFaceSelected);
        faceLayout->addWidget(m_btnFace);
        
        mainLayout->addWidget(faceGroup);
    }
    
    // Cancel button
    auto* cancelBtn = new QPushButton("Cancel");
    cancelBtn->setMinimumHeight(35);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    mainLayout->addWidget(cancelBtn);
    
    // Styling
    setStyleSheet(R"(
        QDialog {
            background-color: #2b2b2b;
            color: #ffffff;
        }
        QGroupBox {
            font-weight: bold;
            border: 1px solid #555;
            border-radius: 5px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px;
        }
        QPushButton {
            background-color: #3c3c3c;
            color: #ffffff;
            border: 1px solid #555;
            border-radius: 5px;
            padding: 8px;
        }
        QPushButton:hover {
            background-color: #4a4a4a;
            border-color: #0078d4;
        }
        QPushButton:pressed {
            background-color: #0078d4;
        }
    )");
}

void SketchPlaneDialog::onXYSelected() {
    m_selectedPlane = SketchPlaneType::XY;
    accept();
}

void SketchPlaneDialog::onXZSelected() {
    m_selectedPlane = SketchPlaneType::XZ;
    accept();
}

void SketchPlaneDialog::onYZSelected() {
    m_selectedPlane = SketchPlaneType::YZ;
    accept();
}

void SketchPlaneDialog::onFaceSelected() {
    m_selectedPlane = SketchPlaneType::Face;
    accept();
}

} // namespace ui
} // namespace opencad
