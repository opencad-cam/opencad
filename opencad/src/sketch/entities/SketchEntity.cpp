/**
 * @file SketchEntity.cpp
 * @brief Implementation of SketchEntity base class
 */

#include "SketchEntity.h"
#include "../constraints/Constraint.h"

namespace opencad {
namespace sketch {

uint64_t SketchEntity::s_nextId = 1;

SketchEntity::SketchEntity()
    : m_id(s_nextId++)
    , m_isConstruction(false)
    , m_isSelected(false)
{
}

void SketchEntity::addConstraint(std::shared_ptr<Constraint> constraint) {
    m_constraints.push_back(constraint);
}

void SketchEntity::removeConstraint(std::shared_ptr<Constraint> constraint) {
    m_constraints.erase(
        std::remove_if(m_constraints.begin(), m_constraints.end(),
            [&constraint](const std::weak_ptr<Constraint>& wp) {
                auto sp = wp.lock();
                return !sp || sp == constraint;
            }),
        m_constraints.end()
    );
}

} // namespace sketch
} // namespace opencad
