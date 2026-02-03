#pragma once

#include <QDialog>
#include <memory>

class QPushButton;

namespace opencad {
namespace ui {

enum class DocumentType { Part, Assembly };

class NewDocumentDialog : public QDialog {
  Q_OBJECT

public:
  explicit NewDocumentDialog(QWidget *parent = nullptr);
  ~NewDocumentDialog() override;

  DocumentType getSelectedType() const;

private:
  void setupUi();

  DocumentType m_selectedType = DocumentType::Part;
};

} // namespace ui
} // namespace opencad
