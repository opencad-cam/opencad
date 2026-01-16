/**
 * @file ConstraintSolver.h
 * @brief Newton-Raphson based constraint solver
 */

#pragma once

#include "../constraints/Constraint.h"
#include "../entities/SketchEntity.h"
#include <vector>
#include <memory>
#include <Eigen/Dense>

namespace opencad {
namespace sketch {

/**
 * @brief Solver status
 */
enum class SolverStatus {
    Solved,           // All constraints satisfied
    PartialSolution,  // Some constraints could not be satisfied
    Redundant,        // Redundant constraints detected
    Overconstrained,  // Too many constraints
    Underconstrained, // Not enough constraints
    Failed,           // Solver failed to converge
    NotRun            // Solver has not been run
};

/**
 * @brief Constraint solver using Newton-Raphson method
 */
class ConstraintSolver {
public:
    ConstraintSolver();
    
    // Configuration
    void setMaxIterations(int maxIter) { m_maxIterations = maxIter; }
    void setTolerance(double tol) { m_tolerance = tol; }
    void setDamping(double damping) { m_damping = damping; }
    
    // Solve
    SolverStatus solve(
        std::vector<SketchEntity::Ptr>& entities,
        const std::vector<Constraint::Ptr>& constraints);
    
    // Results
    SolverStatus status() const { return m_status; }
    int iterationsUsed() const { return m_iterationsUsed; }
    double finalError() const { return m_finalError; }
    
    // DOF analysis
    int totalDOF(const std::vector<SketchEntity::Ptr>& entities) const;
    int constrainedDOF(const std::vector<Constraint::Ptr>& constraints) const;
    int remainingDOF(const std::vector<SketchEntity::Ptr>& entities,
                     const std::vector<Constraint::Ptr>& constraints) const;
    
    // Check if fully constrained
    bool isFullyConstrained(const std::vector<SketchEntity::Ptr>& entities,
                            const std::vector<Constraint::Ptr>& constraints) const;
    
private:
    int m_maxIterations;
    double m_tolerance;
    double m_damping;
    
    SolverStatus m_status;
    int m_iterationsUsed;
    double m_finalError;
    
    // Build parameter vector from entities
    Eigen::VectorXd buildParameterVector(const std::vector<SketchEntity::Ptr>& entities) const;
    
    // Apply parameter vector back to entities
    void applyParameterVector(std::vector<SketchEntity::Ptr>& entities, 
                               const Eigen::VectorXd& params) const;
    
    // Build error vector from constraints
    Eigen::VectorXd buildErrorVector(const std::vector<Constraint::Ptr>& constraints) const;
    
    // Build Jacobian matrix
    Eigen::MatrixXd buildJacobian(const std::vector<SketchEntity::Ptr>& entities,
                                   const std::vector<Constraint::Ptr>& constraints) const;
    
    // Single Newton-Raphson step
    bool newtonStep(std::vector<SketchEntity::Ptr>& entities,
                    const std::vector<Constraint::Ptr>& constraints);
};

} // namespace sketch
} // namespace opencad
