/**
 * @file FixConstraint.cpp
 * @brief Implementation of fix constraint
 */

#include "FixConstraint.h"
#include <cmath>

namespace opencad {
namespace sketch {

FixConstraint::FixConstraint() : Constraint(), m_entity(nullptr) {}

FixConstraint::FixConstraint(SketchPoint::Ptr point)
    : Constraint(), m_entity(point) {
  capturePosition();
}

FixConstraint::FixConstraint(SketchLine::Ptr line)
    : Constraint(), m_entity(line) {
  capturePosition();
}

FixConstraint::FixConstraint(SketchCircle::Ptr circle)
    : Constraint(), m_entity(circle) {
  capturePosition();
}

FixConstraint::FixConstraint(SketchArc::Ptr arc) : Constraint(), m_entity(arc) {
  capturePosition();
}

void FixConstraint::capturePosition() {
  if (!m_entity)
    return;

  switch (m_entity->type()) {
  case EntityType::Point: {
    auto point = std::dynamic_pointer_cast<SketchPoint>(m_entity);
    if (point) {
      m_fixedX = point->x();
      m_fixedY = point->y();
    }
    break;
  }
  case EntityType::Line: {
    auto line = std::dynamic_pointer_cast<SketchLine>(m_entity);
    if (line) {
      m_fixedX = line->startPoint().X();
      m_fixedY = line->startPoint().Y();
      m_fixedX2 = line->endPoint().X();
      m_fixedY2 = line->endPoint().Y();
    }
    break;
  }
  case EntityType::Circle: {
    auto circle = std::dynamic_pointer_cast<SketchCircle>(m_entity);
    if (circle) {
      m_fixedX = circle->center().X();
      m_fixedY = circle->center().Y();
      m_fixedRadius = circle->radius();
    }
    break;
  }
  case EntityType::Arc: {
    auto arc = std::dynamic_pointer_cast<SketchArc>(m_entity);
    if (arc) {
      m_fixedX = arc->center().X();
      m_fixedY = arc->center().Y();
      m_fixedRadius = arc->radius();
    }
    break;
  }
  default:
    break;
  }
}

std::vector<SketchEntity::Ptr> FixConstraint::entities() const {
  if (m_entity) {
    return {m_entity};
  }
  return {};
}

int FixConstraint::dofRemoved() const {
  if (!m_entity)
    return 0;

  switch (m_entity->type()) {
  case EntityType::Point:
    return 2; // x, y
  case EntityType::Line:
    return 4; // x1, y1, x2, y2
  case EntityType::Circle:
    return 3; // cx, cy, r
  case EntityType::Arc:
    return 5; // cx, cy, r, startAngle, endAngle
  default:
    return m_entity->baseDOF();
  }
}

double FixConstraint::error() const {
  if (!m_entity)
    return 0.0;

  double totalError = 0.0;

  switch (m_entity->type()) {
  case EntityType::Point: {
    auto point = std::dynamic_pointer_cast<SketchPoint>(m_entity);
    if (point) {
      double dx = point->x() - m_fixedX;
      double dy = point->y() - m_fixedY;
      totalError = std::sqrt(dx * dx + dy * dy);
    }
    break;
  }
  case EntityType::Line: {
    auto line = std::dynamic_pointer_cast<SketchLine>(m_entity);
    if (line) {
      double dx1 = line->startPoint().X() - m_fixedX;
      double dy1 = line->startPoint().Y() - m_fixedY;
      double dx2 = line->endPoint().X() - m_fixedX2;
      double dy2 = line->endPoint().Y() - m_fixedY2;
      totalError = std::sqrt(dx1 * dx1 + dy1 * dy1 + dx2 * dx2 + dy2 * dy2);
    }
    break;
  }
  case EntityType::Circle: {
    auto circle = std::dynamic_pointer_cast<SketchCircle>(m_entity);
    if (circle) {
      double dx = circle->center().X() - m_fixedX;
      double dy = circle->center().Y() - m_fixedY;
      double dr = circle->radius() - m_fixedRadius;
      totalError = std::sqrt(dx * dx + dy * dy + dr * dr);
    }
    break;
  }
  case EntityType::Arc: {
    auto arc = std::dynamic_pointer_cast<SketchArc>(m_entity);
    if (arc) {
      double dx = arc->center().X() - m_fixedX;
      double dy = arc->center().Y() - m_fixedY;
      double dr = arc->radius() - m_fixedRadius;
      totalError = std::sqrt(dx * dx + dy * dy + dr * dr);
    }
    break;
  }
  default:
    break;
  }

  return totalError;
}

std::vector<double> FixConstraint::jacobian() const {
  std::vector<double> jac;
  if (!m_entity)
    return jac;

  switch (m_entity->type()) {
  case EntityType::Point: {
    auto point = std::dynamic_pointer_cast<SketchPoint>(m_entity);
    if (point) {
      double dx = point->x() - m_fixedX;
      double dy = point->y() - m_fixedY;
      double err = std::sqrt(dx * dx + dy * dy);
      if (err > 1e-10) {
        jac.push_back(dx / err); // d/d(x)
        jac.push_back(dy / err); // d/d(y)
      } else {
        jac.push_back(1.0);
        jac.push_back(0.0);
      }
    }
    break;
  }
  case EntityType::Line: {
    // For lines, we return identity-like Jacobian
    jac = {1.0, 0.0, 0.0, 0.0,  // d/d(s.x)
           0.0, 1.0, 0.0, 0.0,  // d/d(s.y)
           0.0, 0.0, 1.0, 0.0,  // d/d(e.x)
           0.0, 0.0, 0.0, 1.0}; // d/d(e.y)
    break;
  }
  case EntityType::Circle: {
    jac = {1.0, 0.0, 0.0,  // d/d(cx)
           0.0, 1.0, 0.0,  // d/d(cy)
           0.0, 0.0, 1.0}; // d/d(r)
    break;
  }
  default:
    break;
  }

  return jac;
}

Constraint::Ptr FixConstraint::clone() const {
  auto cloned = std::make_shared<FixConstraint>();
  cloned->m_entity = m_entity;
  cloned->m_fixedX = m_fixedX;
  cloned->m_fixedY = m_fixedY;
  cloned->m_fixedX2 = m_fixedX2;
  cloned->m_fixedY2 = m_fixedY2;
  cloned->m_fixedRadius = m_fixedRadius;
  cloned->setId(id());
  cloned->setDriving(isDriving());
  cloned->setEnabled(isEnabled());
  return cloned;
}

} // namespace sketch
} // namespace opencad
