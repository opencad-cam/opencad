/**
 * @file ParallelConstraint.h
 * @brief Parallel constraint - two lines are parallel
 */
#pragma once

#include "../entities/SketchLine.h"
#include "Constraint.h"


namespace opencad {
namespace sketch {

/**
 * @brief Constrains two lines to be parallel
 *
 * Parallel lines have direction vectors that are scalar multiples of each
 * other. The constraint ensures the cross product of direction vectors equals
 * zero. Removes 1 DOF.
 */
class ParallelConstraint : public Constraint {
public:
  using Ptr = std::shared_ptr<ParallelConstraint>;

  ParallelConstraint();
  ParallelConstraint(SketchLine::Ptr line1, SketchLine::Ptr line2);

  ConstraintType type() const override { return ConstraintType::Parallel; }
  std::string typeName() const override { return "Parallel"; }

  std::vector<SketchEntity::Ptr> entities() const override;
  int entityCount() const override { return 2; }
  int dofRemoved() const override { return 1; }

  /**
   * @brief Compute constraint error
   * @return Cross product of normalized direction vectors (should be 0 for
   * parallel)
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
