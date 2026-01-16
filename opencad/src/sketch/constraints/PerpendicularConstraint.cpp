/**
 * @file PerpendicularConstraint.cpp
 * @brief Implementation of perpendicular constraint
 */

#include "PerpendicularConstraint.h"
#include <cmath>

namespace opencad {
namespace sketch {

PerpendicularConstraint::PerpendicularConstraint()
    : Constraint(), m_line1(nullptr), m_line2(nullptr) {}

PerpendicularConstraint::PerpendicularConstraint(SketchLine::Ptr line1,
                                                 SketchLine::Ptr line2)
    : Constraint(), m_line1(line1), m_line2(line2) {}

std::vector<SketchEntity::Ptr> PerpendicularConstraint::entities() const {
  std::vector<SketchEntity::Ptr> result;
  if (m_line1)
    result.push_back(m_line1);
  if (m_line2)
    result.push_back(m_line2);
  return result;
}

double PerpendicularConstraint::error() const {
  if (!m_line1 || !m_line2)
    return 0.0;

  // Get direction vectors of both lines
  gp_Pnt2d s1 = m_line1->startPoint();
  gp_Pnt2d e1 = m_line1->endPoint();
  gp_Pnt2d s2 = m_line2->startPoint();
  gp_Pnt2d e2 = m_line2->endPoint();

  // Direction vectors
  double dx1 = e1.X() - s1.X();
  double dy1 = e1.Y() - s1.Y();
  double dx2 = e2.X() - s2.X();
  double dy2 = e2.Y() - s2.Y();

  // Lengths for normalization
  double len1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
  double len2 = std::sqrt(dx2 * dx2 + dy2 * dy2);

  if (len1 < 1e-10 || len2 < 1e-10)
    return 0.0;

  // Normalize
  dx1 /= len1;
  dy1 /= len1;
  dx2 /= len2;
  dy2 /= len2;

  // Dot product of normalized vectors: should be 0 for perpendicular lines
  // dot = dx1 * dx2 + dy1 * dy2
  return dx1 * dx2 + dy1 * dy2;
}

std::vector<double> PerpendicularConstraint::jacobian() const {
  std::vector<double> jac;
  if (!m_line1 || !m_line2)
    return jac;

  // Get direction vectors of both lines
  gp_Pnt2d s1 = m_line1->startPoint();
  gp_Pnt2d e1 = m_line1->endPoint();
  gp_Pnt2d s2 = m_line2->startPoint();
  gp_Pnt2d e2 = m_line2->endPoint();

  // Direction vectors
  double dx1 = e1.X() - s1.X();
  double dy1 = e1.Y() - s1.Y();
  double dx2 = e2.X() - s2.X();
  double dy2 = e2.Y() - s2.Y();

  double len1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
  double len2 = std::sqrt(dx2 * dx2 + dy2 * dy2);

  if (len1 < 1e-10 || len2 < 1e-10)
    return jac;

  // Normalized directions
  double nx1 = dx1 / len1, ny1 = dy1 / len1;
  double nx2 = dx2 / len2, ny2 = dy2 / len2;

  // Error = nx1 * nx2 + ny1 * ny2 (dot product)

  double len1_3 = len1 * len1 * len1;
  double len2_3 = len2 * len2 * len2;

  // Partial derivatives of normalized vector w.r.t dx1, dy1
  double dnx1_dx1 = (dy1 * dy1) / len1_3;
  double dnx1_dy1 = -(dx1 * dy1) / len1_3;
  double dny1_dx1 = -(dx1 * dy1) / len1_3;
  double dny1_dy1 = (dx1 * dx1) / len1_3;

  double dnx2_dx2 = (dy2 * dy2) / len2_3;
  double dnx2_dy2 = -(dx2 * dy2) / len2_3;
  double dny2_dx2 = -(dx2 * dy2) / len2_3;
  double dny2_dy2 = (dx2 * dx2) / len2_3;

  // error = nx1 * nx2 + ny1 * ny2
  // d(error)/d(dx1) = nx2 * dnx1_dx1 + ny2 * dny1_dx1
  // d(error)/d(dy1) = nx2 * dnx1_dy1 + ny2 * dny1_dy1
  // d(error)/d(dx2) = nx1 * dnx2_dx2 + ny1 * dny2_dx2
  // d(error)/d(dy2) = nx1 * dnx2_dy2 + ny1 * dny2_dy2

  double de_dx1 = nx2 * dnx1_dx1 + ny2 * dny1_dx1;
  double de_dy1 = nx2 * dnx1_dy1 + ny2 * dny1_dy1;
  double de_dx2 = nx1 * dnx2_dx2 + ny1 * dny2_dx2;
  double de_dy2 = nx1 * dnx2_dy2 + ny1 * dny2_dy2;

  // Line 1: s1.x, s1.y, e1.x, e1.y
  jac.push_back(-de_dx1); // d/d(s1.x)
  jac.push_back(-de_dy1); // d/d(s1.y)
  jac.push_back(de_dx1);  // d/d(e1.x)
  jac.push_back(de_dy1);  // d/d(e1.y)

  // Line 2: s2.x, s2.y, e2.x, e2.y
  jac.push_back(-de_dx2); // d/d(s2.x)
  jac.push_back(-de_dy2); // d/d(s2.y)
  jac.push_back(de_dx2);  // d/d(e2.x)
  jac.push_back(de_dy2);  // d/d(e2.y)

  return jac;
}

Constraint::Ptr PerpendicularConstraint::clone() const {
  auto cloned = std::make_shared<PerpendicularConstraint>(m_line1, m_line2);
  cloned->setId(id());
  cloned->setDriving(isDriving());
  cloned->setEnabled(isEnabled());
  return cloned;
}

} // namespace sketch
} // namespace opencad
