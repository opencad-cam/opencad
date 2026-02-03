#include "AssemblyConstraint.h"

namespace opencad {
namespace assembly {

AssemblyConstraint::AssemblyConstraint(ConstraintType type,
                                       std::shared_ptr<Component> c1,
                                       std::shared_ptr<Component> c2)
    : m_type(type), m_c1(c1), m_c2(c2), m_value(0.0) {}

ConstraintType AssemblyConstraint::getType() const { return m_type; }

std::shared_ptr<Component> AssemblyConstraint::getComponent1() const {
  return m_c1;
}

std::shared_ptr<Component> AssemblyConstraint::getComponent2() const {
  return m_c2;
}

bool AssemblyConstraint::isSatisfied() const {
  // Placeholder implementation
  return true;
}

void AssemblyConstraint::setSubShapes(const TopoDS_Shape &s1,
                                      const TopoDS_Shape &s2) {
  m_subShape1 = s1;
  m_subShape2 = s2;
}

} // namespace assembly
} // namespace opencad
