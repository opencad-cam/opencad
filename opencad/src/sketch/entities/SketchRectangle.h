/**
 * @file SketchRectangle.h
 * @brief 2D Rectangle entity for sketches (composed of 4 lines)
 */

#pragma once

#include "SketchEntity.h"
#include "SketchLine.h"
#include <gp_Pnt2d.hxx>
#include <array>

namespace opencad {
namespace sketch {

/**
 * @brief 2D Rectangle entity
 * 
 * A rectangle is defined by two corner points and creates 4 connected lines.
 * 3 DOF: corner position (x,y) + width + height = 4 parameters
 */
class SketchRectangle : public SketchEntity {
public:
    using Ptr = std::shared_ptr<SketchRectangle>;
    
    enum class CreationMode {
        CornerCorner,    // Two opposite corners
        CenterSize,      // Center point + width/height
        ThreePoint       // Three points define corner and two edges
    };
    
    SketchRectangle();
    SketchRectangle(const gp_Pnt2d& corner1, const gp_Pnt2d& corner2);
    SketchRectangle(double x, double y, double width, double height);
    
    // Type
    EntityType type() const override { return EntityType::Rectangle; }
    std::string typeName() const override { return "Rectangle"; }
    
    // Corners
    gp_Pnt2d corner1() const { return m_corner1; }
    gp_Pnt2d corner2() const { return m_corner2; }
    gp_Pnt2d corner3() const;
    gp_Pnt2d corner4() const;
    std::array<gp_Pnt2d, 4> corners() const;
    
    void setCorner1(const gp_Pnt2d& pt) { m_corner1 = pt; }
    void setCorner2(const gp_Pnt2d& pt) { m_corner2 = pt; }
    
    // Dimensions
    double width() const;
    double height() const;
    gp_Pnt2d center() const;
    double area() const;
    
    // Geometry
    Handle(Geom2d_Curve) curve() const override { return Handle(Geom2d_Curve)(); }
    gp_Pnt2d startPoint() const override { return m_corner1; }
    gp_Pnt2d endPoint() const override { return m_corner1; } // Closed
    gp_Pnt2d midPoint() const override { return center(); }
    double length() const override; // Perimeter
    
    // Get the 4 lines
    std::array<SketchLine::Ptr, 4> lines() const;
    
    // DOF: corner1(x,y) + corner2(x,y) = 4
    int baseDOF() const override { return 4; }
    
    // Parameters
    int parameterCount() const override { return 4; }
    double getParameter(int index) const override;
    void setParameter(int index, double value) override;
    
    // Clone
    SketchEntity::Ptr clone() const override;
    
    // Validation
    bool isValid() const override;
    
private:
    gp_Pnt2d m_corner1; // Bottom-left
    gp_Pnt2d m_corner2; // Top-right
};

} // namespace sketch
} // namespace opencad
