/**
 * @file SketchCircle.h
 * @brief 2D Circle entity for sketches
 */

#pragma once

#include "SketchEntity.h"
#include <gp_Pnt2d.hxx>
#include <gp_Circ2d.hxx>
#include <Geom2d_Circle.hxx>

namespace opencad {
namespace sketch {

/**
 * @brief 2D Circle entity
 * 
 * A circle has 3 degrees of freedom: center(x,y) and radius
 */
class SketchCircle : public SketchEntity {
public:
    using Ptr = std::shared_ptr<SketchCircle>;
    
    SketchCircle();
    SketchCircle(const gp_Pnt2d& center, double radius);
    SketchCircle(double cx, double cy, double radius);
    
    // Type
    EntityType type() const override { return EntityType::Circle; }
    std::string typeName() const override { return "Circle"; }
    
    // Center and radius
    gp_Pnt2d center() const { return m_center; }
    double radius() const { return m_radius; }
    void setCenter(const gp_Pnt2d& center) { m_center = center; }
    void setCenter(double x, double y) { m_center.SetCoord(x, y); }
    void setRadius(double radius) { m_radius = radius; }
    
    // Geometry
    Handle(Geom2d_Curve) curve() const override;
    gp_Pnt2d startPoint() const override; // Point at 0 degrees
    gp_Pnt2d endPoint() const override;   // Same as start (closed curve)
    gp_Pnt2d midPoint() const override;   // Point at 180 degrees
    double length() const override;       // Circumference
    
    gp_Circ2d circle2d() const;
    double diameter() const { return 2.0 * m_radius; }
    double area() const;
    
    // DOF: center(x,y) + radius = 3
    int baseDOF() const override { return 3; }
    
    // Parameters
    int parameterCount() const override { return 3; }
    double getParameter(int index) const override;
    void setParameter(int index, double value) override;
    
    // Clone
    SketchEntity::Ptr clone() const override;
    
    // Validation
    bool isValid() const override;
    
    // Utilities
    gp_Pnt2d pointAtAngle(double angle) const;
    bool containsPoint(const gp_Pnt2d& point) const;
    double distanceToPoint(const gp_Pnt2d& point) const;
    
private:
    gp_Pnt2d m_center;
    double m_radius;
};

} // namespace sketch
} // namespace opencad
