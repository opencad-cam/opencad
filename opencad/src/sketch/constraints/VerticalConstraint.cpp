/**
 * @file VerticalConstraint.cpp
 * @brief Implementation of VerticalConstraint
 */

#include "VerticalConstraint.h"

namespace opencad {
namespace sketch {

VerticalConstraint::VerticalConstraint() : Constraint(), m_line(nullptr) {}

VerticalConstraint::VerticalConstraint(SketchLine::Ptr line)
    : Constraint(), m_line(line) {}

std::vector<SketchEntity::Ptr> VerticalConstraint::entities() const {
    if (m_line) return {m_line};
    return {};
}

double VerticalConstraint::error() const {
    if (!m_line) return 0.0;
    return m_line->endPoint().X() - m_line->startPoint().X();
}

std::vector<double> VerticalConstraint::jacobian() const {
    // Error = x2 - x1
    return {-1.0, 0.0, 1.0, 0.0};
}

Constraint::Ptr VerticalConstraint::clone() const {
    auto cloned = std::make_shared<VerticalConstraint>(m_line);
    cloned->setDriving(isDriving());
    cloned->setEnabled(isEnabled());
    return cloned;
}

} // namespace sketch
} // namespace opencad
