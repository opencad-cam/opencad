/**
 * @file EqualConstraint.h
 * @brief Equal constraint - two entities have equal measure (length/radius)
 */
#pragma once

#include "../entities/SketchArc.h"
#include "../entities/SketchCircle.h"
#include "../entities/SketchLine.h"
#include "Constraint.h"


namespace opencad {
namespace sketch {

/**
 * @brief Constraint type for equal measure
 */
enum class EqualType {
  LineToLine,     // Equal line lengths
  CircleToCircle, // Equal circle radii
  ArcToArc,       // Equal arc radii
  CircleToArc     // Equal circle radius to arc radius
};

/**
 * @brief Constrains two entities to have equal measure
 *
 * For lines: equal length
 * For circles/arcs: equal radius
 * Removes 1 DOF.
 */
class EqualConstraint : public Constraint {
public:
  using Ptr = std::shared_ptr<EqualConstraint>;

  EqualConstraint();

  // Line-to-line equal length
  EqualConstraint(SketchLine::Ptr line1, SketchLine::Ptr line2);

  // Circle-to-circle equal radius
  EqualConstraint(SketchCircle::Ptr circle1, SketchCircle::Ptr circle2);

  // Arc-to-arc equal radius
  EqualConstraint(SketchArc::Ptr arc1, SketchArc::Ptr arc2);

  // Circle-to-arc equal radius
  EqualConstraint(SketchCircle::Ptr circle, SketchArc::Ptr arc);

  ConstraintType type() const override { return ConstraintType::Equal; }
  std::string typeName() const override { return "Equal"; }

  std::vector<SketchEntity::Ptr> entities() const override;
  int entityCount() const override { return 2; }
  int dofRemoved() const override { return 1; }

  /**
   * @brief Compute constraint error
   * @return Difference in measure (length or radius)
   */
  double error() const override;

  /**
   * @brief Compute Jacobian for Newton-Raphson solver
   */
  std::vector<double> jacobian() const override;

  Constraint::Ptr clone() const override;

  EqualType equalType() const { return m_equalType; }

private:
  EqualType m_equalType;
  SketchLine::Ptr m_line1;
  SketchLine::Ptr m_line2;
  SketchCircle::Ptr m_circle1;
  SketchCircle::Ptr m_circle2;
  SketchArc::Ptr m_arc1;
  SketchArc::Ptr m_arc2;
};

} // namespace sketch
} // namespace opencad
