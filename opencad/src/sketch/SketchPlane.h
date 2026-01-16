/**
 * @file SketchPlane.h
 * @brief Sketch plane definition
 */

#pragma once

#include <gp_Pln.hxx>
#include <gp_Ax3.hxx>
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <string>

namespace opencad {
namespace sketch {

/**
 * @brief Predefined sketch plane orientations
 */
enum class PlaneOrientation {
    XY_Front,   // Front plane (Z normal)
    XZ_Top,     // Top plane (Y normal)
    YZ_Right,   // Right plane (X normal)
    Custom      // User-defined plane
};

/**
 * @brief Defines the plane on which a sketch lies
 */
class SketchPlane {
public:
    SketchPlane();
    SketchPlane(PlaneOrientation orientation);
    SketchPlane(const gp_Pln& plane);
    SketchPlane(const gp_Pnt& origin, const gp_Dir& normal);
    SketchPlane(const gp_Pnt& origin, const gp_Dir& normal, const gp_Dir& xDir);
    
    // Get underlying OCCT plane
    gp_Pln plane() const { return m_plane; }
    gp_Ax3 axis() const;
    
    // Origin and directions
    gp_Pnt origin() const;
    gp_Dir normal() const;
    gp_Dir xDirection() const;
    gp_Dir yDirection() const;
    
    // Set plane
    void setPlane(const gp_Pln& plane) { m_plane = plane; }
    void setOrigin(const gp_Pnt& origin);
    void setOrientation(PlaneOrientation orientation);
    
    // Transform 2D point to 3D point on plane
    gp_Pnt to3D(double x, double y) const;
    gp_Pnt to3D(const gp_Pnt2d& point2d) const;
    
    // Project 3D point onto plane (returns 2D coords)
    gp_Pnt2d to2D(const gp_Pnt& point3d) const;
    
    // Distance from point to plane
    double distance(const gp_Pnt& point) const;
    
    // Offset plane
    SketchPlane offset(double distance) const;
    
    PlaneOrientation orientation() const { return m_orientation; }
    std::string orientationName() const;
    
private:
    gp_Pln m_plane;
    PlaneOrientation m_orientation;
};

} // namespace sketch
} // namespace opencad
