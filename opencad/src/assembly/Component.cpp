#include "Component.h"

namespace opencad {
namespace assembly {

Component::Component(const std::shared_ptr<core::Shape>& shape)
    : m_shape(shape)
{
}

void Component::setPlacement(const gp_Trsf& trsf) {
    m_placement = trsf;
}

const gp_Trsf& Component::getPlacement() const {
    return m_placement;
}

std::shared_ptr<core::Shape> Component::getShape() const {
    return m_shape;
}

} // namespace assembly
} // namespace opencad
