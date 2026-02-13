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
  Perpendicular,
  Concentric,
  Tangent,
  Lock, // Rigid
  Gear, // Rotation-Rotation ratio
  Screw // Rotation-Translation ratio
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

  // Gear/Screw properties
  void setRatio(double ratio) { m_ratio = ratio; }
  double getRatio() const { return m_ratio; }

  void setPitch(double pitch) { m_pitch = pitch; }
  double getPitch() const { return m_pitch; }

  // Sub-shape support (Face/Edge/Vertex)
  void setSubShapes(const TopoDS_Shape &s1, const TopoDS_Shape &s2);
  TopoDS_Shape getSubShape1() const { return m_subShape1; }
  TopoDS_Shape getSubShape2() const { return m_subShape2; }

  // Alignment support
  void setFlipped(bool flipped) { m_flipped = flipped; }
  bool isFlipped() const { return m_flipped; }

protected:
  ConstraintType m_type;
  std::shared_ptr<Component> m_c1;
  std::shared_ptr<Component> m_c2;
  double m_value = 0.0;
  double m_ratio = 1.0; // For Gear
  double m_pitch = 1.0; // For Screw
  bool m_flipped = false;

  TopoDS_Shape m_subShape1;
  TopoDS_Shape m_subShape2;
};

} // namespace assembly
} // namespace opencad
