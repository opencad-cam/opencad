#pragma once

#include "core/geometry/Shape.h"
#include <gp_Trsf.hxx>
#include <memory>

namespace opencad {
namespace assembly {

class Component {
public:
    Component(const std::shared_ptr<core::Shape>& shape);
    virtual ~Component() = default;

    void setPlacement(const gp_Trsf& trsf);
    const gp_Trsf& getPlacement() const;

    std::shared_ptr<core::Shape> getShape() const;

private:
    std::shared_ptr<core::Shape> m_shape;
    gp_Trsf m_placement;
};

} // namespace assembly
} // namespace opencad
