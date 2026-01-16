/**
 * @file ConcentricConstraint.cpp
 * @brief Implementation of concentric constraint
 */

#include "ConcentricConstraint.h"
#include <cmath>

namespace opencad {
namespace sketch {

ConcentricConstraint::ConcentricConstraint()
    : Constraint(), m_concentricType(ConcentricType::CircleToCircle),
      m_circle1(nullptr), m_circle2(nullptr), m_arc1(nullptr), m_arc2(nullptr) {
}

ConcentricConstraint::ConcentricConstraint(SketchCircle::Ptr circle1,
                                           SketchCircle::Ptr circle2)
    : Constraint(), m_concentricType(ConcentricType::CircleToCircle),
      m_circle1(circle1), m_circle2(circle2), m_arc1(nullptr), m_arc2(nullptr) {
}

ConcentricConstraint::ConcentricConstraint(SketchCircle::Ptr circle,
                                           SketchArc::Ptr arc)
    : Constraint(), m_concentricType(ConcentricType::CircleToArc),
      m_circle1(circle), m_circle2(nullptr), m_arc1(arc), m_arc2(nullptr) {}

ConcentricConstraint::ConcentricConstraint(SketchArc::Ptr arc1,
                                           SketchArc::Ptr arc2)
    : Constraint(), m_concentricType(ConcentricType::ArcToArc),
      m_circle1(nullptr), m_circle2(nullptr), m_arc1(arc1), m_arc2(arc2) {}

std::vector<SketchEntity::Ptr> ConcentricConstraint::entities() const {
  std::vector<SketchEntity::Ptr> result;
  switch (m_concentricType) {
  case ConcentricType::CircleToCircle:
    if (m_circle1)
      result.push_back(m_circle1);
    if (m_circle2)
      result.push_back(m_circle2);
    break;
  case ConcentricType::CircleToArc:
    if (m_circle1)
      result.push_back(m_circle1);
    if (m_arc1)
      result.push_back(m_arc1);
    break;
  case ConcentricType::ArcToArc:
    if (m_arc1)
      result.push_back(m_arc1);
    if (m_arc2)
      result.push_back(m_arc2);
    break;
  }
  return result;
}

gp_Pnt2d ConcentricConstraint::center1() const {
  switch (m_concentricType) {
  case ConcentricType::CircleToCircle:
  case ConcentricType::CircleToArc:
    if (m_circle1)
      return m_circle1->center();
    break;
  case ConcentricType::ArcToArc:
    if (m_arc1)
      return m_arc1->center();
    break;
  }
  return gp_Pnt2d(0, 0);
}

gp_Pnt2d ConcentricConstraint::center2() const {
  switch (m_concentricType) {
  case ConcentricType::CircleToCircle:
    if (m_circle2)
      return m_circle2->center();
    break;
  case ConcentricType::CircleToArc:
    if (m_arc1)
      return m_arc1->center();
    break;
  case ConcentricType::ArcToArc:
    if (m_arc2)
      return m_arc2->center();
    break;
  }
  return gp_Pnt2d(0, 0);
}

double ConcentricConstraint::errorX() const {
  gp_Pnt2d c1 = center1();
  gp_Pnt2d c2 = center2();
  return c1.X() - c2.X();
}

double ConcentricConstraint::errorY() const {
  gp_Pnt2d c1 = center1();
  gp_Pnt2d c2 = center2();
  return c1.Y() - c2.Y();
}

double ConcentricConstraint::error() const {
  double ex = errorX();
  double ey = errorY();
  return std::sqrt(ex * ex + ey * ey);
}

std::vector<double> ConcentricConstraint::jacobian() const {
  std::vector<double> jac;

  // For concentric, we have 2 error equations: ex = c1x - c2x, ey = c1y - c2y
  // Combined error = sqrt(ex^2 + ey^2)
  // d(error)/d(c1x) = ex/error, d(error)/d(c1y) = ey/error
  // d(error)/d(c2x) = -ex/error, d(error)/d(c2y) = -ey/error

  double ex = errorX();
  double ey = errorY();
  double err = std::sqrt(ex * ex + ey * ey);

  if (err < 1e-10) {
    // When centers are coincident, use unit vectors
    err = 1.0;
    ex = 1.0;
    ey = 0.0;
  }

  switch (m_concentricType) {
  case ConcentricType::CircleToCircle:
    // Circle 1: cx, cy, r
    jac.push_back(ex / err); // d/d(c1.x)
    jac.push_back(ey / err); // d/d(c1.y)
    jac.push_back(0.0);      // d/d(r1) - radius doesn't affect concentricity

    // Circle 2: cx, cy, r
    jac.push_back(-ex / err); // d/d(c2.x)
    jac.push_back(-ey / err); // d/d(c2.y)
    jac.push_back(0.0);       // d/d(r2)
    break;

  case ConcentricType::CircleToArc:
    // Circle 1: cx, cy, r
    jac.push_back(ex / err); // d/d(c1.x)
    jac.push_back(ey / err); // d/d(c1.y)
    jac.push_back(0.0);      // d/d(r1)

    // Arc: cx, cy, r, startAngle, endAngle
    jac.push_back(-ex / err); // d/d(arc.cx)
    jac.push_back(-ey / err); // d/d(arc.cy)
    jac.push_back(0.0);       // d/d(r)
    jac.push_back(0.0);       // d/d(startAngle)
    jac.push_back(0.0);       // d/d(endAngle)
    break;

  case ConcentricType::ArcToArc:
    // Arc 1: cx, cy, r, startAngle, endAngle
    jac.push_back(ex / err); // d/d(arc1.cx)
    jac.push_back(ey / err); // d/d(arc1.cy)
    jac.push_back(0.0);      // d/d(r1)
    jac.push_back(0.0);      // d/d(startAngle1)
    jac.push_back(0.0);      // d/d(endAngle1)

    // Arc 2: cx, cy, r, startAngle, endAngle
    jac.push_back(-ex / err); // d/d(arc2.cx)
    jac.push_back(-ey / err); // d/d(arc2.cy)
    jac.push_back(0.0);       // d/d(r2)
    jac.push_back(0.0);       // d/d(startAngle2)
    jac.push_back(0.0);       // d/d(endAngle2)
    break;
  }

  return jac;
}

Constraint::Ptr ConcentricConstraint::clone() const {
  std::shared_ptr<ConcentricConstraint> cloned;

  switch (m_concentricType) {
  case ConcentricType::CircleToCircle:
    cloned = std::make_shared<ConcentricConstraint>(m_circle1, m_circle2);
    break;
  case ConcentricType::CircleToArc:
    cloned = std::make_shared<ConcentricConstraint>(m_circle1, m_arc1);
    break;
  case ConcentricType::ArcToArc:
    cloned = std::make_shared<ConcentricConstraint>(m_arc1, m_arc2);
    break;
  }

  if (cloned) {
    cloned->setId(id());
    cloned->setDriving(isDriving());
    cloned->setEnabled(isEnabled());
  }
  return cloned;
}

} // namespace sketch
} // namespace opencad
