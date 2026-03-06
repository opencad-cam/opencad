/**
 * @file FixConstraint.h
 * @brief Fix constraint - locks entity in place
 */
#pragma once

#include "../entities/SketchArc.h"
#include "../entities/SketchCircle.h"
#include "../entities/SketchLine.h"
#include "../entities/SketchPoint.h"
#include "Constraint.h"

namespace opencad {
namespace sketch {

/**
 * @brief Constrains an entity to remain fixed at its current position
 *
 * For points: fixes X and Y coordinates (removes 2 DOF)
 * For lines: fixes all 4 coordinates (removes 4 DOF)
 * For circles: fixes center and radius (removes 3 DOF)
 */
class FixConstraint : public Constraint {
public:
  using Ptr = std::shared_ptr<FixConstraint>;

  FixConstraint();

  // Fix a point
  explicit FixConstraint(SketchPoint::Ptr point);

  // Fix a line
  explicit FixConstraint(SketchLine::Ptr line);

  // Fix a circle
  explicit FixConstraint(SketchCircle::Ptr circle);

  // Fix an arc
  explicit FixConstraint(SketchArc::Ptr arc);

  ConstraintType type() const override { return ConstraintType::Fix; }
  std::string typeName() const override { return "Fix"; }

  std::vector<SketchEntity::Ptr> entities() const override;
  int entityCount() const override { return 1; }
  int dofRemoved() const override;

  /**
   * @brief Compute constraint error vector
   */
  std::vector<double> errorVector() const override;
  double error() const override;

  /**
   * @brief Compute Jacobian for Newton-Raphson solver
   */
  std::vector<double> jacobian() const override;

  Constraint::Ptr clone() const override;

  // Get fixed position values
  double fixedX() const { return m_fixedX; }
  double fixedY() const { return m_fixedY; }
  double fixedX2() const { return m_fixedX2; } // For line end point
  double fixedY2() const { return m_fixedY2; }
  double fixedRadius() const { return m_fixedRadius; }

private:
  SketchEntity::Ptr m_entity;

  // Fixed values (stored when constraint is created)
  double m_fixedX = 0.0;
  double m_fixedY = 0.0;
  double m_fixedX2 = 0.0; // For line end point
  double m_fixedY2 = 0.0;
  double m_fixedRadius = 0.0;

  void capturePosition();
};

} // namespace sketch
} // namespace opencad
