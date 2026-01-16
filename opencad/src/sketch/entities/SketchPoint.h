/**
 * @file SketchPoint.h
 * @brief 2D Point entity for sketches
 */

#pragma once

#include "SketchEntity.h"
#include <gp_Pnt2d.hxx>

namespace opencad {
namespace sketch {

/**
 * @brief 2D Point entity
 * 
 * A point has 2 degrees of freedom (x, y position).
 * Used as reference points, endpoints, and for coincident constraints.
 */
class SketchPoint : public SketchEntity {
public:
    using Ptr = std::shared_ptr<SketchPoint>;
    
    SketchPoint();
    SketchPoint(double x, double y);
    SketchPoint(const gp_Pnt2d& point);
    
    // Type
    EntityType type() const override { return EntityType::Point; }
    std::string typeName() const override { return "Point"; }
    
    // Position
    gp_Pnt2d position() const { return m_position; }
    void setPosition(const gp_Pnt2d& pos) { m_position = pos; }
    void setPosition(double x, double y) { m_position.SetCoord(x, y); }
    
    double x() const { return m_position.X(); }
    double y() const { return m_position.Y(); }
    void setX(double x) { m_position.SetX(x); }
    void setY(double y) { m_position.SetY(y); }
    
    // Geometry (point has no curve representation)
    Handle(Geom2d_Curve) curve() const override { return Handle(Geom2d_Curve)(); }
    gp_Pnt2d startPoint() const override { return m_position; }
    gp_Pnt2d endPoint() const override { return m_position; }
    gp_Pnt2d midPoint() const override { return m_position; }
    double length() const override { return 0.0; }
    
    // DOF: x and y coordinates
    int baseDOF() const override { return 2; }
    
    // Parameters: [0] = x, [1] = y
    int parameterCount() const override { return 2; }
    double getParameter(int index) const override;
    void setParameter(int index, double value) override;
    
    // Clone
    SketchEntity::Ptr clone() const override;
    
    // Validation
    bool isValid() const override { return true; }
    
    // Distance to another point
    double distanceTo(const SketchPoint& other) const;
    double distanceTo(const gp_Pnt2d& point) const;
    
private:
    gp_Pnt2d m_position;
};

} // namespace sketch
} // namespace opencad
