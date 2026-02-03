#include "Component.h"
#include <QUuid>
#include <TopLoc_Location.hxx>

namespace opencad {
namespace assembly {

Component::Component(const std::shared_ptr<opencad::core::Shape> &shape)
    : m_shape(shape) {}

void Component::setPlacement(const gp_Trsf &trsf) { m_placement = trsf; }

const gp_Trsf &Component::getPlacement() const { return m_placement; }

std::shared_ptr<opencad::core::Shape> Component::getShape() const {
  return m_shape;
}

TopoDS_Shape Component::getTransformedShape() const {
  if (!m_shape || m_shape->isNull()) {
    return TopoDS_Shape();
  }

  TopoDS_Shape shape = m_shape->occShape();
  TopLoc_Location loc(m_placement);
  return shape.Moved(loc);
}

} // namespace assembly
} // namespace opencad
