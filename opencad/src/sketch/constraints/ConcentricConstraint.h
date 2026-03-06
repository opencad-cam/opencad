/**
 * @file ConcentricConstraint.h
 * @brief Concentric constraint - two circles/arcs share the same center
 */
#pragma once

#include "../entities/SketchArc.h"
#include "../entities/SketchCircle.h"
#include "Constraint.h"

namespace opencad {
namespace sketch {

/**
 * @brief Concentric type enumeration
 */
enum class ConcentricType { CircleToCircle, CircleToArc, ArcToArc };

/**
 * @brief Constrains two circles/arcs to share the same center
 *
 * Removes 2 DOF (matches both X and Y coordinates of centers)
 */
class ConcentricConstraint : public Constraint {
public:
  using Ptr = std::shared_ptr<ConcentricConstraint>;

  ConcentricConstraint();

  // Circle to circle
  ConcentricConstraint(SketchCircle::Ptr circle1, SketchCircle::Ptr circle2);

  // Circle to arc
  ConcentricConstraint(SketchCircle::Ptr circle, SketchArc::Ptr arc);

  // Arc to arc
  ConcentricConstraint(SketchArc::Ptr arc1, SketchArc::Ptr arc2);

  ConstraintType type() const override { return ConstraintType::Concentric; }
  std::string typeName() const override { return "Concentric"; }

  std::vector<SketchEntity::Ptr> entities() const override;
  int entityCount() const override { return 2; }
  int dofRemoved() const override { return 2; }

  /**
   * @brief Compute constraint error
   * @return Distance between centers (should be 0 for concentric)
   */
  std::vector<double> errorVector() const override;
  double error() const override;

  /**
   * @brief Returns X error (difference in X coordinates)
   */
  double errorX() const;

  /**
   * @brief Returns Y error (difference in Y coordinates)
   */
  double errorY() const;

  /**
   * @brief Compute Jacobian for Newton-Raphson solver
   */
  std::vector<double> jacobian() const override;

  Constraint::Ptr clone() const override;

  ConcentricType concentricType() const { return m_concentricType; }

private:
  ConcentricType m_concentricType;
  SketchCircle::Ptr m_circle1;
  SketchCircle::Ptr m_circle2;
  SketchArc::Ptr m_arc1;
  SketchArc::Ptr m_arc2;

  // Get center points
  gp_Pnt2d center1() const;
  gp_Pnt2d center2() const;
};

} // namespace sketch
} // namespace opencad
