/**
 * @file SketchPoint.cpp
 * @brief Implementation of SketchPoint
 */

#include "SketchPoint.h"

namespace opencad {
namespace sketch {

SketchPoint::SketchPoint()
    : SketchEntity()
    , m_position(0.0, 0.0)
{
}

SketchPoint::SketchPoint(double x, double y)
    : SketchEntity()
    , m_position(x, y)
{
}

SketchPoint::SketchPoint(const gp_Pnt2d& point)
    : SketchEntity()
    , m_position(point)
{
}

double SketchPoint::getParameter(int index) const {
    switch (index) {
        case 0: return m_position.X();
        case 1: return m_position.Y();
        default: return 0.0;
    }
}

void SketchPoint::setParameter(int index, double value) {
    switch (index) {
        case 0: m_position.SetX(value); break;
        case 1: m_position.SetY(value); break;
    }
}

SketchEntity::Ptr SketchPoint::clone() const {
    auto cloned = std::make_shared<SketchPoint>(m_position);
    cloned->setConstruction(isConstruction());
    return cloned;
}

double SketchPoint::distanceTo(const SketchPoint& other) const {
    return m_position.Distance(other.m_position);
}

double SketchPoint::distanceTo(const gp_Pnt2d& point) const {
    return m_position.Distance(point);
}

} // namespace sketch
} // namespace opencad
