#pragma once
#include "Component.h"
#include <TopoDS_Shape.hxx>
#include <memory>
#include <string>


namespace opencad {
namespace assembly {

enum class ConstraintType {
  Coincident,
  Distance,
  Angle,
  Parallel,
  Perpendicular
};

class AssemblyConstraint {
public:
  AssemblyConstraint(ConstraintType type, std::shared_ptr<Component> c1,
                     std::shared_ptr<Component> c2);
  virtual ~AssemblyConstraint() = default;

  ConstraintType getType() const;
  std::shared_ptr<Component> getComponent1() const;
  std::shared_ptr<Component> getComponent2() const;

  virtual bool isSatisfied() const;

  void setValue(double value) { m_value = value; }
  double getValue() const { return m_value; }

  // Sub-shape support (Face/Edge/Vertex)
  void setSubShapes(const TopoDS_Shape &s1, const TopoDS_Shape &s2);
  TopoDS_Shape getSubShape1() const { return m_subShape1; }
  TopoDS_Shape getSubShape2() const { return m_subShape2; }

protected:
  ConstraintType m_type;
  std::shared_ptr<Component> m_c1;
  std::shared_ptr<Component> m_c2;
  double m_value = 0.0;

  TopoDS_Shape m_subShape1;
  TopoDS_Shape m_subShape2;
};

} // namespace assembly
} // namespace opencad
