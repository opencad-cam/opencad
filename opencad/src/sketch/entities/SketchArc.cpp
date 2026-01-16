/**
 * @file SketchArc.cpp
 * @brief Implementation of SketchArc
 */

#include "SketchArc.h"
#include <Geom2d_Circle.hxx>
#include <Geom2d_TrimmedCurve.hxx>
#include <cmath>
#include <stdexcept>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace opencad {
namespace sketch {

SketchArc::SketchArc()
    : SketchEntity(), m_center(0.0, 0.0), m_radius(1.0), m_startAngle(0.0),
      m_endAngle(M_PI / 2.0), m_hasThreePointData(false), m_arcStart(0.0, 0.0),
      m_arcEnd(0.0, 0.0), m_arcThrough(0.0, 0.0) {}

SketchArc::SketchArc(const gp_Pnt2d &center, double radius, double startAngle,
                     double endAngle)
    : SketchEntity(), m_center(center), m_radius(radius),
      m_startAngle(startAngle), m_endAngle(endAngle),
      m_hasThreePointData(false), m_arcStart(0.0, 0.0), m_arcEnd(0.0, 0.0),
      m_arcThrough(0.0, 0.0) {}

SketchArc::SketchArc(const gp_Pnt2d &start, const gp_Pnt2d &end,
                     const gp_Pnt2d &through)
    : SketchEntity(), m_center(0.0, 0.0), m_radius(1.0), m_startAngle(0.0),
      m_endAngle(M_PI), m_hasThreePointData(true), m_arcStart(start),
      m_arcEnd(end), m_arcThrough(through) {
  // Compute center and radius from three points
  // Order: start -> through -> end (matching GC_MakeArcOfCircle)
  computeFromThreePoints(start, through, end);
}

void SketchArc::computeFromThreePoints(const gp_Pnt2d &pStart,
                                       const gp_Pnt2d &pThrough,
                                       const gp_Pnt2d &pEnd) {
  // Find the two farthest points - they will be START and END
  // The remaining point will be THROUGH (point on arc)
  double dist_AB = pStart.Distance(pThrough);
  double dist_BC = pThrough.Distance(pEnd);
  double dist_AC = pStart.Distance(pEnd);

  gp_Pnt2d actualStart, actualThrough, actualEnd;

  if (dist_AC >= dist_AB && dist_AC >= dist_BC) {
    // pStart and pEnd are farthest - pThrough is through point
    actualStart = pStart;
    actualEnd = pEnd;
    actualThrough = pThrough;
  } else if (dist_AB >= dist_AC && dist_AB >= dist_BC) {
    // pStart and pThrough are farthest - pEnd is through point
    actualStart = pStart;
    actualEnd = pThrough;
    actualThrough = pEnd;
  } else {
    // pThrough and pEnd are farthest - pStart is through point
    actualStart = pThrough;
    actualEnd = pEnd;
    actualThrough = pStart;
  }

  // Update stored three-point data with actual positions
  m_arcStart = actualStart;
  m_arcEnd = actualEnd;
  m_arcThrough = actualThrough;

  // Calculate circumcenter of triangle formed by three points
  double ax = actualStart.X(), ay = actualStart.Y();
  double bx = actualThrough.X(), by = actualThrough.Y();
  double cx = actualEnd.X(), cy = actualEnd.Y();

  double d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));

  if (std::abs(d) < 1e-10) {
    // Points are collinear - invalid arc
    m_radius = 0.0;
    return;
  }

  double aSq = ax * ax + ay * ay;
  double bSq = bx * bx + by * by;
  double cSq = cx * cx + cy * cy;

  double centerX = (aSq * (by - cy) + bSq * (cy - ay) + cSq * (ay - by)) / d;
  double centerY = (aSq * (cx - bx) + bSq * (ax - cx) + cSq * (bx - ax)) / d;

  m_center.SetCoord(centerX, centerY);
  m_radius = m_center.Distance(actualStart);

  // Calculate angles from center
  double startAngle =
      std::atan2(actualStart.Y() - centerY, actualStart.X() - centerX);
  double throughAngle =
      std::atan2(actualThrough.Y() - centerY, actualThrough.X() - centerX);
  double endAngle =
      std::atan2(actualEnd.Y() - centerY, actualEnd.X() - centerX);

  // Determine arc direction by checking if "through" is between start and end
  // going CCW or CW.
  auto normalizeAngle = [](double angle) {
    while (angle < 0)
      angle += 2.0 * M_PI;
    while (angle >= 2.0 * M_PI)
      angle -= 2.0 * M_PI;
    return angle;
  };

  // Normalize all angles to [0, 2*PI)
  startAngle = normalizeAngle(startAngle);
  throughAngle = normalizeAngle(throughAngle);
  endAngle = normalizeAngle(endAngle);

  // Calculate CCW distance from start to a target angle
  auto ccwDistance = [](double from, double to) {
    double dist = to - from;
    if (dist < 0)
      dist += 2.0 * M_PI;
    return dist;
  };

  // CCW distances from start
  double ccwToThrough = ccwDistance(startAngle, throughAngle);
  double ccwToEnd = ccwDistance(startAngle, endAngle);

  // If going CCW from start, we should hit "through" before "end"
  // for the arc to include the through point
  bool goCCW = (ccwToThrough < ccwToEnd);

  m_startAngle = startAngle;

  if (goCCW) {
    // CCW: end angle is greater than start
    m_endAngle = startAngle + ccwToEnd;
  } else {
    // CW: end angle is less than start (negative sweep)
    double cwToEnd = 2.0 * M_PI - ccwToEnd;
    m_endAngle = startAngle - cwToEnd;
  }
}

double SketchArc::sweepAngle() const {
  double sweep = m_endAngle - m_startAngle;
  // Normalize to (-2*PI, 2*PI)
  while (sweep > 2.0 * M_PI)
    sweep -= 2.0 * M_PI;
  while (sweep < -2.0 * M_PI)
    sweep += 2.0 * M_PI;
  return sweep;
}

Handle(Geom2d_Curve) SketchArc::curve() const {
  if (!isValid()) {
    return Handle(Geom2d_Curve)();
  }

  gp_Ax2d axis(m_center, gp_Dir2d(1.0, 0.0));
  Handle(Geom2d_Circle) fullCircle =
      new Geom2d_Circle(gp_Circ2d(axis, m_radius));

  return new Geom2d_TrimmedCurve(fullCircle, m_startAngle, m_endAngle);
}

gp_Circ2d SketchArc::circle2d() const {
  gp_Ax2d axis(m_center, gp_Dir2d(1.0, 0.0));
  return gp_Circ2d(axis, m_radius);
}

gp_Pnt2d SketchArc::startPoint() const {
  if (m_hasThreePointData) {
    return m_arcStart;
  }
  return pointAtAngle(m_startAngle);
}

gp_Pnt2d SketchArc::endPoint() const {
  if (m_hasThreePointData) {
    return m_arcEnd;
  }
  return pointAtAngle(m_endAngle);
}

gp_Pnt2d SketchArc::midPoint() const {
  if (m_hasThreePointData) {
    return m_arcThrough;
  }
  double midAngle = (m_startAngle + m_endAngle) / 2.0;
  return pointAtAngle(midAngle);
}

double SketchArc::length() const { return std::abs(sweepAngle()) * m_radius; }

gp_Pnt2d SketchArc::pointAtAngle(double angle) const {
  return gp_Pnt2d(m_center.X() + m_radius * std::cos(angle),
                  m_center.Y() + m_radius * std::sin(angle));
}

gp_Pnt2d SketchArc::pointAtParameter(double t) const {
  double angle = m_startAngle + t * sweepAngle();
  return pointAtAngle(angle);
}

double SketchArc::distanceToPoint(const gp_Pnt2d &point) const {
  double dist = m_center.Distance(point);
  return std::abs(dist - m_radius);
}

double SketchArc::getParameter(int index) const {
  switch (index) {
  case 0:
    return m_center.X();
  case 1:
    return m_center.Y();
  case 2:
    return m_radius;
  case 3:
    return m_startAngle;
  case 4:
    return m_endAngle;
  default:
    return 0.0;
  }
}

void SketchArc::setParameter(int index, double value) {
  switch (index) {
  case 0:
    m_center.SetX(value);
    break;
  case 1:
    m_center.SetY(value);
    break;
  case 2:
    m_radius = value;
    break;
  case 3:
    m_startAngle = value;
    break;
  case 4:
    m_endAngle = value;
    break;
  }
}

SketchEntity::Ptr SketchArc::clone() const {
  if (m_hasThreePointData) {
    return std::make_shared<SketchArc>(m_arcStart, m_arcEnd, m_arcThrough);
  }
  auto cloned =
      std::make_shared<SketchArc>(m_center, m_radius, m_startAngle, m_endAngle);
  cloned->setConstruction(isConstruction());
  return cloned;
}

bool SketchArc::isValid() const {
  if (m_radius < 1e-10) {
    return false;
  }
  // Check that sweep is not zero
  if (std::abs(sweepAngle()) < 1e-10) {
    return false;
  }
  return true;
}

} // namespace sketch
} // namespace opencad
