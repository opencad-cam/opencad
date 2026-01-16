/**
 * @file ChamferDialog.h
 * @brief Dialog for chamfer feature parameters
 */

#pragma once

#include <QDialog>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QButtonGroup>

namespace opencad {
namespace ui {

class ChamferDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit ChamferDialog(QWidget* parent = nullptr);
    
    bool isSymmetric() const { return m_symmetricRadio->isChecked(); }
    double distance1() const { return m_distance1Spin->value(); }
    double distance2() const { return m_distance2Spin->value(); }
    double angle() const { return m_angleSpin->value(); }
    bool isAngleMode() const { return m_angleRadio->isChecked(); }
    
private slots:
    void onModeChanged();
    
private:
    QRadioButton* m_symmetricRadio;
    QRadioButton* m_asymmetricRadio;
    QRadioButton* m_angleRadio;
    QDoubleSpinBox* m_distance1Spin;
    QDoubleSpinBox* m_distance2Spin;
    QDoubleSpinBox* m_angleSpin;
    QLabel* m_distance2Label;
    QLabel* m_angleLabel;
};

} // namespace ui
} // namespace opencad
