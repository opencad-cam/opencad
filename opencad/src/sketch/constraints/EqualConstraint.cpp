/**
 * @file EqualConstraint.cpp
 * @brief Implementation of equal constraint
 */

#include "EqualConstraint.h"
#include <cmath>

namespace opencad {
namespace sketch {

EqualConstraint::EqualConstraint()
    : Constraint(), m_equalType(EqualType::LineToLine), m_line1(nullptr),
      m_line2(nullptr), m_circle1(nullptr), m_circle2(nullptr), m_arc1(nullptr),
      m_arc2(nullptr) {}

EqualConstraint::EqualConstraint(SketchLine::Ptr line1, SketchLine::Ptr line2)
    : Constraint(), m_equalType(EqualType::LineToLine), m_line1(line1),
      m_line2(line2), m_circle1(nullptr), m_circle2(nullptr), m_arc1(nullptr),
      m_arc2(nullptr) {}

EqualConstraint::EqualConstraint(SketchCircle::Ptr circle1,
                                 SketchCircle::Ptr circle2)
    : Constraint(), m_equalType(EqualType::CircleToCircle), m_line1(nullptr),
      m_line2(nullptr), m_circle1(circle1), m_circle2(circle2), m_arc1(nullptr),
      m_arc2(nullptr) {}

EqualConstraint::EqualConstraint(SketchArc::Ptr arc1, SketchArc::Ptr arc2)
    : Constraint(), m_equalType(EqualType::ArcToArc), m_line1(nullptr),
      m_line2(nullptr), m_circle1(nullptr), m_circle2(nullptr), m_arc1(arc1),
      m_arc2(arc2) {}

EqualConstraint::EqualConstraint(SketchCircle::Ptr circle, SketchArc::Ptr arc)
    : Constraint(), m_equalType(EqualType::CircleToArc), m_line1(nullptr),
      m_line2(nullptr), m_circle1(circle), m_circle2(nullptr), m_arc1(arc),
      m_arc2(nullptr) {}

std::vector<SketchEntity::Ptr> EqualConstraint::entities() const {
  std::vector<SketchEntity::Ptr> result;
  switch (m_equalType) {
  case EqualType::LineToLine:
    if (m_line1)
      result.push_back(m_line1);
    if (m_line2)
      result.push_back(m_line2);
    break;
  case EqualType::CircleToCircle:
    if (m_circle1)
      result.push_back(m_circle1);
    if (m_circle2)
      result.push_back(m_circle2);
    break;
  case EqualType::ArcToArc:
    if (m_arc1)
      result.push_back(m_arc1);
    if (m_arc2)
      result.push_back(m_arc2);
    break;
  case EqualType::CircleToArc:
    if (m_circle1)
      result.push_back(m_circle1);
    if (m_arc1)
      result.push_back(m_arc1);
    break;
  }
  return result;
}

double EqualConstraint::error() const {
  switch (m_equalType) {
  case EqualType::LineToLine: {
    if (!m_line1 || !m_line2)
      return 0.0;
    double len1 = m_line1->length();
    double len2 = m_line2->length();
    return len1 - len2;
  }
  case EqualType::CircleToCircle: {
    if (!m_circle1 || !m_circle2)
      return 0.0;
    return m_circle1->radius() - m_circle2->radius();
  }
  case EqualType::ArcToArc: {
    if (!m_arc1 || !m_arc2)
      return 0.0;
    return m_arc1->radius() - m_arc2->radius();
  }
  case EqualType::CircleToArc: {
    if (!m_circle1 || !m_arc1)
      return 0.0;
    return m_circle1->radius() - m_arc1->radius();
  }
  }
  return 0.0;
}

std::vector<double> EqualConstraint::jacobian() const {
  std::vector<double> jac;

  switch (m_equalType) {
  case EqualType::LineToLine: {
    if (!m_line1 || !m_line2)
      return jac;

    // Line 1 length = sqrt((e1x-s1x)^2 + (e1y-s1y)^2)
    gp_Pnt2d s1 = m_line1->startPoint();
    gp_Pnt2d e1 = m_line1->endPoint();
    gp_Pnt2d s2 = m_line2->startPoint();
    gp_Pnt2d e2 = m_line2->endPoint();

    double dx1 = e1.X() - s1.X();
    double dy1 = e1.Y() - s1.Y();
    double dx2 = e2.X() - s2.X();
    double dy2 = e2.Y() - s2.Y();

    double len1 = std::sqrt(dx1 * dx1 + dy1 * dy1);
    double len2 = std::sqrt(dx2 * dx2 + dy2 * dy2);

    if (len1 < 1e-10 || len2 < 1e-10)
      return jac;

    // d(len1)/d(s1x) = -dx1/len1, d(len1)/d(e1x) = dx1/len1, etc.
    // error = len1 - len2

    // Line 1: s1.x, s1.y, e1.x, e1.y
    jac.push_back(-dx1 / len1); // d/d(s1.x)
    jac.push_back(-dy1 / len1); // d/d(s1.y)
    jac.push_back(dx1 / len1);  // d/d(e1.x)
    jac.push_back(dy1 / len1);  // d/d(e1.y)

    // Line 2: s2.x, s2.y, e2.x, e2.y (negative because error = len1 - len2)
    jac.push_back(dx2 / len2);  // d/d(s2.x) = -(-dx2/len2)
    jac.push_back(dy2 / len2);  // d/d(s2.y)
    jac.push_back(-dx2 / len2); // d/d(e2.x)
    jac.push_back(-dy2 / len2); // d/d(e2.y)
    break;
  }
  case EqualType::CircleToCircle: {
    if (!m_circle1 || !m_circle2)
      return jac;
    // Circle parameters: center.x, center.y, radius
    // error = r1 - r2
    // d/d(r1) = 1, d/d(r2) = -1

    // Circle 1: cx, cy, r
    jac.push_back(0.0); // d/d(cx)
    jac.push_back(0.0); // d/d(cy)
    jac.push_back(1.0); // d/d(r1)

    // Circle 2: cx, cy, r
    jac.push_back(0.0);  // d/d(cx)
    jac.push_back(0.0);  // d/d(cy)
    jac.push_back(-1.0); // d/d(r2)
    break;
  }
  case EqualType::ArcToArc: {
    if (!m_arc1 || !m_arc2)
      return jac;
    // Arc parameters: center.x, center.y, radius, startAngle, endAngle
    // error = r1 - r2

    // Arc 1: cx, cy, r, startAngle, endAngle
    jac.push_back(0.0); // d/d(cx)
    jac.push_back(0.0); // d/d(cy)
    jac.push_back(1.0); // d/d(r1)
    jac.push_back(0.0); // d/d(startAngle)
    jac.push_back(0.0); // d/d(endAngle)

    // Arc 2: cx, cy, r, startAngle, endAngle
    jac.push_back(0.0);  // d/d(cx)
    jac.push_back(0.0);  // d/d(cy)
    jac.push_back(-1.0); // d/d(r2)
    jac.push_back(0.0);  // d/d(startAngle)
    jac.push_back(0.0);  // d/d(endAngle)
    break;
  }
  case EqualType::CircleToArc: {
    if (!m_circle1 || !m_arc1)
      return jac;

    // Circle 1: cx, cy, r
    jac.push_back(0.0); // d/d(cx)
    jac.push_back(0.0); // d/d(cy)
    jac.push_back(1.0); // d/d(r1)

    // Arc 1: cx, cy, r, startAngle, endAngle
    jac.push_back(0.0);  // d/d(cx)
    jac.push_back(0.0);  // d/d(cy)
    jac.push_back(-1.0); // d/d(r)
    jac.push_back(0.0);  // d/d(startAngle)
    jac.push_back(0.0);  // d/d(endAngle)
    break;
  }
  }

  return jac;
}

Constraint::Ptr EqualConstraint::clone() const {
  std::shared_ptr<EqualConstraint> cloned;

  switch (m_equalType) {
  case EqualType::LineToLine:
    cloned = std::make_shared<EqualConstraint>(m_line1, m_line2);
    break;
  case EqualType::CircleToCircle:
    cloned = std::make_shared<EqualConstraint>(m_circle1, m_circle2);
    break;
  case EqualType::ArcToArc:
    cloned = std::make_shared<EqualConstraint>(m_arc1, m_arc2);
    break;
  case EqualType::CircleToArc:
    cloned = std::make_shared<EqualConstraint>(m_circle1, m_arc1);
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
