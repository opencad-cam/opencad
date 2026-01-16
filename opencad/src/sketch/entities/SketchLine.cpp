/**
 * @file SketchLine.cpp
 * @brief Implementation of SketchLine
 */

#include "SketchLine.h"
#include <cmath>

namespace opencad {
namespace sketch {

SketchLine::SketchLine()
    : SketchEntity()
    , m_start(0.0, 0.0)
    , m_end(1.0, 0.0)
{
}

SketchLine::SketchLine(double x1, double y1, double x2, double y2)
    : SketchEntity()
    , m_start(x1, y1)
    , m_end(x2, y2)
{
}

SketchLine::SketchLine(const gp_Pnt2d& start, const gp_Pnt2d& end)
    : SketchEntity()
    , m_start(start)
    , m_end(end)
{
}

SketchLine::SketchLine(const SketchPoint::Ptr& start, const SketchPoint::Ptr& end)
    : SketchEntity()
    , m_start(start ? start->position() : gp_Pnt2d(0, 0))
    , m_end(end ? end->position() : gp_Pnt2d(1, 0))
{
}

gp_Pnt2d SketchLine::midPoint() const {
    return gp_Pnt2d(
        (m_start.X() + m_end.X()) / 2.0,
        (m_start.Y() + m_end.Y()) / 2.0
    );
}

Handle(Geom2d_Curve) SketchLine::curve() const {
    if (!isValid()) {
        return Handle(Geom2d_Curve)();
    }
    
    gp_Lin2d lin(m_start, direction());
    Handle(Geom2d_Line) geomLine = new Geom2d_Line(lin);
    
    return new Geom2d_TrimmedCurve(geomLine, 0.0, length());
}

gp_Lin2d SketchLine::line2d() const {
    return gp_Lin2d(m_start, direction());
}

gp_Dir2d SketchLine::direction() const {
    gp_Vec2d vec(m_start, m_end);
    if (vec.Magnitude() < 1e-10) {
        return gp_Dir2d(1.0, 0.0); // Default direction
    }
    return gp_Dir2d(vec);
}

double SketchLine::length() const {
    return m_start.Distance(m_end);
}

double SketchLine::angle() const {
    double dx = m_end.X() - m_start.X();
    double dy = m_end.Y() - m_start.Y();
    return std::atan2(dy, dx);
}

double SketchLine::getParameter(int index) const {
    switch (index) {
        case 0: return m_start.X();
        case 1: return m_start.Y();
        case 2: return m_end.X();
        case 3: return m_end.Y();
        default: return 0.0;
    }
}

void SketchLine::setParameter(int index, double value) {
    switch (index) {
        case 0: m_start.SetX(value); break;
        case 1: m_start.SetY(value); break;
        case 2: m_end.SetX(value); break;
        case 3: m_end.SetY(value); break;
    }
}

SketchEntity::Ptr SketchLine::clone() const {
    auto cloned = std::make_shared<SketchLine>(m_start, m_end);
    cloned->setConstruction(isConstruction());
    return cloned;
}

bool SketchLine::isValid() const {
    return length() > 1e-10;
}

bool SketchLine::isHorizontal(double tolerance) const {
    return std::abs(m_start.Y() - m_end.Y()) <= tolerance;
}

bool SketchLine::isVertical(double tolerance) const {
    return std::abs(m_start.X() - m_end.X()) <= tolerance;
}

double SketchLine::distanceToPoint(const gp_Pnt2d& point) const {
    gp_Vec2d lineVec(m_start, m_end);
    gp_Vec2d pointVec(m_start, point);
    
    double len = lineVec.Magnitude();
    if (len < 1e-10) {
        return m_start.Distance(point);
    }
    
    double t = pointVec.Dot(lineVec) / (len * len);
    t = std::max(0.0, std::min(1.0, t)); // Clamp to segment
    
    gp_Pnt2d closest(
        m_start.X() + t * (m_end.X() - m_start.X()),
        m_start.Y() + t * (m_end.Y() - m_start.Y())
    );
    
    return point.Distance(closest);
}

gp_Pnt2d SketchLine::closestPoint(const gp_Pnt2d& point) const {
    gp_Vec2d lineVec(m_start, m_end);
    gp_Vec2d pointVec(m_start, point);
    
    double len = lineVec.Magnitude();
    if (len < 1e-10) {
        return m_start;
    }
    
    double t = pointVec.Dot(lineVec) / (len * len);
    t = std::max(0.0, std::min(1.0, t));
    
    return gp_Pnt2d(
        m_start.X() + t * (m_end.X() - m_start.X()),
        m_start.Y() + t * (m_end.Y() - m_start.Y())
    );
}

bool SketchLine::intersects(const SketchLine& other, gp_Pnt2d& intersection) const {
    double x1 = m_start.X(), y1 = m_start.Y();
    double x2 = m_end.X(), y2 = m_end.Y();
    double x3 = other.m_start.X(), y3 = other.m_start.Y();
    double x4 = other.m_end.X(), y4 = other.m_end.Y();
    
    double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    if (std::abs(denom) < 1e-10) {
        return false; // Parallel lines
    }
    
    double t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
    double u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denom;
    
    if (t >= 0.0 && t <= 1.0 && u >= 0.0 && u <= 1.0) {
        intersection.SetCoord(
            x1 + t * (x2 - x1),
            y1 + t * (y2 - y1)
        );
        return true;
    }
    
    return false;
}

} // namespace sketch
} // namespace opencad
