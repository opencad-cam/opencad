/**
 * @file Constraint.cpp
 * @brief Implementation of Constraint base class
 */

#include "Constraint.h"
#include <cmath>

namespace opencad {
namespace sketch {

uint64_t Constraint::s_nextId = 1;

Constraint::Constraint()
    : m_id(s_nextId++)
    , m_isDriving(true)
    , m_isEnabled(true)
{
}

bool Constraint::isSatisfied(double tolerance) const {
    return std::abs(error()) <= tolerance;
}

ConstraintStatus Constraint::status() const {
    if (!m_isEnabled) {
        return ConstraintStatus::Invalid;
    }
    
    if (isSatisfied()) {
        return ConstraintStatus::Satisfied;
    }
    
    return ConstraintStatus::NotSatisfied;
}

} // namespace sketch
} // namespace opencad
