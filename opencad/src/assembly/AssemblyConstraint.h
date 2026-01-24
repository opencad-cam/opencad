#pragma once
#include <string>
#include <memory>
#include "Component.h"

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
    AssemblyConstraint(ConstraintType type,
                       std::shared_ptr<Component> c1,
                       std::shared_ptr<Component> c2);
    virtual ~AssemblyConstraint() = default;

    ConstraintType getType() const;
    std::shared_ptr<Component> getComponent1() const;
    std::shared_ptr<Component> getComponent2() const;

    virtual bool isSatisfied() const;

protected:
    ConstraintType m_type;
    std::shared_ptr<Component> m_c1;
    std::shared_ptr<Component> m_c2;
};

} // namespace assembly
} // namespace opencad
