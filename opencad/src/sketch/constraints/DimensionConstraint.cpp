/**
 * @file DimensionConstraint.cpp
 * @brief Implementation of DimensionConstraint
 */

#include "DimensionConstraint.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace opencad {
namespace sketch {

DimensionConstraint::DimensionConstraint()
    : Constraint()
    , m_dimType(DimensionType::PointToPoint)
    , m_dimension(0.0)
{
}

DimensionConstraint::Ptr DimensionConstraint::createPointToPoint(
    SketchPoint::Ptr p1, SketchPoint::Ptr p2, double distance) {
    auto c = std::make_shared<DimensionConstraint>();
    c->m_dimType = DimensionType::PointToPoint;
    c->m_point1 = p1;
    c->m_point2 = p2;
    c->m_dimension = distance;
    return c;
}

DimensionConstraint::Ptr DimensionConstraint::createLineLength(
    SketchLine::Ptr line, double length) {
    auto c = std::make_shared<DimensionConstraint>();
    c->m_dimType = DimensionType::LineLength;
    c->m_line1 = line;
    c->m_dimension = length;
    return c;
}

DimensionConstraint::Ptr DimensionConstraint::createRadius(
    SketchCircle::Ptr circle, double radius) {
    auto c = std::make_shared<DimensionConstraint>();
    c->m_dimType = DimensionType::Radius;
    c->m_circle = circle;
    c->m_dimension = radius;
    return c;
}

DimensionConstraint::Ptr DimensionConstraint::createRadius(
    SketchArc::Ptr arc, double radius) {
    auto c = std::make_shared<DimensionConstraint>();
    c->m_dimType = DimensionType::Radius;
    c->m_arc = arc;
    c->m_dimension = radius;
    return c;
}

DimensionConstraint::Ptr DimensionConstraint::createAngle(
    SketchLine::Ptr line1, SketchLine::Ptr line2, double angleDegrees) {
    auto c = std::make_shared<DimensionConstraint>();
    c->m_dimType = DimensionType::Angle;
    c->m_line1 = line1;
    c->m_line2 = line2;
    c->m_dimension = angleDegrees * M_PI / 180.0; // Store in radians
    return c;
}

std::string DimensionConstraint::typeName() const {
    switch (m_dimType) {
        case DimensionType::PointToPoint: return "Distance";
        case DimensionType::PointToLine: return "Distance";
        case DimensionType::LineLength: return "Length";
        case DimensionType::LineToLine: return "Distance";
        case DimensionType::Radius: return "Radius";
        case DimensionType::Diameter: return "Diameter";
        case DimensionType::Angle: return "Angle";
        default: return "Dimension";
    }
}

std::vector<SketchEntity::Ptr> DimensionConstraint::entities() const {
    std::vector<SketchEntity::Ptr> result;
    if (m_point1) result.push_back(m_point1);
    if (m_point2) result.push_back(m_point2);
    if (m_line1) result.push_back(m_line1);
    if (m_line2) result.push_back(m_line2);
    if (m_circle) result.push_back(m_circle);
    if (m_arc) result.push_back(m_arc);
    return result;
}

int DimensionConstraint::entityCount() const {
    int count = 0;
    if (m_point1) count++;
    if (m_point2) count++;
    if (m_line1) count++;
    if (m_line2) count++;
    if (m_circle) count++;
    if (m_arc) count++;
    return count;
}

double DimensionConstraint::measuredValue() const {
    switch (m_dimType) {
        case DimensionType::PointToPoint:
            if (m_point1 && m_point2) {
                return m_point1->position().Distance(m_point2->position());
            }
            break;
            
        case DimensionType::LineLength:
            if (m_line1) {
                return m_line1->length();
            }
            break;
            
        case DimensionType::Radius:
            if (m_circle) return m_circle->radius();
            if (m_arc) return m_arc->radius();
            break;
            
        case DimensionType::Angle:
            if (m_line1 && m_line2) {
                double a1 = m_line1->angle();
                double a2 = m_line2->angle();
                return std::abs(a2 - a1);
            }
            break;
            
        default:
            break;
    }
    return 0.0;
}

double DimensionConstraint::error() const {
    return measuredValue() - m_dimension;
}

std::vector<double> DimensionConstraint::jacobian() const {
    // Simplified - full implementation requires chain rule through entities
    switch (m_dimType) {
        case DimensionType::PointToPoint:
            if (m_point1 && m_point2) {
                double dx = m_point2->x() - m_point1->x();
                double dy = m_point2->y() - m_point1->y();
                double d = std::sqrt(dx*dx + dy*dy);
                if (d < 1e-10) d = 1e-10;
                // d(dist)/d(x1,y1,x2,y2)
                return {-dx/d, -dy/d, dx/d, dy/d};
            }
            break;
            
        case DimensionType::LineLength:
            if (m_line1) {
                double dx = m_line1->endPoint().X() - m_line1->startPoint().X();
                double dy = m_line1->endPoint().Y() - m_line1->startPoint().Y();
                double d = m_line1->length();
                if (d < 1e-10) d = 1e-10;
                return {-dx/d, -dy/d, dx/d, dy/d};
            }
            break;
            
        case DimensionType::Radius:
            // d(radius)/d(cx, cy, r) = (0, 0, 1)
            return {0.0, 0.0, 1.0};
            
        default:
            break;
    }
    return {};
}

Constraint::Ptr DimensionConstraint::clone() const {
    auto cloned = std::make_shared<DimensionConstraint>();
    cloned->m_dimType = m_dimType;
    cloned->m_dimension = m_dimension;
    cloned->m_point1 = m_point1;
    cloned->m_point2 = m_point2;
    cloned->m_line1 = m_line1;
    cloned->m_line2 = m_line2;
    cloned->m_circle = m_circle;
    cloned->m_arc = m_arc;
    cloned->setDriving(isDriving());
    cloned->setEnabled(isEnabled());
    return cloned;
}

} // namespace sketch
} // namespace opencad
