/**
 * @file ConstraintSolver.cpp
 * @brief Implementation of Newton-Raphson constraint solver
 */

#include "ConstraintSolver.h"
#include <algorithm>
#include <cmath>


namespace opencad {
namespace sketch {

ConstraintSolver::ConstraintSolver()
    : m_maxIterations(50), m_tolerance(1e-9), m_damping(1.0),
      m_status(SolverStatus::NotRun), m_iterationsUsed(0), m_finalError(0.0) {}

int ConstraintSolver::totalDOF(
    const std::vector<SketchEntity::Ptr> &entities) const {
  int dof = 0;
  for (const auto &entity : entities) {
    dof += entity->baseDOF();
  }
  return dof;
}

int ConstraintSolver::constrainedDOF(
    const std::vector<Constraint::Ptr> &constraints) const {
  int dof = 0;
  for (const auto &constraint : constraints) {
    if (constraint->isEnabled() && constraint->isDriving()) {
      dof += constraint->dofRemoved();
    }
  }
  return dof;
}

int ConstraintSolver::remainingDOF(
    const std::vector<SketchEntity::Ptr> &entities,
    const std::vector<Constraint::Ptr> &constraints) const {
  return totalDOF(entities) - constrainedDOF(constraints);
}

bool ConstraintSolver::isFullyConstrained(
    const std::vector<SketchEntity::Ptr> &entities,
    const std::vector<Constraint::Ptr> &constraints) const {
  return remainingDOF(entities, constraints) == 0;
}

Eigen::VectorXd ConstraintSolver::buildParameterVector(
    const std::vector<SketchEntity::Ptr> &entities) const {

  int totalParams = 0;
  for (const auto &entity : entities) {
    totalParams += entity->parameterCount();
  }

  Eigen::VectorXd params(totalParams);
  int idx = 0;
  for (const auto &entity : entities) {
    for (int i = 0; i < entity->parameterCount(); ++i) {
      params(idx++) = entity->getParameter(i);
    }
  }
  return params;
}

void ConstraintSolver::applyParameterVector(
    std::vector<SketchEntity::Ptr> &entities,
    const Eigen::VectorXd &params) const {

  int idx = 0;
  for (auto &entity : entities) {
    for (int i = 0; i < entity->parameterCount(); ++i) {
      entity->setParameter(i, params(idx++));
    }
  }
}

Eigen::VectorXd ConstraintSolver::buildErrorVector(
    const std::vector<Constraint::Ptr> &constraints) const {

  int numEquations = 0;
  for (const auto &c : constraints) {
    if (c->isEnabled() && c->isDriving()) {
      numEquations += c->dofRemoved();
    }
  }

  Eigen::VectorXd errors(numEquations);
  int idx = 0;
  for (const auto &c : constraints) {
    if (c->isEnabled() && c->isDriving()) {
      std::vector<double> errs = c->errorVector();
      for (double e : errs) {
        if (idx < numEquations)
          errors(idx++) = e;
      }
    }
  }
  return errors;
}

Eigen::MatrixXd ConstraintSolver::buildJacobian(
    const std::vector<SketchEntity::Ptr> &entities,
    const std::vector<Constraint::Ptr> &constraints) const {

  // Count parameters and constraints
  int numParams = 0;
  for (const auto &e : entities) {
    numParams += e->parameterCount();
  }

  int numEquations = 0;
  for (const auto &c : constraints) {
    if (c->isEnabled() && c->isDriving()) {
      numEquations += c->dofRemoved();
    }
  }

  // Build Jacobian matrix
  Eigen::MatrixXd J = Eigen::MatrixXd::Zero(numEquations, numParams);

  int rowIdx = 0;
  for (const auto &constraint : constraints) {
    if (!constraint->isEnabled() || !constraint->isDriving())
      continue;

    int dof = constraint->dofRemoved();
    auto jac = constraint->jacobian();
    auto constraintEntities = constraint->entities();

    int entityParamsTotal = 0;
    for (const auto &entity : constraintEntities) {
      entityParamsTotal += entity->parameterCount();
    }

    // Map constraint jacobian to global parameter indices
    for (int r = 0; r < dof; ++r) {
      int paramCol = 0;
      for (const auto &entity : constraintEntities) {
        // Find entity's parameter offset
        int paramOffset = 0;
        for (const auto &e : entities) {
          if (e == entity)
            break;
          paramOffset += e->parameterCount();
        }

        // Copy jacobian values
        for (int i = 0; i < entity->parameterCount(); ++i) {
          if (r * entityParamsTotal + paramCol < static_cast<int>(jac.size())) {
            J(rowIdx + r, paramOffset + i) =
                jac[r * entityParamsTotal + paramCol];
          }
          paramCol++;
        }
      }
    }

    rowIdx += dof;
  }

  return J;
}

bool ConstraintSolver::newtonStep(
    std::vector<SketchEntity::Ptr> &entities,
    const std::vector<Constraint::Ptr> &constraints) {

  // Build error vector
  Eigen::VectorXd errors = buildErrorVector(constraints);

  // Check if already converged
  m_finalError = errors.norm();
  if (m_finalError < m_tolerance) {
    return true;
  }

  // Build Jacobian
  Eigen::MatrixXd J = buildJacobian(entities, constraints);

  // Solve J * delta = -errors using pseudo-inverse (for over/under-constrained
  // systems)
  Eigen::VectorXd delta;

  if (J.rows() == J.cols()) {
    // Square system - use LU decomposition
    delta = J.fullPivLu().solve(-errors);
  } else {
    // Non-square - use SVD pseudo-inverse
    delta = J.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(-errors);
  }

  // Apply damped update
  Eigen::VectorXd params = buildParameterVector(entities);
  params += m_damping * delta;
  applyParameterVector(entities, params);

  return false;
}

SolverStatus
ConstraintSolver::solve(std::vector<SketchEntity::Ptr> &entities,
                        const std::vector<Constraint::Ptr> &constraints) {

  m_status = SolverStatus::NotRun;
  m_iterationsUsed = 0;
  m_finalError = 0.0;

  if (entities.empty() || constraints.empty()) {
    m_status = SolverStatus::Underconstrained;
    return m_status;
  }

  // Check DOF
  int remaining = remainingDOF(entities, constraints);
  if (remaining < 0) {
    m_status = SolverStatus::Overconstrained;
    // Continue anyway - solver may find a solution
  }

  // Newton-Raphson iterations
  for (int iter = 0; iter < m_maxIterations; ++iter) {
    m_iterationsUsed = iter + 1;

    if (newtonStep(entities, constraints)) {
      m_status = SolverStatus::Solved;
      return m_status;
    }

    // Check for convergence
    if (m_finalError < m_tolerance) {
      m_status = SolverStatus::Solved;
      return m_status;
    }
  }

  // Check final error
  if (m_finalError < m_tolerance * 10) {
    m_status = SolverStatus::PartialSolution;
  } else {
    m_status = SolverStatus::Failed;
  }

  return m_status;
}

} // namespace sketch
} // namespace opencad
