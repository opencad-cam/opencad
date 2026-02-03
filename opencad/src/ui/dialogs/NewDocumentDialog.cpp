#include "NewDocumentDialog.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>


namespace opencad {
namespace ui {

NewDocumentDialog::NewDocumentDialog(QWidget *parent) : QDialog(parent) {
  setupUi();
  setWindowTitle("New Document");
}

NewDocumentDialog::~NewDocumentDialog() = default;

void NewDocumentDialog::setupUi() {
  auto layout = new QVBoxLayout(this);

  auto label = new QLabel("Select Document Type:", this);
  label->setAlignment(Qt::AlignCenter);
  layout->addWidget(label);

  auto btnLayout = new QHBoxLayout();

  // Part Button
  auto btnPart = new QPushButton("Part", this);
  btnPart->setMinimumSize(100, 100);
  // In a real app we would add icons here
  connect(btnPart, &QPushButton::clicked, this, [this]() {
    m_selectedType = DocumentType::Part;
    accept();
  });

  // Assembly Button
  auto btnAssembly = new QPushButton("Assembly", this);
  btnAssembly->setMinimumSize(100, 100);
  connect(btnAssembly, &QPushButton::clicked, this, [this]() {
    m_selectedType = DocumentType::Assembly;
    accept();
  });

  btnLayout->addWidget(btnPart);
  btnLayout->addWidget(btnAssembly);

  layout->addLayout(btnLayout);

  // Quick exit
  auto cancelBtn = new QPushButton("Cancel", this);
  connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
  layout->addWidget(cancelBtn);
}

DocumentType NewDocumentDialog::getSelectedType() const {
  return m_selectedType;
}

} // namespace ui
} // namespace opencad
