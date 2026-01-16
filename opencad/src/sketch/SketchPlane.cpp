/**
 * @file SketchPlane.cpp
 * @brief Implementation of SketchPlane
 */

#include "SketchPlane.h"

namespace opencad {
namespace sketch {

SketchPlane::SketchPlane()
    : m_plane(gp_Pln())
    , m_orientation(PlaneOrientation::XY_Front)
{
}

SketchPlane::SketchPlane(PlaneOrientation orientation)
    : m_orientation(orientation)
{
    setOrientation(orientation);
}

SketchPlane::SketchPlane(const gp_Pln& plane)
    : m_plane(plane)
    , m_orientation(PlaneOrientation::Custom)
{
}

SketchPlane::SketchPlane(const gp_Pnt& origin, const gp_Dir& normal)
    : m_plane(origin, normal)
    , m_orientation(PlaneOrientation::Custom)
{
}

SketchPlane::SketchPlane(const gp_Pnt& origin, const gp_Dir& normal, const gp_Dir& xDir)
    : m_orientation(PlaneOrientation::Custom)
{
    gp_Ax3 ax3(origin, normal, xDir);
    m_plane = gp_Pln(ax3);
}

gp_Ax3 SketchPlane::axis() const {
    return m_plane.Position();
}

gp_Pnt SketchPlane::origin() const {
    return m_plane.Location();
}

gp_Dir SketchPlane::normal() const {
    return m_plane.Axis().Direction();
}

gp_Dir SketchPlane::xDirection() const {
    return m_plane.Position().XDirection();
}

gp_Dir SketchPlane::yDirection() const {
    return m_plane.Position().YDirection();
}

void SketchPlane::setOrigin(const gp_Pnt& origin) {
    m_plane.SetLocation(origin);
}

void SketchPlane::setOrientation(PlaneOrientation orientation) {
    m_orientation = orientation;
    gp_Pnt origin(0, 0, 0);
    
    switch (orientation) {
        case PlaneOrientation::XY_Front:
            m_plane = gp_Pln(gp_Ax3(origin, gp_Dir(0, 0, 1), gp_Dir(1, 0, 0)));
            break;
        case PlaneOrientation::XZ_Top:
            m_plane = gp_Pln(gp_Ax3(origin, gp_Dir(0, 1, 0), gp_Dir(1, 0, 0)));
            break;
        case PlaneOrientation::YZ_Right:
            m_plane = gp_Pln(gp_Ax3(origin, gp_Dir(1, 0, 0), gp_Dir(0, 1, 0)));
            break;
        case PlaneOrientation::Custom:
            // Keep current plane
            break;
    }
}

gp_Pnt SketchPlane::to3D(double x, double y) const {
    gp_Pnt origin = this->origin();
    gp_Dir xDir = xDirection();
    gp_Dir yDir = yDirection();
    
    return gp_Pnt(
        origin.X() + x * xDir.X() + y * yDir.X(),
        origin.Y() + x * xDir.Y() + y * yDir.Y(),
        origin.Z() + x * xDir.Z() + y * yDir.Z()
    );
}

gp_Pnt SketchPlane::to3D(const gp_Pnt2d& point2d) const {
    return to3D(point2d.X(), point2d.Y());
}

gp_Pnt2d SketchPlane::to2D(const gp_Pnt& point3d) const {
    gp_Pnt origin = this->origin();
    gp_Dir xDir = xDirection();
    gp_Dir yDir = yDirection();
    
    gp_Vec vec(origin, point3d);
    double x = vec.Dot(gp_Vec(xDir));
    double y = vec.Dot(gp_Vec(yDir));
    
    return gp_Pnt2d(x, y);
}

double SketchPlane::distance(const gp_Pnt& point) const {
    return m_plane.Distance(point);
}

SketchPlane SketchPlane::offset(double dist) const {
    gp_Pnt newOrigin = origin().Translated(gp_Vec(normal()) * dist);
    return SketchPlane(newOrigin, normal(), xDirection());
}

std::string SketchPlane::orientationName() const {
    switch (m_orientation) {
        case PlaneOrientation::XY_Front: return "Front";
        case PlaneOrientation::XZ_Top: return "Top";
        case PlaneOrientation::YZ_Right: return "Right";
        case PlaneOrientation::Custom: return "Custom";
        default: return "Unknown";
    }
}

} // namespace sketch
} // namespace opencad
