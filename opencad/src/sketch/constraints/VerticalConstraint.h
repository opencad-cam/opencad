/**
 * @file VerticalConstraint.h
 * @brief Vertical constraint for sketch lines
 */

#pragma once

#include "Constraint.h"
#include "../entities/SketchLine.h"

namespace opencad {
namespace sketch {

/**
 * @brief Constrains a line to be vertical (parallel to Y axis)
 */
class VerticalConstraint : public Constraint {
public:
    using Ptr = std::shared_ptr<VerticalConstraint>;
    
    VerticalConstraint();
    explicit VerticalConstraint(SketchLine::Ptr line);
    
    ConstraintType type() const override { return ConstraintType::Vertical; }
    std::string typeName() const override { return "Vertical"; }
    
    SketchLine::Ptr line() const { return m_line; }
    void setLine(SketchLine::Ptr line) { m_line = line; }
    
    std::vector<SketchEntity::Ptr> entities() const override;
    int entityCount() const override { return 1; }
    int dofRemoved() const override { return 1; }
    
    double error() const override;
    std::vector<double> jacobian() const override;
    Constraint::Ptr clone() const override;
    
private:
    SketchLine::Ptr m_line;
};

} // namespace sketch
} // namespace opencad
