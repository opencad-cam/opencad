/**
 * @file DimensionConstraint.h
 * @brief Dimensional constraints (distance, angle, radius)
 */

#pragma once

#include "Constraint.h"
#include "../entities/SketchEntity.h"
#include "../entities/SketchPoint.h"
#include "../entities/SketchLine.h"
#include "../entities/SketchCircle.h"
#include "../entities/SketchArc.h"

namespace opencad {
namespace sketch {

/**
 * @brief Dimension type
 */
enum class DimensionType {
    PointToPoint,     // Distance between two points
    PointToLine,      // Distance from point to line
    LineLength,       // Length of a line
    LineToLine,       // Distance between parallel lines
    Radius,           // Circle/Arc radius
    Diameter,         // Circle/Arc diameter
    Angle             // Angle between two lines
};

/**
 * @brief Dimensional constraint - drives geometric dimensions
 */
class DimensionConstraint : public Constraint {
public:
    using Ptr = std::shared_ptr<DimensionConstraint>;
    
    DimensionConstraint();
    
    // Point-to-point distance
    static Ptr createPointToPoint(SketchPoint::Ptr p1, SketchPoint::Ptr p2, double distance);
    
    // Line length
    static Ptr createLineLength(SketchLine::Ptr line, double length);
    
    // Radius
    static Ptr createRadius(SketchCircle::Ptr circle, double radius);
    static Ptr createRadius(SketchArc::Ptr arc, double radius);
    
    // Angle between lines
    static Ptr createAngle(SketchLine::Ptr line1, SketchLine::Ptr line2, double angleDegrees);
    
    ConstraintType type() const override { return ConstraintType::Distance; }
    std::string typeName() const override;
    
    // Dimension
    bool hasDimension() const override { return true; }
    double dimension() const override { return m_dimension; }
    void setDimension(double value) override { m_dimension = value; }
    
    DimensionType dimensionType() const { return m_dimType; }
    
    std::vector<SketchEntity::Ptr> entities() const override;
    int entityCount() const override;
    int dofRemoved() const override { return 1; }
    
    double error() const override;
    std::vector<double> jacobian() const override;
    Constraint::Ptr clone() const override;
    
    // Current measured value (before constraint is applied)
    double measuredValue() const;
    
private:
    DimensionType m_dimType;
    double m_dimension;
    
    SketchPoint::Ptr m_point1;
    SketchPoint::Ptr m_point2;
    SketchLine::Ptr m_line1;
    SketchLine::Ptr m_line2;
    SketchCircle::Ptr m_circle;
    SketchArc::Ptr m_arc;
};

} // namespace sketch
} // namespace opencad
