/**
 * @file CoincidentConstraint.h
 * @brief Coincident constraint - two points at same location
 */

#pragma once

#include "Constraint.h"
#include "../entities/SketchPoint.h"
#include "../entities/SketchLine.h"

namespace opencad {
namespace sketch {

/**
 * @brief Constrains two points to be at the same location
 * 
 * Removes 2 DOF (matches both X and Y coordinates)
 */
class CoincidentConstraint : public Constraint {
public:
    using Ptr = std::shared_ptr<CoincidentConstraint>;
    
    CoincidentConstraint();
    CoincidentConstraint(SketchPoint::Ptr point1, SketchPoint::Ptr point2);
    
    // Point-to-endpoint: which endpoint (0=start, 1=end)
    CoincidentConstraint(SketchPoint::Ptr point, SketchLine::Ptr line, int endpoint);
    
    // Endpoint-to-endpoint
    CoincidentConstraint(SketchLine::Ptr line1, int endpoint1, 
                         SketchLine::Ptr line2, int endpoint2);
    
    ConstraintType type() const override { return ConstraintType::Coincident; }
    std::string typeName() const override { return "Coincident"; }
    
    std::vector<SketchEntity::Ptr> entities() const override;
    int entityCount() const override { return 2; }
    int dofRemoved() const override { return 2; }
    
    // Returns two errors: dx and dy
    double error() const override;
    double errorX() const;
    double errorY() const;
    
    std::vector<double> jacobian() const override;
    Constraint::Ptr clone() const override;
    
    // Get the two points being constrained
    gp_Pnt2d point1() const;
    gp_Pnt2d point2() const;
    
private:
    SketchPoint::Ptr m_point1;
    SketchPoint::Ptr m_point2;
    SketchLine::Ptr m_line1;
    SketchLine::Ptr m_line2;
    int m_endpoint1;  // 0 = start, 1 = end
    int m_endpoint2;
};

} // namespace sketch
} // namespace opencad
