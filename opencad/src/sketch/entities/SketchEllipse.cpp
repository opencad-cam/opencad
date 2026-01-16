/**
 * @file SketchEllipse.cpp
 * @brief Implementation of SketchEllipse
 */

#include "SketchEllipse.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace opencad {
namespace sketch {

SketchEllipse::SketchEllipse()
    : SketchEntity()
    , m_center(0.0, 0.0)
    , m_majorRadius(2.0)
    , m_minorRadius(1.0)
    , m_rotationAngle(0.0)
{
}

SketchEllipse::SketchEllipse(const gp_Pnt2d& center, double majorRadius, double minorRadius, double angle)
    : SketchEntity()
    , m_center(center)
    , m_majorRadius(majorRadius)
    , m_minorRadius(minorRadius)
    , m_rotationAngle(angle)
{
}

gp_Pnt2d SketchEllipse::focus1() const {
    double c = focalDistance();
    return gp_Pnt2d(
        m_center.X() + c * std::cos(m_rotationAngle),
        m_center.Y() + c * std::sin(m_rotationAngle)
    );
}

gp_Pnt2d SketchEllipse::focus2() const {
    double c = focalDistance();
    return gp_Pnt2d(
        m_center.X() - c * std::cos(m_rotationAngle),
        m_center.Y() - c * std::sin(m_rotationAngle)
    );
}

double SketchEllipse::focalDistance() const {
    double a = std::max(m_majorRadius, m_minorRadius);
    double b = std::min(m_majorRadius, m_minorRadius);
    return std::sqrt(a * a - b * b);
}

Handle(Geom2d_Curve) SketchEllipse::curve() const {
    if (!isValid()) {
        return Handle(Geom2d_Curve)();
    }
    
    gp_Elips2d elips = ellipse2d();
    return new Geom2d_Ellipse(elips);
}

gp_Elips2d SketchEllipse::ellipse2d() const {
    gp_Dir2d majorDir(std::cos(m_rotationAngle), std::sin(m_rotationAngle));
    gp_Ax2d axis(m_center, majorDir);
    return gp_Elips2d(axis, m_majorRadius, m_minorRadius);
}

gp_Pnt2d SketchEllipse::startPoint() const {
    return pointAtAngle(0.0);
}

gp_Pnt2d SketchEllipse::endPoint() const {
    return pointAtAngle(0.0); // Closed curve
}

gp_Pnt2d SketchEllipse::midPoint() const {
    return pointAtAngle(M_PI);
}

gp_Pnt2d SketchEllipse::pointAtAngle(double angle) const {
    double x = m_majorRadius * std::cos(angle);
    double y = m_minorRadius * std::sin(angle);
    
    // Rotate by rotation angle
    double xr = x * std::cos(m_rotationAngle) - y * std::sin(m_rotationAngle);
    double yr = x * std::sin(m_rotationAngle) + y * std::cos(m_rotationAngle);
    
    return gp_Pnt2d(m_center.X() + xr, m_center.Y() + yr);
}

double SketchEllipse::length() const {
    // Ramanujan approximation for ellipse perimeter
    double a = m_majorRadius;
    double b = m_minorRadius;
    double h = (a - b) * (a - b) / ((a + b) * (a + b));
    return M_PI * (a + b) * (1.0 + 3.0 * h / (10.0 + std::sqrt(4.0 - 3.0 * h)));
}

double SketchEllipse::area() const {
    return M_PI * m_majorRadius * m_minorRadius;
}

double SketchEllipse::getParameter(int index) const {
    switch (index) {
        case 0: return m_center.X();
        case 1: return m_center.Y();
        case 2: return m_majorRadius;
        case 3: return m_minorRadius;
        case 4: return m_rotationAngle;
        default: return 0.0;
    }
}

void SketchEllipse::setParameter(int index, double value) {
    switch (index) {
        case 0: m_center.SetX(value); break;
        case 1: m_center.SetY(value); break;
        case 2: m_majorRadius = value; break;
        case 3: m_minorRadius = value; break;
        case 4: m_rotationAngle = value; break;
    }
}

SketchEntity::Ptr SketchEllipse::clone() const {
    auto cloned = std::make_shared<SketchEllipse>(m_center, m_majorRadius, m_minorRadius, m_rotationAngle);
    cloned->setConstruction(isConstruction());
    return cloned;
}

bool SketchEllipse::isValid() const {
    return m_majorRadius > 1e-10 && m_minorRadius > 1e-10;
}

} // namespace sketch
} // namespace opencad
