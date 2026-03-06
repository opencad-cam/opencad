/**
 * @file CoincidentConstraint.cpp
 * @brief Implementation of CoincidentConstraint
 */

#include "CoincidentConstraint.h"
#include <cmath>

namespace opencad {
namespace sketch {

CoincidentConstraint::CoincidentConstraint()
    : Constraint(), m_point1(nullptr), m_point2(nullptr), m_line1(nullptr),
      m_line2(nullptr), m_endpoint1(-1), m_endpoint2(-1) {}

CoincidentConstraint::CoincidentConstraint(SketchPoint::Ptr point1,
                                           SketchPoint::Ptr point2)
    : Constraint(), m_point1(point1), m_point2(point2), m_line1(nullptr),
      m_line2(nullptr), m_endpoint1(-1), m_endpoint2(-1) {}

CoincidentConstraint::CoincidentConstraint(SketchPoint::Ptr point,
                                           SketchLine::Ptr line, int endpoint)
    : Constraint(), m_point1(point), m_point2(nullptr), m_line1(line),
      m_line2(nullptr), m_endpoint1(endpoint), m_endpoint2(-1) {}

CoincidentConstraint::CoincidentConstraint(SketchLine::Ptr line1, int endpoint1,
                                           SketchLine::Ptr line2, int endpoint2)
    : Constraint(), m_point1(nullptr), m_point2(nullptr), m_line1(line1),
      m_line2(line2), m_endpoint1(endpoint1), m_endpoint2(endpoint2) {}

std::vector<SketchEntity::Ptr> CoincidentConstraint::entities() const {
  std::vector<SketchEntity::Ptr> result;
  if (m_point1)
    result.push_back(m_point1);
  if (m_point2)
    result.push_back(m_point2);
  if (m_line1)
    result.push_back(m_line1);
  if (m_line2)
    result.push_back(m_line2);
  return result;
}

gp_Pnt2d CoincidentConstraint::point1() const {
  if (m_point1) {
    return m_point1->position();
  }
  if (m_line1) {
    return m_endpoint1 == 0 ? m_line1->startPoint() : m_line1->endPoint();
  }
  return gp_Pnt2d(0, 0);
}

gp_Pnt2d CoincidentConstraint::point2() const {
  if (m_point2) {
    return m_point2->position();
  }
  if (m_point1 && m_line1) {
    return m_endpoint1 == 0 ? m_line1->startPoint() : m_line1->endPoint();
  }
  if (m_line2) {
    return m_endpoint2 == 0 ? m_line2->startPoint() : m_line2->endPoint();
  }
  return gp_Pnt2d(0, 0);
}

double CoincidentConstraint::errorX() const {
  gp_Pnt2d p1 = point1();
  gp_Pnt2d p2 = point2();
  return p2.X() - p1.X();
}

double CoincidentConstraint::errorY() const {
  gp_Pnt2d p1 = point1();
  gp_Pnt2d p2 = point2();
  return p2.Y() - p1.Y();
}

std::vector<double> CoincidentConstraint::errorVector() const {
  return {errorX(), errorY()};
}

double CoincidentConstraint::error() const {
  // Combined error magnitude
  double dx = errorX();
  double dy = errorY();
  return std::sqrt(dx * dx + dy * dy);
}

std::vector<double> CoincidentConstraint::jacobian() const {
  std::vector<double> result;
  auto ents = entities();
  int totalParams = 0;
  for (auto e : ents)
    totalParams += e->parameterCount();

  result.assign(2 * totalParams, 0.0);
  int paramOffset = 0;

  if (!ents.empty()) {
    auto e1 = ents[0];
    if (e1->type() == EntityType::Point) {
      result[0 * totalParams + paramOffset + 0] = -1.0;
      result[1 * totalParams + paramOffset + 1] = -1.0;
    } else if (e1->type() == EntityType::Line) {
      int offset = (m_endpoint1 == 0) ? 0 : 2;
      result[0 * totalParams + paramOffset + offset] = -1.0;
      result[1 * totalParams + paramOffset + offset + 1] = -1.0;
    }
    paramOffset += e1->parameterCount();
  }

  if (ents.size() > 1) {
    auto e2 = ents[1];
    if (e2->type() == EntityType::Point) {
      result[0 * totalParams + paramOffset + 0] = 1.0;
      result[1 * totalParams + paramOffset + 1] = 1.0;
    } else if (e2->type() == EntityType::Line) {
      int offset = (m_endpoint2 == 0) ? 0 : 2;
      result[0 * totalParams + paramOffset + offset] = 1.0;
      result[1 * totalParams + paramOffset + offset + 1] = 1.0;
    }
  }

  return result;
}

Constraint::Ptr CoincidentConstraint::clone() const {
  auto cloned = std::make_shared<CoincidentConstraint>();
  cloned->m_point1 = m_point1;
  cloned->m_point2 = m_point2;
  cloned->m_line1 = m_line1;
  cloned->m_line2 = m_line2;
  cloned->m_endpoint1 = m_endpoint1;
  cloned->m_endpoint2 = m_endpoint2;
  cloned->setDriving(isDriving());
  cloned->setEnabled(isEnabled());
  return cloned;
}

} // namespace sketch
} // namespace opencad
