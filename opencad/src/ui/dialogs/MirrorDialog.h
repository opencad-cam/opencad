/**
 * @file MirrorDialog.h
 * @brief Dialog for mirror feature parameters
 */

#pragma once

#include <QDialog>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>

namespace opencad {
namespace ui {

class MirrorDialog : public QDialog {
    Q_OBJECT
    
public:
    enum class MirrorPlane { XY, XZ, YZ };
    
    explicit MirrorDialog(QWidget* parent = nullptr);
    
    MirrorPlane selectedPlane() const;
    bool keepOriginal() const { return m_keepOriginalCheck->isChecked(); }
    
private:
    QRadioButton* m_xyRadio;
    QRadioButton* m_xzRadio;
    QRadioButton* m_yzRadio;
    QCheckBox* m_keepOriginalCheck;
};

} // namespace ui
} // namespace opencad
