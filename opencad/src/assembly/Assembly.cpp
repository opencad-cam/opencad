#include "Assembly.h"
#include "ComponentGroup.h"
#include "ConstraintSolver.h"
#include <algorithm>

namespace opencad {
namespace assembly {

void Assembly::addNode(std::shared_ptr<AssemblyNode> node) {
  if (node) {
    m_nodes.push_back(node);
  }
}

void Assembly::removeNode(std::shared_ptr<AssemblyNode> node) {
  auto it = std::remove(m_nodes.begin(), m_nodes.end(), node);
  if (it != m_nodes.end()) {
    m_nodes.erase(it, m_nodes.end());
  }
}

void Assembly::addComponent(std::shared_ptr<Component> component) {
  addNode(component);
}

void Assembly::removeComponent(std::shared_ptr<Component> component) {
  // We need to find where this component is (root or inside group)
  // For now, let's assume it's at root or handle simple recursive search
  // removal? Or just iterate root nodes.

  // Try removing from root first
  auto it = std::remove(m_nodes.begin(), m_nodes.end(), component);
  if (it != m_nodes.end()) {
    m_nodes.erase(it, m_nodes.end());
    return;
  }

  // TODO: extensive search if we want to remove from sub-groups via this API
  // For now, assuming root usage for backward compat
}

std::vector<std::shared_ptr<Component>> Assembly::getComponents() const {
  std::vector<std::shared_ptr<Component>> list;
  for (const auto &node : m_nodes) {
    collectComponents(node, list);
  }
  return list;
}

void Assembly::collectComponents(
    std::shared_ptr<AssemblyNode> node,
    std::vector<std::shared_ptr<Component>> &list) const {
  if (!node)
    return;

  if (auto comp = std::dynamic_pointer_cast<Component>(node)) {
    list.push_back(comp);
  } else if (node->isGroup()) {
    auto group = std::static_pointer_cast<ComponentGroup>(node);
    for (const auto &child : group->getChildren()) {
      collectComponents(child, list);
    }
  }
}

void Assembly::moveNode(std::shared_ptr<AssemblyNode> node,
                        std::shared_ptr<AssemblyNode> newParent, int index) {
  // 1. Remove from old parent
  auto oldParent = node->getParent();
  if (oldParent) {
    if (oldParent->isGroup()) {
      std::static_pointer_cast<ComponentGroup>(oldParent)->removeChild(node);
    }
  } else {
    removeNode(node);
  }

  // 2. Add to new parent
  if (newParent && newParent->isGroup()) {
    // insertion at index not yet fully supported in ComponentGroup interface
    // (push_back only), need to add insert logic if index != -1
    std::static_pointer_cast<ComponentGroup>(newParent)->addChild(node);
  } else {
    // Add to root
    if (index >= 0 && index <= (int)m_nodes.size()) {
      m_nodes.insert(m_nodes.begin() + index, node);
    } else {
      m_nodes.push_back(node);
    }
    node->setParent(nullptr);
  }
}

void Assembly::addConstraint(std::shared_ptr<AssemblyConstraint> constraint) {
  m_constraints.push_back(constraint);
}

void Assembly::removeConstraint(
    std::shared_ptr<AssemblyConstraint> constraint) {
  auto it = std::remove(m_constraints.begin(), m_constraints.end(), constraint);
  if (it != m_constraints.end()) {
    m_constraints.erase(it, m_constraints.end());
  }
}

const std::vector<std::shared_ptr<AssemblyConstraint>> &
Assembly::getConstraints() const {
  return m_constraints;
}

void Assembly::clear() {
  m_constraints.clear();
  m_nodes.clear();
  // m_components.clear(); // Removed
}

bool Assembly::solve() {
  ConstraintSolver solver;
  return solver.solve(*this);
}

} // namespace assembly
} // namespace opencad
