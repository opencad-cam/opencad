/**
 * @file LoftDialog.h
 * @brief Dialog for loft feature parameters
 */

#pragma once

#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QListWidget>
#include <QPushButton>
#include <QCheckBox>

namespace opencad {
namespace ui {

class LoftDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit LoftDialog(QWidget* parent = nullptr);
    
    QStringList selectedProfiles() const;
    bool solidOutput() const { return m_solidCheck->isChecked(); }
    bool ruledSurface() const { return m_ruledCheck->isChecked(); }
    
    void setAvailableSketches(const QStringList& sketches);
    
private slots:
    void onAddProfile();
    void onRemoveProfile();
    void onMoveUp();
    void onMoveDown();
    
private:
    QListWidget* m_availableList;
    QListWidget* m_selectedList;
    QCheckBox* m_solidCheck;
    QCheckBox* m_ruledCheck;
};

} // namespace ui
} // namespace opencad
