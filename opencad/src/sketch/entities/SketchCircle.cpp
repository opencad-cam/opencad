/**
 * @file SketchCircle.cpp
 * @brief Implementation of SketchCircle
 */

#include "SketchCircle.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace opencad {
namespace sketch {

SketchCircle::SketchCircle()
    : SketchEntity()
    , m_center(0.0, 0.0)
    , m_radius(1.0)
{
}

SketchCircle::SketchCircle(const gp_Pnt2d& center, double radius)
    : SketchEntity()
    , m_center(center)
    , m_radius(radius)
{
}

SketchCircle::SketchCircle(double cx, double cy, double radius)
    : SketchEntity()
    , m_center(cx, cy)
    , m_radius(radius)
{
}

Handle(Geom2d_Curve) SketchCircle::curve() const {
    if (!isValid()) {
        return Handle(Geom2d_Curve)();
    }
    
    gp_Circ2d circ = circle2d();
    return new Geom2d_Circle(circ);
}

gp_Circ2d SketchCircle::circle2d() const {
    gp_Ax2d axis(m_center, gp_Dir2d(1.0, 0.0));
    return gp_Circ2d(axis, m_radius);
}

gp_Pnt2d SketchCircle::startPoint() const {
    return pointAtAngle(0.0);
}

gp_Pnt2d SketchCircle::endPoint() const {
    return pointAtAngle(0.0); // Closed curve
}

gp_Pnt2d SketchCircle::midPoint() const {
    return pointAtAngle(M_PI);
}

double SketchCircle::length() const {
    return 2.0 * M_PI * m_radius;
}

double SketchCircle::area() const {
    return M_PI * m_radius * m_radius;
}

gp_Pnt2d SketchCircle::pointAtAngle(double angle) const {
    return gp_Pnt2d(
        m_center.X() + m_radius * std::cos(angle),
        m_center.Y() + m_radius * std::sin(angle)
    );
}

bool SketchCircle::containsPoint(const gp_Pnt2d& point) const {
    return m_center.Distance(point) <= m_radius;
}

double SketchCircle::distanceToPoint(const gp_Pnt2d& point) const {
    double dist = m_center.Distance(point);
    return std::abs(dist - m_radius);
}

double SketchCircle::getParameter(int index) const {
    switch (index) {
        case 0: return m_center.X();
        case 1: return m_center.Y();
        case 2: return m_radius;
        default: return 0.0;
    }
}

void SketchCircle::setParameter(int index, double value) {
    switch (index) {
        case 0: m_center.SetX(value); break;
        case 1: m_center.SetY(value); break;
        case 2: m_radius = value; break;
    }
}

SketchEntity::Ptr SketchCircle::clone() const {
    auto cloned = std::make_shared<SketchCircle>(m_center, m_radius);
    cloned->setConstruction(isConstruction());
    return cloned;
}

bool SketchCircle::isValid() const {
    return m_radius > 1e-10;
}

} // namespace sketch
} // namespace opencad
