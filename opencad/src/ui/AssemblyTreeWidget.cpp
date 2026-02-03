#include "AssemblyTreeWidget.h"
#include "assembly/Component.h"
#include "assembly/ComponentGroup.h"
#include "dialogs/ComponentPropertiesDialog.h"
#include <QDropEvent>
#include <QHeaderView>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>

namespace opencad {
namespace ui {

AssemblyTreeWidget::AssemblyTreeWidget(QWidget *parent) : QTreeWidget(parent) {
  setupTree();
  setContextMenuPolicy(Qt::CustomContextMenu);

  setDragEnabled(true);
  setAcceptDrops(true);
  setDragDropMode(QAbstractItemView::InternalMove);
  setSelectionMode(QAbstractItemView::ExtendedSelection);

  connect(this, &QTreeWidget::customContextMenuRequested, this,
          &AssemblyTreeWidget::onCustomContextMenuRequested);
  connect(this, &QTreeWidget::itemDoubleClicked, this,
          &AssemblyTreeWidget::onItemDoubleClicked);
  connect(this, &QTreeWidget::itemSelectionChanged, this,
          &AssemblyTreeWidget::onSelectionChanged);
  connect(this, &QTreeWidget::itemChanged, this,
          &AssemblyTreeWidget::onItemChanged);
}

AssemblyTreeWidget::~AssemblyTreeWidget() = default;

void AssemblyTreeWidget::setAssembly(
    std::shared_ptr<assembly::Assembly> assembly) {
  m_assembly = assembly;
  updateTree();
}

void AssemblyTreeWidget::setupTree() {
  QStringList headers;
  headers << "Assembly"
          << "Vis"; // Added Visualization column? Or just use checkstate
  // Actually, standard QTreeWidget checkstate is on column 0 usually.
  // Let's stick to standard behavior: Checkbox on item itself.
  setHeaderLabel("Assembly");
  header()->setSectionResizeMode(QHeaderView::ResizeToContents);

  // Create root nodes
  m_componentsRoot = new QTreeWidgetItem(this);
  m_componentsRoot->setText(0, "Components");
  m_componentsRoot->setExpanded(true);
  m_componentsRoot->setIcon(0, QIcon::fromTheme("folder"));
  m_componentsRoot->setFlags(m_componentsRoot->flags() &
                             ~Qt::ItemIsDragEnabled); // Root not draggable

  m_constraintsRoot = new QTreeWidgetItem(this);
  m_constraintsRoot->setText(0, "Mates");
  m_constraintsRoot->setExpanded(true);
  m_constraintsRoot->setIcon(0, QIcon::fromTheme("folder"));
  m_constraintsRoot->setFlags(m_constraintsRoot->flags() &
                              ~Qt::ItemIsDragEnabled);
}

void AssemblyTreeWidget::updateTree() {
  if (!m_assembly)
    return;

  bool wasBlocked = signalsBlocked();
  blockSignals(true);

  // Clear existing children of roots
  qDeleteAll(m_componentsRoot->takeChildren());
  qDeleteAll(m_constraintsRoot->takeChildren());

  // Add Nodes recursively
  for (const auto &node : m_assembly->getNodes()) {
    buildTreeRecursively(m_componentsRoot, node);
  }

  // Add Constraints
  for (const auto &constraint : m_assembly->getConstraints()) {
    addConstraintItem(constraint);
  }

  blockSignals(wasBlocked);
}

void AssemblyTreeWidget::buildTreeRecursively(
    QTreeWidgetItem *parentItem, std::shared_ptr<assembly::AssemblyNode> node) {
  if (!node)
    return;

  auto item = new QTreeWidgetItem(parentItem);
  item->setText(0, QString::fromStdString(node->getName()));

  // Visibility Checkbox
  item->setCheckState(0, node->isVisible() ? Qt::Checked : Qt::Unchecked);

  // Store pointer
  // Use setData with size_t or simple casting if pointer arithmetic is safe
  // Ideally we'd store QUuid and look it up, but for speed pointer is OK if
  // assembly ensures lifetime To be safer, we can store QUuid string
  item->setData(0, Qt::UserRole,
                QVariant::fromValue(static_cast<void *>(node.get())));

  if (node->isGroup()) {
    item->setIcon(0, QIcon::fromTheme("folder-open")); // Or style's folder icon

    auto group = std::static_pointer_cast<assembly::ComponentGroup>(node);
    for (const auto &child : group->getChildren()) {
      buildTreeRecursively(item, child);
    }
    item->setExpanded(true); // Auto expand groups?
  } else {
    // Show anchor icon if fixed, otherwise package
    if (node->isFixed()) {
      item->setIcon(0, QIcon::fromTheme("anchor"));
    } else {
      item->setIcon(0, QIcon::fromTheme("package")); // Component icon
    }
  }
}

std::shared_ptr<assembly::AssemblyNode>
AssemblyTreeWidget::getNodeFromItem(QTreeWidgetItem *item) const {
  if (!item || !m_assembly)
    return nullptr;

  void *ptr = item->data(0, Qt::UserRole).value<void *>();
  if (!ptr)
    return nullptr;

  // Search in assembly nodes recursively to find shared_ptr matching this raw
  // pointer This is the safe way to get shared_ptr from raw pointer

  // Helper lambda
  std::function<std::shared_ptr<assembly::AssemblyNode>(
      const std::vector<std::shared_ptr<assembly::AssemblyNode>> &, void *)>
      finder;
  finder =
      [&](const std::vector<std::shared_ptr<assembly::AssemblyNode>> &nodes,
          void *p) -> std::shared_ptr<assembly::AssemblyNode> {
    for (const auto &node : nodes) {
      if (node.get() == p)
        return node;
      if (node->isGroup()) {
        auto group = std::static_pointer_cast<assembly::ComponentGroup>(node);
        auto found = finder(group->getChildren(), p);
        if (found)
          return found;
      }
    }
    return nullptr;
  };

  return finder(m_assembly->getNodes(), ptr);
}

void AssemblyTreeWidget::onItemChanged(QTreeWidgetItem *item, int column) {
  if (column == 0) {
    auto node = getNodeFromItem(item);
    if (node) {
      bool visible = (item->checkState(0) == Qt::Checked);
      if (node->isVisible() != visible) {
        node->setVisible(visible);
        emit visibilityChanged();
      }
    }
  }
}

void AssemblyTreeWidget::addConstraintItem(
    std::shared_ptr<assembly::AssemblyConstraint> constraint) {
  auto item = new QTreeWidgetItem(m_constraintsRoot);
  item->setText(0, "Mate"); // TODO: Real name
  item->setIcon(0, QIcon::fromTheme("link"));
  item->setData(0, Qt::UserRole,
                QVariant::fromValue(static_cast<void *>(constraint.get())));
}

void AssemblyTreeWidget::onItemDoubleClicked(QTreeWidgetItem *item,
                                             int column) {
  // Zoom to fit?
}

void AssemblyTreeWidget::onCustomContextMenuRequested(const QPoint &pos) {
  auto item = itemAt(pos);
  QMenu menu(this);

  auto createGroupAction = menu.addAction("Create New Group");
  connect(createGroupAction, &QAction::triggered, this,
          &AssemblyTreeWidget::createGroup);

  if (item) {
    auto node = getNodeFromItem(item);
    if (node) {
      menu.addSeparator();

      // Properties Action
      if (!node->isGroup()) {
        auto comp = std::dynamic_pointer_cast<assembly::Component>(node);
        if (comp) {
          auto propsAction = menu.addAction("Properties");
          connect(propsAction, &QAction::triggered, [this, comp]() {
            opencad::ui::ComponentPropertiesDialog dialog(comp, this);
            if (dialog.exec() == QDialog::Accepted) {
              emit structChanged();     // Name might have changed
              emit visibilityChanged(); // Position implies visual update
            }
          });
        }
      }

      auto renameAction = menu.addAction("Rename");
      connect(renameAction, &QAction::triggered, [this, node, item]() {
        bool ok;
        QString text = QInputDialog::getText(
            this, "Rename", "New Name:", QLineEdit::Normal,
            QString::fromStdString(node->getName()), &ok);
        if (ok && !text.isEmpty()) {
          node->setName(text.toStdString());
          item->setText(0, text);
          emit structChanged();
        }
      });

      // Fix/Float (Anchor) toggle
      QString fixedLabel = node->isFixed() ? "Float (Unfix)" : "Fix (Anchor)";
      auto fixedAction = menu.addAction(fixedLabel);
      connect(fixedAction, &QAction::triggered, [this, node, item]() {
        node->setFixed(!node->isFixed());
        // Update icon to show fixed state
        if (node->isFixed()) {
          item->setIcon(0, QIcon::fromTheme("anchor")); // or lock icon
        } else {
          if (node->isGroup()) {
            item->setIcon(0, QIcon::fromTheme("folder-open"));
          } else {
            item->setIcon(0, QIcon::fromTheme("package"));
          }
        }
        emit structChanged();
      });

      auto deleteAction = menu.addAction("Delete");
      connect(deleteAction, &QAction::triggered, [this, node]() {
        m_assembly->removeNode(
            node); // Need recursive removal from parent if parent is group
        // Actually removeNode in Assembly removed from root. We need remove
        // from parent.

        auto parent = node->getParent();
        if (parent && parent->isGroup()) {
          std::static_pointer_cast<assembly::ComponentGroup>(parent)
              ->removeChild(node);
        } else {
          m_assembly->removeNode(node);
        }

        updateTree();
        emit visibilityChanged(); // To refresh viewport
        emit structChanged();
      });
    }
  }

  menu.exec(mapToGlobal(pos));
}

void AssemblyTreeWidget::createGroup() {
  if (!m_assembly)
    return;

  bool ok;
  QString text = QInputDialog::getText(
      this, "New Group", "Group Name:", QLineEdit::Normal, "New Group", &ok);
  if (ok && !text.isEmpty()) {
    auto group = std::make_shared<assembly::ComponentGroup>(text.toStdString());

    // Add to currently selected group or root
    auto items = selectedItems();
    if (!items.isEmpty()) {
      auto parentNode = getNodeFromItem(items.first());
      if (parentNode && parentNode->isGroup()) {
        std::static_pointer_cast<assembly::ComponentGroup>(parentNode)
            ->addChild(group);
      } else {
        // If selected item is a component, add group to its parent?
        // Or just add to root? Let's add to root for simplicity or current
        // level logic. Simple version: Add to root.
        m_assembly->addNode(group);
      }
    } else {
      m_assembly->addNode(group);
    }
    updateTree();
    emit structChanged();
  }
}

// Drag and Drop Logic
QStringList AssemblyTreeWidget::mimeTypes() const {
  return QStringList() << "application/x-opencad-assembly-item";
}

QMimeData *
AssemblyTreeWidget::mimeData(const QList<QTreeWidgetItem *> &items) const {
  QMimeData *mimeData = new QMimeData;
  QByteArray encodedData;
  QDataStream stream(&encodedData, QIODevice::WriteOnly);

  // We only support dragging one item effectively for structural reorder for
  // now But let's write pointer addresses
  for (QTreeWidgetItem *item : items) {
    if (item == m_componentsRoot || item == m_constraintsRoot)
      continue;
    void *ptr = item->data(0, Qt::UserRole).value<void *>();
    stream.writeRawData(reinterpret_cast<char *>(&ptr), sizeof(void *));
  }

  mimeData->setData("application/x-opencad-assembly-item", encodedData);
  return mimeData;
}

void AssemblyTreeWidget::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasFormat("application/x-opencad-assembly-item"))
    event->acceptProposedAction();
}

void AssemblyTreeWidget::dragMoveEvent(QDragMoveEvent *event) {
  if (event->mimeData()->hasFormat("application/x-opencad-assembly-item"))
    event->acceptProposedAction();
}

void AssemblyTreeWidget::onSelectionChanged() {
  QList<QTreeWidgetItem *> items = selectedItems();
  if (items.isEmpty()) {
    emit componentSelected(nullptr);
    return;
  }

  // Collect all selected components
  std::vector<std::shared_ptr<assembly::Component>> selectedComps;
  for (QTreeWidgetItem *item : items) {
    auto node = getNodeFromItem(item);
    if (node && !node->isGroup()) {
      auto comp = std::dynamic_pointer_cast<assembly::Component>(node);
      if (comp) {
        selectedComps.push_back(comp);
      }
    }
  }

  // Emit single component for backward compatibility
  if (!selectedComps.empty()) {
    emit componentSelected(selectedComps.front());
  }

  // Emit all selected components for multi-part operations
  if (selectedComps.size() > 1) {
    emit componentsSelected(selectedComps);
  }
}

std::vector<std::shared_ptr<assembly::Component>>
AssemblyTreeWidget::getSelectedComponents() const {
  std::vector<std::shared_ptr<assembly::Component>> selectedComps;

  QList<QTreeWidgetItem *> items = selectedItems();
  for (QTreeWidgetItem *item : items) {
    auto node = getNodeFromItem(item);
    if (node && !node->isGroup()) {
      auto comp = std::dynamic_pointer_cast<assembly::Component>(node);
      if (comp) {
        selectedComps.push_back(comp);
      }
    }
  }

  return selectedComps;
}

std::shared_ptr<assembly::AssemblyNode>
AssemblyTreeWidget::findNode(void *ptr) const {
  if (!m_assembly)
    return nullptr;

  // Reuse the lambda logic from getNodeFromItem or just call it recursively
  // We can extract the recursive finder to a private static/member method
  // For now, I'll duplicate the lambda logic slightly modified for Member
  // function usage

  std::function<std::shared_ptr<assembly::AssemblyNode>(
      const std::vector<std::shared_ptr<assembly::AssemblyNode>> &, void *)>
      finder;

  finder =
      [&](const std::vector<std::shared_ptr<assembly::AssemblyNode>> &nodes,
          void *p) -> std::shared_ptr<assembly::AssemblyNode> {
    for (const auto &node : nodes) {
      if (node.get() == p)
        return node;
      if (node->isGroup()) {
        auto group = std::static_pointer_cast<assembly::ComponentGroup>(node);
        auto found = finder(group->getChildren(), p);
        if (found)
          return found;
      }
    }
    return nullptr;
  };

  return finder(m_assembly->getNodes(), ptr);
}

Qt::DropActions AssemblyTreeWidget::supportedDropActions() const {
  return Qt::MoveAction;
}

void AssemblyTreeWidget::dropEvent(QDropEvent *event) {
  if (event->mimeData()->hasFormat("application/x-opencad-assembly-item")) {
    QByteArray encodedData =
        event->mimeData()->data("application/x-opencad-assembly-item");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    void *ptr;
    if (stream.readRawData(reinterpret_cast<char *>(&ptr), sizeof(void *)) ==
        sizeof(void *)) {

      auto nodeToMove = findNode(ptr);

      if (nodeToMove) {
        QTreeWidgetItem *targetItem = itemAt(event->position().toPoint());
        auto targetNode = getNodeFromItem(
            targetItem); // This might be null if dropped on root background

        // Debug
        // qDebug() << "Moving node" <<
        // QString::fromStdString(nodeToMove->getName());

        if (targetNode) {
          // Dropped on a node
          if (targetNode->isGroup()) {
            // Move INTO group
            m_assembly->moveNode(nodeToMove, targetNode);
          } else {
            // Dropped on a component (sibling)
            // Move to parent of target, at index of target?
            // Assembly::moveNode simplified implementation currently puts it at
            // end if parent is group or root. For now, let's assume we move to
            // the SAME parent as the target.

            // Wait, strict reordering requires finding the index.
            // Assembly::moveNode(node, parent, index)

            // Let's implement: Add to parent of target.
            auto newParent = targetNode->getParent();
            // We also need to know the index for correct ordering, but let's
            // just append for now to be safe or try to find index.

            m_assembly->moveNode(nodeToMove, newParent);
          }
        } else {
          // Dropped on empty space -> Move to Root
          m_assembly->moveNode(nodeToMove, nullptr);
        }

        updateTree();
        emit structChanged();
      }
    }
    event->accept();
  }
}

void AssemblyTreeWidget::selectComponent(
    std::shared_ptr<assembly::Component> component) {
  if (!component)
    return;

  QTreeWidgetItemIterator it(this);
  while (*it) {
    if ((*it)->data(0, Qt::UserRole).value<void *>() == component.get()) {
      bool wasBlocked = signalsBlocked();
      blockSignals(true);
      setCurrentItem(*it);
      scrollToItem(*it);
      blockSignals(wasBlocked);
      return;
    }
    ++it;
  }
}

} // namespace ui
} // namespace opencad
