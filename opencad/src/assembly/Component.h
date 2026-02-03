#pragma once

#include "AssemblyNode.h"
#include "core/geometry/Shape.h"
#include <gp_Trsf.hxx>
#include <memory>
#include <string>

namespace opencad {
namespace assembly {

class Component : public AssemblyNode {
public:
  Component(const std::shared_ptr<opencad::core::Shape> &shape);
  ~Component() override = default;

  void setPlacement(const gp_Trsf &trsf);
  const gp_Trsf &getPlacement() const;

  std::shared_ptr<opencad::core::Shape> getShape() const;

  // Name and ID are now handled by AssemblyNode base class
  // setName/getName/id are available from base

  /**
   * @brief Get the shape with the component's placement applied
   * @return TopoDS_Shape with location
   */
  TopoDS_Shape getTransformedShape() const;

private:
  std::shared_ptr<opencad::core::Shape> m_shape;
  gp_Trsf m_placement;
};

} // namespace assembly
} // namespace opencad
