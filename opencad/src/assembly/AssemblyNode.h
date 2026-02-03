#pragma once

#include <QUuid>
#include <memory>
#include <string>

namespace opencad {
namespace assembly {

class AssemblyNode : public std::enable_shared_from_this<AssemblyNode> {
public:
  AssemblyNode()
      : m_id(QUuid::createUuid()), m_visible(true), m_isFixed(false) {}
  virtual ~AssemblyNode() = default;

  void setName(const std::string &name) { m_name = name; }
  std::string getName() const { return m_name; }

  QUuid id() const { return m_id; }

  void setVisible(bool visible) { m_visible = visible; }

  bool isVisible() const {
    if (!m_visible)
      return false;
    if (auto parent = m_parent.lock()) {
      return parent->isVisible();
    }
    return true;
  }

  void setParent(std::shared_ptr<AssemblyNode> parent) { m_parent = parent; }
  std::shared_ptr<AssemblyNode> getParent() const { return m_parent.lock(); }

  // Fix/Float (anchor) support
  void setFixed(bool fixed) { m_isFixed = fixed; }
  bool isFixed() const { return m_isFixed; }

  virtual bool isGroup() const { return false; }

protected:
  std::string m_name;
  QUuid m_id;
  bool m_visible;
  bool m_isFixed;
  std::weak_ptr<AssemblyNode> m_parent;
};

} // namespace assembly
} // namespace opencad
