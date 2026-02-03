#pragma once
#include "AssemblyConstraint.h"
#include "AssemblyNode.h"
#include "Component.h"
#include <memory>
#include <vector>


namespace opencad {
namespace assembly {

class Assembly {
public:
  Assembly() = default;
  ~Assembly() = default;

  // Node management
  void addNode(std::shared_ptr<AssemblyNode> node);
  void removeNode(std::shared_ptr<AssemblyNode> node);
  const std::vector<std::shared_ptr<AssemblyNode>> &getNodes() const {
    return m_nodes;
  }

  // Reordering
  // Move 'count' items starting at 'index' to 'destIndex'
  // Simplified for now: just move one item
  void moveNode(std::shared_ptr<AssemblyNode> node,
                std::shared_ptr<AssemblyNode> newParent, int index = -1);

  // Backward compatibility helpers
  void addComponent(std::shared_ptr<Component> component);
  void removeComponent(std::shared_ptr<Component> component);

  /**
   * @brief Get all leaf components recursively
   */
  std::vector<std::shared_ptr<Component>> getComponents() const;

  void addConstraint(std::shared_ptr<AssemblyConstraint> constraint);
  void removeConstraint(std::shared_ptr<AssemblyConstraint> constraint);
  const std::vector<std::shared_ptr<AssemblyConstraint>> &
  getConstraints() const;

  void clear();

  bool solve();

private:
  void collectComponents(std::shared_ptr<AssemblyNode> node,
                         std::vector<std::shared_ptr<Component>> &list) const;

  std::vector<std::shared_ptr<AssemblyNode>> m_nodes; // Root nodes
  std::vector<std::shared_ptr<AssemblyConstraint>> m_constraints;
};

} // namespace assembly
} // namespace opencad
