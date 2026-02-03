#pragma once

#include "Assembly.h"
#include "AssemblyConstraint.h"
#include "Component.h"
#include <BRepExtrema_DistShapeShape.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <memory>
#include <vector>

namespace opencad {
namespace assembly {

/**
 * @class ConstraintSolver
 * @brief Iterative solver for assembly constraints
 *
 * Uses gradient descent approach to minimize constraint residuals.
 * Each iteration moves unfixed components to better satisfy constraints.
 */
class ConstraintSolver {
public:
  ConstraintSolver() = default;
  ~ConstraintSolver() = default;

  /**
   * @brief Solve all constraints in the assembly
   * @param assembly The assembly to solve
   * @param maxIterations Maximum number of solver iterations
   * @param tolerance Convergence tolerance (in mm)
   * @return true if converged within tolerance
   */
  bool solve(Assembly &assembly, int maxIterations = 100,
             double tolerance = 0.01);

  /**
   * @brief Get the residual (total constraint error) after last solve
   */
  double getResidual() const { return m_residual; }

  /**
   * @brief Get number of iterations used in last solve
   */
  int getIterationCount() const { return m_iterations; }

  /**
   * @brief Get error message if solve failed
   */
  std::string getErrorMessage() const { return m_errorMessage; }

private:
  /**
   * @brief Correction vector (Translation + Rotation)
   */
  struct SolverDelta {
    gp_Vec translation = gp_Vec(0, 0, 0);
    gp_Vec rotationAxis = gp_Vec(0, 0, 1);
    double rotationAngle = 0.0; // In radians

    bool hasRotation() const { return std::abs(rotationAngle) > 1e-6; }
  };

  /**
   * @brief Compute the correction (Translation + Rotation) for coincident
   * constraint
   *
   * Calculates translation to make shapes touch and rotation to align faces.
   */
  SolverDelta
  computeCoincidentCorrection(std::shared_ptr<AssemblyConstraint> constraint);

  /**
   * @brief Compute the correction for distance constraint
   *
   * Calculates translation needed to achieve target distance.
   * Rotation might be needed if parallel alignment is implied.
   */
  SolverDelta
  computeDistanceCorrection(std::shared_ptr<AssemblyConstraint> constraint);

  /**
   * @brief Compute closest distance between two components (or sub-shapes)
   */
  double computeDistance(std::shared_ptr<AssemblyConstraint> constraint);

  /**
   * @brief Apply a delta movement to a component
   */
  void applyDelta(std::shared_ptr<Component> component,
                  const SolverDelta &delta);

  /**
   * @brief Compute total residual for all constraints
   */
  double computeTotalResidual(const Assembly &assembly);

  double m_residual = 0.0;
  int m_iterations = 0;
  std::string m_errorMessage;

  // Solver parameters
  double m_dampingFactor = 0.5; // Prevents overshooting
};

} // namespace assembly
} // namespace opencad
