/**
 * @file ShellDialog.h
 * @brief Dialog for shell feature parameters
 */

#pragma once

#include <QDialog>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>

namespace opencad {
namespace ui {

class ShellDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit ShellDialog(QWidget* parent = nullptr);
    
    double thickness() const { return m_thicknessSpin->value(); }
    bool outward() const { return m_outwardCheck->isChecked(); }
    
private:
    QDoubleSpinBox* m_thicknessSpin;
    QCheckBox* m_outwardCheck;
};

} // namespace ui
} // namespace opencad
