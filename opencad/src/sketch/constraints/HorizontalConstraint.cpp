/**
 * @file HorizontalConstraint.cpp
 * @brief Implementation of HorizontalConstraint
 */

#include "HorizontalConstraint.h"

namespace opencad {
namespace sketch {

HorizontalConstraint::HorizontalConstraint()
    : Constraint()
    , m_line(nullptr)
{
}

HorizontalConstraint::HorizontalConstraint(SketchLine::Ptr line)
    : Constraint()
    , m_line(line)
{
}

std::vector<SketchEntity::Ptr> HorizontalConstraint::entities() const {
    if (m_line) {
        return {m_line};
    }
    return {};
}

double HorizontalConstraint::error() const {
    if (!m_line) return 0.0;
    
    // Error is the difference in Y coordinates of endpoints
    return m_line->endPoint().Y() - m_line->startPoint().Y();
}

std::vector<double> HorizontalConstraint::jacobian() const {
    // Parameters: [x1, y1, x2, y2]
    // Error = y2 - y1
    // d(Error)/dx1 = 0
    // d(Error)/dy1 = -1
    // d(Error)/dx2 = 0
    // d(Error)/dy2 = 1
    return {0.0, -1.0, 0.0, 1.0};
}

Constraint::Ptr HorizontalConstraint::clone() const {
    auto cloned = std::make_shared<HorizontalConstraint>(m_line);
    cloned->setDriving(isDriving());
    cloned->setEnabled(isEnabled());
    return cloned;
}

} // namespace sketch
} // namespace opencad
