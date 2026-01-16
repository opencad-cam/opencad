/**
 * @file SketchEllipse.h
 * @brief 2D Ellipse entity for sketches
 */

#pragma once

#include "SketchEntity.h"
#include <gp_Pnt2d.hxx>
#include <gp_Elips2d.hxx>
#include <Geom2d_Ellipse.hxx>

namespace opencad {
namespace sketch {

/**
 * @brief 2D Ellipse entity
 * 
 * An ellipse has 5 DOF: center(x,y), major radius, minor radius, rotation angle
 */
class SketchEllipse : public SketchEntity {
public:
    using Ptr = std::shared_ptr<SketchEllipse>;
    
    SketchEllipse();
    SketchEllipse(const gp_Pnt2d& center, double majorRadius, double minorRadius, double angle = 0.0);
    
    // Type
    EntityType type() const override { return EntityType::Ellipse; }
    std::string typeName() const override { return "Ellipse"; }
    
    // Properties
    gp_Pnt2d center() const { return m_center; }
    double majorRadius() const { return m_majorRadius; }
    double minorRadius() const { return m_minorRadius; }
    double rotationAngle() const { return m_rotationAngle; }
    
    void setCenter(const gp_Pnt2d& center) { m_center = center; }
    void setMajorRadius(double r) { m_majorRadius = r; }
    void setMinorRadius(double r) { m_minorRadius = r; }
    void setRotationAngle(double angle) { m_rotationAngle = angle; }
    
    // Focus points
    gp_Pnt2d focus1() const;
    gp_Pnt2d focus2() const;
    double focalDistance() const;
    
    // Geometry
    Handle(Geom2d_Curve) curve() const override;
    gp_Pnt2d startPoint() const override;
    gp_Pnt2d endPoint() const override;
    gp_Pnt2d midPoint() const override;
    double length() const override; // Approximate perimeter
    
    gp_Elips2d ellipse2d() const;
    gp_Pnt2d pointAtAngle(double angle) const;
    double area() const;
    
    // DOF: center(x,y) + majorRadius + minorRadius + angle = 5
    int baseDOF() const override { return 5; }
    
    // Parameters
    int parameterCount() const override { return 5; }
    double getParameter(int index) const override;
    void setParameter(int index, double value) override;
    
    // Clone
    SketchEntity::Ptr clone() const override;
    
    // Validation
    bool isValid() const override;
    
private:
    gp_Pnt2d m_center;
    double m_majorRadius;
    double m_minorRadius;
    double m_rotationAngle;
};

} // namespace sketch
} // namespace opencad
