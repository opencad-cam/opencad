/**
 * @file TangentConstraint.h
 * @brief Tangent constraint - two curves touch tangentially
 */
#pragma once

#include "../entities/SketchArc.h"
#include "../entities/SketchCircle.h"
#include "../entities/SketchLine.h"
#include "Constraint.h"


namespace opencad {
namespace sketch {

/**
 * @brief Tangent type enumeration
 */
enum class TangentType {
  LineToCircle,   // Line tangent to circle
  LineToArc,      // Line tangent to arc
  CircleToCircle, // Two circles tangent (internally or externally)
  ArcToArc,       // Two arcs tangent
  CircleToArc     // Circle tangent to arc
};

/**
 * @brief Constrains two curves to touch tangentially
 *
 * For line-circle: distance from line to center = radius
 * For circle-circle: distance between centers = r1 + r2 (external) or |r1 - r2|
 * (internal) Removes 1 DOF.
 */
class TangentConstraint : public Constraint {
public:
  using Ptr = std::shared_ptr<TangentConstraint>;

  TangentConstraint();

  // Line tangent to circle
  TangentConstraint(SketchLine::Ptr line, SketchCircle::Ptr circle);

  // Line tangent to arc
  TangentConstraint(SketchLine::Ptr line, SketchArc::Ptr arc);

  // Circle to circle (external tangency by default)
  TangentConstraint(SketchCircle::Ptr circle1, SketchCircle::Ptr circle2,
                    bool internal = false);

  // Arc to arc (external tangency by default)
  TangentConstraint(SketchArc::Ptr arc1, SketchArc::Ptr arc2,
                    bool internal = false);

  // Circle to arc
  TangentConstraint(SketchCircle::Ptr circle, SketchArc::Ptr arc,
                    bool internal = false);

  ConstraintType type() const override { return ConstraintType::Tangent; }
  std::string typeName() const override { return "Tangent"; }

  std::vector<SketchEntity::Ptr> entities() const override;
  int entityCount() const override { return 2; }
  int dofRemoved() const override { return 1; }

  /**
   * @brief Compute constraint error
   * @return Distance error for tangency condition
   */
  double error() const override;

  /**
   * @brief Compute Jacobian for Newton-Raphson solver
   */
  std::vector<double> jacobian() const override;

  Constraint::Ptr clone() const override;

  TangentType tangentType() const { return m_tangentType; }
  bool isInternal() const { return m_internal; }

private:
  TangentType m_tangentType;
  bool m_internal; // For circle-circle tangency

  SketchLine::Ptr m_line;
  SketchCircle::Ptr m_circle1;
  SketchCircle::Ptr m_circle2;
  SketchArc::Ptr m_arc1;
  SketchArc::Ptr m_arc2;

  // Helper: distance from point to line
  double pointToLineDistance(const gp_Pnt2d &point, const gp_Pnt2d &lineStart,
                             const gp_Pnt2d &lineEnd) const;
};

} // namespace sketch
} // namespace opencad
