#pragma once
/**
 * @file SketchPlaneDialog.h
 * @brief Dialog for selecting sketch plane (XY, XZ, YZ or face)
 */

#include <QDialog>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QButtonGroup>
#include <QRadioButton>

namespace opencad {
namespace ui {

/**
 * @brief Sketch plane selection result
 */
enum class SketchPlaneType {
    None,
    XY,      // Top plane (Z=0)
    XZ,      // Front plane (Y=0)
    YZ,      // Right plane (X=0)
    Face     // Select from 3D model
};

/**
 * @class SketchPlaneDialog
 * @brief Modal dialog for selecting sketch plane
 */
class SketchPlaneDialog : public QDialog {
    Q_OBJECT

public:
    explicit SketchPlaneDialog(QWidget* parent = nullptr, bool hasSolid = false);
    
    /// Get selected plane type
    SketchPlaneType selectedPlane() const { return m_selectedPlane; }

private slots:
    void onXYSelected();
    void onXZSelected();
    void onYZSelected();
    void onFaceSelected();

private:
    void setupUI(bool hasSolid);
    
    SketchPlaneType m_selectedPlane = SketchPlaneType::None;
    
    QPushButton* m_btnXY = nullptr;
    QPushButton* m_btnXZ = nullptr;
    QPushButton* m_btnYZ = nullptr;
    QPushButton* m_btnFace = nullptr;
};

} // namespace ui
} // namespace opencad
