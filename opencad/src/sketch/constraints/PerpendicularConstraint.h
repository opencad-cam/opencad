/**
 * @file PerpendicularConstraint.h
 * @brief Perpendicular constraint - two lines at 90 degrees
 */
#pragma once

#include "../entities/SketchLine.h"
#include "Constraint.h"


namespace opencad {
namespace sketch {

/**
 * @brief Constrains two lines to be perpendicular (90 degrees)
 *
 * The constraint ensures the dot product of direction vectors equals zero.
 * Removes 1 DOF.
 */
class PerpendicularConstraint : public Constraint {
public:
  using Ptr = std::shared_ptr<PerpendicularConstraint>;

  PerpendicularConstraint();
  PerpendicularConstraint(SketchLine::Ptr line1, SketchLine::Ptr line2);

  ConstraintType type() const override { return ConstraintType::Perpendicular; }
  std::string typeName() const override { return "Perpendicular"; }

  std::vector<SketchEntity::Ptr> entities() const override;
  int entityCount() const override { return 2; }
  int dofRemoved() const override { return 1; }

  /**
   * @brief Compute constraint error
   * @return Dot product of normalized direction vectors (should be 0 for
   * perpendicular)
   */
  double error() const override;

  /**
   * @brief Compute Jacobian for Newton-Raphson solver
   * @return Partial derivatives with respect to line endpoint coordinates
   */
  std::vector<double> jacobian() const override;

  Constraint::Ptr clone() const override;

  // Accessors
  SketchLine::Ptr line1() const { return m_line1; }
  SketchLine::Ptr line2() const { return m_line2; }

private:
  SketchLine::Ptr m_line1;
  SketchLine::Ptr m_line2;
};

} // namespace sketch
} // namespace opencad
