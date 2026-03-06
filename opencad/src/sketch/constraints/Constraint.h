/**
 * @file Constraint.h
 * @brief Base class for all sketch constraints
 */

#pragma once

#include "../entities/SketchEntity.h"
#include <memory>
#include <string>
#include <vector>

namespace opencad {
namespace sketch {

/**
 * @brief Constraint type enumeration
 */
enum class ConstraintType {
  Horizontal,    // Line is horizontal
  Vertical,      // Line is vertical
  Coincident,    // Two points are at same location
  Concentric,    // Two circles/arcs share center
  Tangent,       // Two curves touch tangentially
  Perpendicular, // Two lines at 90 degrees
  Parallel,      // Two lines are parallel
  Equal,         // Two entities have equal measure
  Fix,           // Entity is fixed in place
  Distance,      // Distance between points/lines
  Angle,         // Angle between lines
  Radius,        // Circle/arc radius
  Diameter,      // Circle/arc diameter
  Midpoint,      // Point is at midpoint of line
  Symmetric,     // Symmetric about line/point
  Collinear,     // Points on same line
  Coradial       // Same radius
};

/**
 * @brief Constraint status
 */
enum class ConstraintStatus {
  Satisfied,    // Constraint is satisfied
  NotSatisfied, // Constraint is violated
  Redundant,    // Constraint is redundant
  Conflicting,  // Constraint conflicts with others
  Invalid       // Invalid constraint setup
};

/**
 * @brief Base class for sketch constraints
 */
class Constraint {
public:
  using Ptr = std::shared_ptr<Constraint>;

  Constraint();
  virtual ~Constraint() = default;

  // Identification
  uint64_t id() const { return m_id; }
  void setId(uint64_t id) { m_id = id; }

  // Type
  virtual ConstraintType type() const = 0;
  virtual std::string typeName() const = 0;

  // Entities involved
  virtual std::vector<SketchEntity::Ptr> entities() const = 0;
  virtual int entityCount() const = 0;

  // Constraint value (for dimensional constraints)
  virtual bool hasDimension() const { return false; }
  virtual double dimension() const { return 0.0; }
  virtual void setDimension(double value) { (void)value; }

  // ========== ASSOCIATIVE PARAMETER BINDING ==========
  // Bind constraint dimension to a named parameter
  bool isBoundToParameter() const { return !m_boundParameterName.empty(); }
  std::string boundParameterName() const { return m_boundParameterName; }
  void bindToParameter(const std::string &paramName) {
    m_boundParameterName = paramName;
  }
  void unbindParameter() { m_boundParameterName.clear(); }

  // Expression support (e.g., "width * 2" or "radius + 5")
  bool hasExpression() const { return !m_expression.empty(); }
  std::string expression() const { return m_expression; }
  void setExpression(const std::string &expr) { m_expression = expr; }
  void clearExpression() { m_expression.clear(); }

  // Driving vs Driven
  bool isDriving() const { return m_isDriving; }
  void setDriving(bool driving) { m_isDriving = driving; }

  // Degrees of freedom removed by this constraint
  virtual int dofRemoved() const = 0;

  // Evaluation
  virtual std::vector<double> errorVector() const { return {error()}; }
  virtual double error() const = 0; // How far from satisfied
  virtual bool isSatisfied(double tolerance = 1e-6) const;
  virtual ConstraintStatus status() const;

  // Jacobian for solver (derivatives of error w.r.t. parameters)
  virtual std::vector<double> jacobian() const = 0;

  // Clone
  virtual Ptr clone() const = 0;

  // Enabled state
  bool isEnabled() const { return m_isEnabled; }
  void setEnabled(bool enabled) { m_isEnabled = enabled; }

protected:
  uint64_t m_id;
  bool m_isDriving; // True = constraint drives geometry, False = reference only
  bool m_isEnabled;

  // Associative bindings
  std::string m_boundParameterName; // e.g., "width", "height", "myRadius"
  std::string m_expression;         // e.g., "width * 2", "radius + 10"

  static uint64_t s_nextId;
};

} // namespace sketch
} // namespace opencad
