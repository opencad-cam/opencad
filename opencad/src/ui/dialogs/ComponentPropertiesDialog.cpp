#include "ComponentPropertiesDialog.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace opencad {
namespace ui {

ComponentPropertiesDialog::ComponentPropertiesDialog(
    std::shared_ptr<assembly::Component> component, QWidget *parent)
    : QDialog(parent), m_component(component) {

  setWindowTitle("Component Properties");
  setModal(true);

  auto *layout = new QVBoxLayout(this);

  // Name
  auto *nameLayout = new QHBoxLayout();
  nameLayout->addWidget(new QLabel("Name:"));
  m_nameEdit = new QLineEdit(QString::fromStdString(m_component->getName()));
  nameLayout->addWidget(m_nameEdit);
  layout->addLayout(nameLayout);

  // Position
  auto *posGroup = new QGroupBox("Position");
  auto *posLayout = new QGridLayout(posGroup);

  m_posX = new QDoubleSpinBox();
  m_posX->setRange(-10000.0, 10000.0);
  m_posX->setSingleStep(1.0);

  m_posY = new QDoubleSpinBox();
  m_posY->setRange(-10000.0, 10000.0);
  m_posY->setSingleStep(1.0);

  m_posZ = new QDoubleSpinBox();
  m_posZ->setRange(-10000.0, 10000.0);
  m_posZ->setSingleStep(1.0);

  // Get current transform
  gp_Trsf transform = m_component->getPlacement();
  gp_XYZ trans = transform.TranslationPart();

  m_posX->setValue(trans.X());
  m_posY->setValue(trans.Y());
  m_posZ->setValue(trans.Z());

  posLayout->addWidget(new QLabel("X:"), 0, 0);
  posLayout->addWidget(m_posX, 0, 1);
  posLayout->addWidget(new QLabel("Y:"), 1, 0);
  posLayout->addWidget(m_posY, 1, 1);
  posLayout->addWidget(new QLabel("Z:"), 2, 0);
  posLayout->addWidget(m_posZ, 2, 1);

  layout->addWidget(posGroup);

  // Buttons
  auto *buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this,
          &ComponentPropertiesDialog::onOk);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  layout->addWidget(buttonBox);
}

void ComponentPropertiesDialog::onOk() {
  onApply();
  accept();
}

void ComponentPropertiesDialog::onApply() {
  if (!m_component)
    return;

  // Update Name
  m_component->setName(m_nameEdit->text().toStdString());

  // Update Transform
  gp_Trsf newTransform;
  // We preserve rotation for now as we don't have rotation inputs yet.
  // Ideally we decompose, update translation, then recompose.
  gp_Trsf current = m_component->getPlacement();
  gp_XYZ newTrans(m_posX->value(), m_posY->value(), m_posZ->value());

  // Create new transform with SAME rotation but NEW translation
  // SetTranslationPart changes T but keeps R
  current.SetTranslationPart(newTrans);

  m_component->setPlacement(current);
}

} // namespace ui
} // namespace opencad
