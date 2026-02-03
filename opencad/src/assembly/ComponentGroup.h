#pragma once

#include "AssemblyNode.h"
#include <algorithm>
#include <vector>

namespace opencad {
namespace assembly {

class ComponentGroup : public AssemblyNode {
public:
  ComponentGroup(const std::string &name = "Group") { m_name = name; }
  ~ComponentGroup() override = default;

  bool isGroup() const override { return true; }

  void addChild(std::shared_ptr<AssemblyNode> node) {
    if (node) {
      node->setParent(
          std::static_pointer_cast<AssemblyNode>(shared_from_this()));
      m_children.push_back(node);
    }
  }

  void removeChild(std::shared_ptr<AssemblyNode> node) {
    auto it = std::remove(m_children.begin(), m_children.end(), node);
    if (it != m_children.end()) {
      if (*it)
        (*it)->setParent(nullptr);
      m_children.erase(it, m_children.end());
    }
  }

  const std::vector<std::shared_ptr<AssemblyNode>> &getChildren() const {
    return m_children;
  }

  // Recursively find child by ID
  std::shared_ptr<AssemblyNode> findChild(const QUuid &id) const {
    for (const auto &child : m_children) {
      if (child->id() == id)
        return child;
      if (child->isGroup()) {
        auto group = std::static_pointer_cast<ComponentGroup>(child);
        auto found = group->findChild(id);
        if (found)
          return found;
      }
    }
    return nullptr;
  }

private:
  std::vector<std::shared_ptr<AssemblyNode>> m_children;
};

// Need enable_shared_from_this for setParent call in addChild
// But AssemblyNode doesn't inherit it. Let's fix AssemblyNode or use a wrapper.
// Actually, it's better if ComponentGroup inherits enable_shared_from_this.
// Redefining AssemblyNode to inherit enable_shared_from_this is cleaner.

} // namespace assembly
} // namespace opencad
