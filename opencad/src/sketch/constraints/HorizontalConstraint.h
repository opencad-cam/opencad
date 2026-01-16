/**
 * @file HorizontalConstraint.h
 * @brief Horizontal constraint for sketch lines
 */

#pragma once

#include "Constraint.h"
#include "../entities/SketchLine.h"

namespace opencad {
namespace sketch {

/**
 * @brief Constrains a line to be horizontal (parallel to X axis)
 * 
 * Removes 1 DOF (the angle of the line)
 */
class HorizontalConstraint : public Constraint {
public:
    using Ptr = std::shared_ptr<HorizontalConstraint>;
    
    HorizontalConstraint();
    explicit HorizontalConstraint(SketchLine::Ptr line);
    
    // Type
    ConstraintType type() const override { return ConstraintType::Horizontal; }
    std::string typeName() const override { return "Horizontal"; }
    
    // Entity
    SketchLine::Ptr line() const { return m_line; }
    void setLine(SketchLine::Ptr line) { m_line = line; }
    
    std::vector<SketchEntity::Ptr> entities() const override;
    int entityCount() const override { return 1; }
    
    // DOF
    int dofRemoved() const override { return 1; }
    
    // Error: difference in Y coordinates
    double error() const override;
    
    // Jacobian: d(error)/d(parameters)
    std::vector<double> jacobian() const override;
    
    // Clone
    Constraint::Ptr clone() const override;
    
private:
    SketchLine::Ptr m_line;
};

} // namespace sketch
} // namespace opencad
