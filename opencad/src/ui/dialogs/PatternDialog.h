/**
 * @file PatternDialog.h
 * @brief Dialog for pattern feature parameters
 */

#pragma once

#include <QDialog>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QStackedWidget>
#include <QCheckBox>

namespace opencad {
namespace ui {

class PatternDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit PatternDialog(QWidget* parent = nullptr);
    
    bool isLinear() const { return m_linearRadio->isChecked(); }
    
    // Linear pattern
    int countX() const { return m_countXSpin->value(); }
    int countY() const { return m_countYSpin->value(); }
    double spacingX() const { return m_spacingXSpin->value(); }
    double spacingY() const { return m_spacingYSpin->value(); }
    
    // Circular pattern
    int circularCount() const { return m_circularCountSpin->value(); }
    double totalAngle() const { return m_totalAngleSpin->value(); }
    bool equalSpacing() const { return m_equalSpacingCheck->isChecked(); }
    
private slots:
    void onTypeChanged();
    
private:
    QRadioButton* m_linearRadio;
    QRadioButton* m_circularRadio;
    QStackedWidget* m_stackedWidget;
    
    // Linear
    QSpinBox* m_countXSpin;
    QSpinBox* m_countYSpin;
    QDoubleSpinBox* m_spacingXSpin;
    QDoubleSpinBox* m_spacingYSpin;
    
    // Circular
    QSpinBox* m_circularCountSpin;
    QDoubleSpinBox* m_totalAngleSpin;
    QCheckBox* m_equalSpacingCheck;
};

} // namespace ui
} // namespace opencad
