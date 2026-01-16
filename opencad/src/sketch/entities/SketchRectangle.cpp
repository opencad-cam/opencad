/**
 * @file SketchRectangle.cpp
 * @brief Implementation of SketchRectangle
 */

#include "SketchRectangle.h"
#include <cmath>
#include <algorithm>

namespace opencad {
namespace sketch {

SketchRectangle::SketchRectangle()
    : SketchEntity()
    , m_corner1(0.0, 0.0)
    , m_corner2(1.0, 1.0)
{
}

SketchRectangle::SketchRectangle(const gp_Pnt2d& corner1, const gp_Pnt2d& corner2)
    : SketchEntity()
    , m_corner1(corner1)
    , m_corner2(corner2)
{
}

SketchRectangle::SketchRectangle(double x, double y, double width, double height)
    : SketchEntity()
    , m_corner1(x, y)
    , m_corner2(x + width, y + height)
{
}

gp_Pnt2d SketchRectangle::corner3() const {
    return gp_Pnt2d(m_corner2.X(), m_corner1.Y());
}

gp_Pnt2d SketchRectangle::corner4() const {
    return gp_Pnt2d(m_corner1.X(), m_corner2.Y());
}

std::array<gp_Pnt2d, 4> SketchRectangle::corners() const {
    return {m_corner1, corner3(), m_corner2, corner4()};
}

double SketchRectangle::width() const {
    return std::abs(m_corner2.X() - m_corner1.X());
}

double SketchRectangle::height() const {
    return std::abs(m_corner2.Y() - m_corner1.Y());
}

gp_Pnt2d SketchRectangle::center() const {
    return gp_Pnt2d(
        (m_corner1.X() + m_corner2.X()) / 2.0,
        (m_corner1.Y() + m_corner2.Y()) / 2.0
    );
}

double SketchRectangle::area() const {
    return width() * height();
}

double SketchRectangle::length() const {
    return 2.0 * (width() + height());
}

std::array<SketchLine::Ptr, 4> SketchRectangle::lines() const {
    auto c = corners();
    return {
        std::make_shared<SketchLine>(c[0], c[1]), // Bottom
        std::make_shared<SketchLine>(c[1], c[2]), // Right
        std::make_shared<SketchLine>(c[2], c[3]), // Top
        std::make_shared<SketchLine>(c[3], c[0])  // Left
    };
}

double SketchRectangle::getParameter(int index) const {
    switch (index) {
        case 0: return m_corner1.X();
        case 1: return m_corner1.Y();
        case 2: return m_corner2.X();
        case 3: return m_corner2.Y();
        default: return 0.0;
    }
}

void SketchRectangle::setParameter(int index, double value) {
    switch (index) {
        case 0: m_corner1.SetX(value); break;
        case 1: m_corner1.SetY(value); break;
        case 2: m_corner2.SetX(value); break;
        case 3: m_corner2.SetY(value); break;
    }
}

SketchEntity::Ptr SketchRectangle::clone() const {
    auto cloned = std::make_shared<SketchRectangle>(m_corner1, m_corner2);
    cloned->setConstruction(isConstruction());
    return cloned;
}

bool SketchRectangle::isValid() const {
    return width() > 1e-10 && height() > 1e-10;
}

} // namespace sketch
} // namespace opencad
