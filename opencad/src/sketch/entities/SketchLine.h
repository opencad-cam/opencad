/**
 * @file SketchLine.h
 * @brief 2D Line segment entity for sketches
 */

#pragma once

#include "SketchEntity.h"
#include "SketchPoint.h"
#include <gp_Pnt2d.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Lin2d.hxx>
#include <Geom2d_Line.hxx>
#include <Geom2d_TrimmedCurve.hxx>

namespace opencad {
namespace sketch {

/**
 * @brief 2D Line segment entity
 * 
 * A line segment has 4 degrees of freedom (x1, y1, x2, y2).
 * Can be constrained to horizontal, vertical, fixed length, etc.
 */
class SketchLine : public SketchEntity {
public:
    using Ptr = std::shared_ptr<SketchLine>;
    
    SketchLine();
    SketchLine(double x1, double y1, double x2, double y2);
    SketchLine(const gp_Pnt2d& start, const gp_Pnt2d& end);
    SketchLine(const SketchPoint::Ptr& start, const SketchPoint::Ptr& end);
    
    // Type
    EntityType type() const override { return EntityType::Line; }
    std::string typeName() const override { return "Line"; }
    
    // Endpoints
    gp_Pnt2d startPoint() const override { return m_start; }
    gp_Pnt2d endPoint() const override { return m_end; }
    gp_Pnt2d midPoint() const override;
    
    void setStartPoint(const gp_Pnt2d& pt) { m_start = pt; }
    void setEndPoint(const gp_Pnt2d& pt) { m_end = pt; }
    void setStartPoint(double x, double y) { m_start.SetCoord(x, y); }
    void setEndPoint(double x, double y) { m_end.SetCoord(x, y); }
    
    // Geometry
    Handle(Geom2d_Curve) curve() const override;
    gp_Lin2d line2d() const;
    gp_Dir2d direction() const;
    double length() const override;
    double angle() const; // Angle from X-axis in radians
    
    // DOF: start(x,y) + end(x,y) = 4
    int baseDOF() const override { return 4; }
    
    // Parameters: [0]=x1, [1]=y1, [2]=x2, [3]=y2
    int parameterCount() const override { return 4; }
    double getParameter(int index) const override;
    void setParameter(int index, double value) override;
    
    // Clone
    SketchEntity::Ptr clone() const override;
    
    // Validation
    bool isValid() const override;
    
    // Utilities
    bool isHorizontal(double tolerance = 1e-6) const;
    bool isVertical(double tolerance = 1e-6) const;
    double distanceToPoint(const gp_Pnt2d& point) const;
    gp_Pnt2d closestPoint(const gp_Pnt2d& point) const;
    
    // Intersection
    bool intersects(const SketchLine& other, gp_Pnt2d& intersection) const;
    
private:
    gp_Pnt2d m_start;
    gp_Pnt2d m_end;
};

} // namespace sketch
} // namespace opencad
