#pragma once

#include "assembly/Assembly.h"
#include "assembly/AssemblyNode.h"
#include <QTreeWidget>
#include <memory>

namespace opencad {
namespace ui {

class AssemblyTreeWidget : public QTreeWidget {
  Q_OBJECT

public:
  explicit AssemblyTreeWidget(QWidget *parent = nullptr);
  ~AssemblyTreeWidget() override;

  void setAssembly(std::shared_ptr<assembly::Assembly> assembly);
  void updateTree();
  void selectComponent(std::shared_ptr<assembly::Component> component);

signals:
  void componentSelected(std::shared_ptr<assembly::Component> component);
  void componentsSelected(
      std::vector<std::shared_ptr<assembly::Component>> components);
  void
  constraintSelected(std::shared_ptr<assembly::AssemblyConstraint> constraint);
  void visibilityChanged();
  void structChanged(); // Tree structure changed (reorder/group)

public:
  /**
   * @brief Get all currently selected components (for multi-select operations)
   */
  std::vector<std::shared_ptr<assembly::Component>>
  getSelectedComponents() const;

protected:
  // Drag and Drop overrides
  void dropEvent(QDropEvent *event) override;
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dragMoveEvent(QDragMoveEvent *event) override;
  QMimeData *mimeData(const QList<QTreeWidgetItem *> &items) const override;
  QStringList mimeTypes() const override;
  Qt::DropActions supportedDropActions() const override;

private slots:
  void onItemDoubleClicked(QTreeWidgetItem *item, int column);
  void onItemChanged(QTreeWidgetItem *item, int column);
  void onCustomContextMenuRequested(const QPoint &pos);
  void onSelectionChanged();

private:
  std::shared_ptr<assembly::Assembly> m_assembly;
  std::shared_ptr<assembly::AssemblyNode>
      m_copiedNode; // For clipboard logic if needed

  QTreeWidgetItem *m_componentsRoot = nullptr;
  QTreeWidgetItem *m_constraintsRoot = nullptr;

  void setupTree();
  void buildTreeRecursively(QTreeWidgetItem *parentItem,
                            std::shared_ptr<assembly::AssemblyNode> node);

  // Helpers
  std::shared_ptr<assembly::AssemblyNode>
  getNodeFromItem(QTreeWidgetItem *item) const;

  std::shared_ptr<assembly::AssemblyNode> findNode(void *ptr) const;

  void addComponentItem(
      std::shared_ptr<assembly::Component> component); // Deprecated/Refactor
  void
  addConstraintItem(std::shared_ptr<assembly::AssemblyConstraint> constraint);

  // Grouping
  void createGroup();
};

} // namespace ui
} // namespace opencad
