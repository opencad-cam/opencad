/**
 * @file LoftDialog.cpp
 * @brief Loft dialog implementation
 */

#include "LoftDialog.h"
#include <QHBoxLayout>

namespace opencad {
namespace ui {

LoftDialog::LoftDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Loft");
    setMinimumWidth(500);
    setMinimumHeight(400);
    
    auto* layout = new QVBoxLayout(this);
    
    // Info
    auto* infoLabel = new QLabel(
        "Loft creates a solid by blending between multiple profile sketches.\n"
        "Select at least 2 profiles and arrange them in order.");
    infoLabel->setWordWrap(true);
    layout->addWidget(infoLabel);
    
    // Profile selection area
    auto* profileGroup = new QGroupBox("Profile Sketches");
    auto* profileLayout = new QHBoxLayout(profileGroup);
    
    // Available sketches
    auto* availableLayout = new QVBoxLayout();
    availableLayout->addWidget(new QLabel("Available:"));
    m_availableList = new QListWidget();
    m_availableList->setSelectionMode(QAbstractItemView::SingleSelection);
    availableLayout->addWidget(m_availableList);
    profileLayout->addLayout(availableLayout);
    
    // Add/Remove buttons
    auto* buttonLayout = new QVBoxLayout();
    buttonLayout->addStretch();
    auto* addBtn = new QPushButton("→ Add");
    auto* removeBtn = new QPushButton("← Remove");
    buttonLayout->addWidget(addBtn);
    buttonLayout->addWidget(removeBtn);
    buttonLayout->addStretch();
    profileLayout->addLayout(buttonLayout);
    
    connect(addBtn, &QPushButton::clicked, this, &LoftDialog::onAddProfile);
    connect(removeBtn, &QPushButton::clicked, this, &LoftDialog::onRemoveProfile);
    
    // Selected sketches with order
    auto* selectedLayout = new QVBoxLayout();
    selectedLayout->addWidget(new QLabel("Selected (in order):"));
    m_selectedList = new QListWidget();
    m_selectedList->setSelectionMode(QAbstractItemView::SingleSelection);
    selectedLayout->addWidget(m_selectedList);
    
    // Up/Down buttons
    auto* orderLayout = new QHBoxLayout();
    auto* upBtn = new QPushButton("↑ Up");
    auto* downBtn = new QPushButton("↓ Down");
    orderLayout->addWidget(upBtn);
    orderLayout->addWidget(downBtn);
    selectedLayout->addLayout(orderLayout);
    
    connect(upBtn, &QPushButton::clicked, this, &LoftDialog::onMoveUp);
    connect(downBtn, &QPushButton::clicked, this, &LoftDialog::onMoveDown);
    
    profileLayout->addLayout(selectedLayout);
    layout->addWidget(profileGroup);
    
    // Options
    auto* optGroup = new QGroupBox("Options");
    auto* optLayout = new QVBoxLayout(optGroup);
    
    m_solidCheck = new QCheckBox("Create solid (uncheck for surface)");
    m_solidCheck->setChecked(true);
    optLayout->addWidget(m_solidCheck);
    
    m_ruledCheck = new QCheckBox("Ruled surface (linear interpolation)");
    m_ruledCheck->setChecked(false);
    optLayout->addWidget(m_ruledCheck);
    
    layout->addWidget(optGroup);
    
    // Buttons
    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    layout->addWidget(buttons);
}

void LoftDialog::setAvailableSketches(const QStringList& sketches) {
    m_availableList->clear();
    m_availableList->addItems(sketches);
}

QStringList LoftDialog::selectedProfiles() const {
    QStringList result;
    for (int i = 0; i < m_selectedList->count(); ++i) {
        result << m_selectedList->item(i)->text();
    }
    return result;
}

void LoftDialog::onAddProfile() {
    auto* item = m_availableList->currentItem();
    if (item) {
        m_selectedList->addItem(item->text());
        delete m_availableList->takeItem(m_availableList->row(item));
    }
}

void LoftDialog::onRemoveProfile() {
    auto* item = m_selectedList->currentItem();
    if (item) {
        m_availableList->addItem(item->text());
        delete m_selectedList->takeItem(m_selectedList->row(item));
    }
}

void LoftDialog::onMoveUp() {
    int row = m_selectedList->currentRow();
    if (row > 0) {
        auto* item = m_selectedList->takeItem(row);
        m_selectedList->insertItem(row - 1, item);
        m_selectedList->setCurrentRow(row - 1);
    }
}

void LoftDialog::onMoveDown() {
    int row = m_selectedList->currentRow();
    if (row >= 0 && row < m_selectedList->count() - 1) {
        auto* item = m_selectedList->takeItem(row);
        m_selectedList->insertItem(row + 1, item);
        m_selectedList->setCurrentRow(row + 1);
    }
}

} // namespace ui
} // namespace opencad
