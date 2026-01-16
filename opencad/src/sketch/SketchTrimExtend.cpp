/**
 * @file SketchTrimExtend.cpp
 * @brief Implementation of trim and extend operations
 *
 * OpenCAD - Modular CAD/CAE Platform
 */

#include "SketchTrimExtend.h"
#include "Sketch.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace opencad {
namespace sketch {

bool SketchTrimExtend::lineLineIntersection(const gp_Pnt2d &l1Start,
                                            const gp_Pnt2d &l1End,
                                            const gp_Pnt2d &l2Start,
                                            const gp_Pnt2d &l2End,
                                            gp_Pnt2d &intersection) {
  double x1 = l1Start.X(), y1 = l1Start.Y();
  double x2 = l1End.X(), y2 = l1End.Y();
  double x3 = l2Start.X(), y3 = l2Start.Y();
  double x4 = l2End.X(), y4 = l2End.Y();

  double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
  if (std::abs(denom) < 1e-10) {
    return false; // Lines are parallel
  }

  double t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;

  intersection = gp_Pnt2d(x1 + t * (x2 - x1), y1 + t * (y2 - y1));
  return true;
}

std::vector<gp_Pnt2d> SketchTrimExtend::lineCircleIntersection(
    const gp_Pnt2d &lineStart, const gp_Pnt2d &lineEnd, const gp_Pnt2d &center,
    double radius) {
  std::vector<gp_Pnt2d> intersections;

  double dx = lineEnd.X() - lineStart.X();
  double dy = lineEnd.Y() - lineStart.Y();
  double fx = lineStart.X() - center.X();
  double fy = lineStart.Y() - center.Y();

  double a = dx * dx + dy * dy;
  double b = 2.0 * (fx * dx + fy * dy);
  double c = fx * fx + fy * fy - radius * radius;

  double discriminant = b * b - 4.0 * a * c;

  if (discriminant < 0) {
    return intersections; // No intersection
  }

  discriminant = std::sqrt(discriminant);

  double t1 = (-b - discriminant) / (2.0 * a);
  double t2 = (-b + discriminant) / (2.0 * a);

  intersections.push_back(
      gp_Pnt2d(lineStart.X() + t1 * dx, lineStart.Y() + t1 * dy));

  if (std::abs(discriminant) > 1e-10) {
    intersections.push_back(
        gp_Pnt2d(lineStart.X() + t2 * dx, lineStart.Y() + t2 * dy));
  }

  return intersections;
}

std::vector<gp_Pnt2d>
SketchTrimExtend::circleCircleIntersection(const gp_Pnt2d &c1, double r1,
                                           const gp_Pnt2d &c2, double r2) {
  std::vector<gp_Pnt2d> intersections;

  double d =
      std::sqrt(std::pow(c2.X() - c1.X(), 2) + std::pow(c2.Y() - c1.Y(), 2));

  if (d > r1 + r2 || d < std::abs(r1 - r2) || d < 1e-10) {
    return intersections; // No intersection or identical circles
  }

  double a = (r1 * r1 - r2 * r2 + d * d) / (2.0 * d);
  double h = std::sqrt(r1 * r1 - a * a);

  double px = c1.X() + a * (c2.X() - c1.X()) / d;
  double py = c1.Y() + a * (c2.Y() - c1.Y()) / d;

  double offsetX = h * (c2.Y() - c1.Y()) / d;
  double offsetY = h * (c2.X() - c1.X()) / d;

  intersections.push_back(gp_Pnt2d(px + offsetX, py - offsetY));
  intersections.push_back(gp_Pnt2d(px - offsetX, py + offsetY));

  return intersections;
}

std::vector<gp_Pnt2d>
SketchTrimExtend::findIntersections(SketchEntity::Ptr entity1,
                                    SketchEntity::Ptr entity2) {
  std::vector<gp_Pnt2d> result;

  if (!entity1 || !entity2)
    return result;

  // Line-Line
  if (entity1->type() == EntityType::Line &&
      entity2->type() == EntityType::Line) {
    auto line1 = std::dynamic_pointer_cast<SketchLine>(entity1);
    auto line2 = std::dynamic_pointer_cast<SketchLine>(entity2);
    if (line1 && line2) {
      gp_Pnt2d intersection;
      if (lineLineIntersection(line1->startPoint(), line1->endPoint(),
                               line2->startPoint(), line2->endPoint(),
                               intersection)) {
        result.push_back(intersection);
      }
    }
  }
  // Line-Circle
  else if (entity1->type() == EntityType::Line &&
           entity2->type() == EntityType::Circle) {
    auto line = std::dynamic_pointer_cast<SketchLine>(entity1);
    auto circle = std::dynamic_pointer_cast<SketchCircle>(entity2);
    if (line && circle) {
      result = lineCircleIntersection(line->startPoint(), line->endPoint(),
                                      circle->center(), circle->radius());
    }
  } else if (entity1->type() == EntityType::Circle &&
             entity2->type() == EntityType::Line) {
    auto circle = std::dynamic_pointer_cast<SketchCircle>(entity1);
    auto line = std::dynamic_pointer_cast<SketchLine>(entity2);
    if (line && circle) {
      result = lineCircleIntersection(line->startPoint(), line->endPoint(),
                                      circle->center(), circle->radius());
    }
  }
  // Circle-Circle
  else if (entity1->type() == EntityType::Circle &&
           entity2->type() == EntityType::Circle) {
    auto circle1 = std::dynamic_pointer_cast<SketchCircle>(entity1);
    auto circle2 = std::dynamic_pointer_cast<SketchCircle>(entity2);
    if (circle1 && circle2) {
      result = circleCircleIntersection(circle1->center(), circle1->radius(),
                                        circle2->center(), circle2->radius());
    }
  }
  // Line-Arc
  else if (entity1->type() == EntityType::Line &&
           entity2->type() == EntityType::Arc) {
    auto line = std::dynamic_pointer_cast<SketchLine>(entity1);
    auto arc = std::dynamic_pointer_cast<SketchArc>(entity2);
    if (line && arc) {
      result = lineCircleIntersection(line->startPoint(), line->endPoint(),
                                      arc->center(), arc->radius());
      // Filter to arc range
      // TODO: Filter intersections that are on the arc segment
    }
  }

  return result;
}

double SketchTrimExtend::distanceToEntity(SketchEntity::Ptr entity,
                                          const gp_Pnt2d &point) {
  if (!entity)
    return std::numeric_limits<double>::max();

  if (entity->type() == EntityType::Line) {
    auto line = std::dynamic_pointer_cast<SketchLine>(entity);
    if (line) {
      // Distance from point to line segment
      gp_Pnt2d s = line->startPoint();
      gp_Pnt2d e = line->endPoint();

      double dx = e.X() - s.X();
      double dy = e.Y() - s.Y();
      double lenSq = dx * dx + dy * dy;

      if (lenSq < 1e-10) {
        return std::sqrt(std::pow(point.X() - s.X(), 2) +
                         std::pow(point.Y() - s.Y(), 2));
      }

      double t = std::max(0.0, std::min(1.0, ((point.X() - s.X()) * dx +
                                              (point.Y() - s.Y()) * dy) /
                                                 lenSq));

      double projX = s.X() + t * dx;
      double projY = s.Y() + t * dy;

      return std::sqrt(std::pow(point.X() - projX, 2) +
                       std::pow(point.Y() - projY, 2));
    }
  } else if (entity->type() == EntityType::Circle) {
    auto circle = std::dynamic_pointer_cast<SketchCircle>(entity);
    if (circle) {
      double distToCenter =
          std::sqrt(std::pow(point.X() - circle->center().X(), 2) +
                    std::pow(point.Y() - circle->center().Y(), 2));
      return std::abs(distToCenter - circle->radius());
    }
  }

  return std::numeric_limits<double>::max();
}

TrimResult SketchTrimExtend::trim(Sketch &sketch, SketchEntity::Ptr entity,
                                  const gp_Pnt2d &clickPoint) {
  TrimResult result;

  if (!entity) {
    result.error = "No entity provided";
    return result;
  }

  // Find all intersections with other entities
  std::vector<std::pair<gp_Pnt2d, double>> intersectionsWithParam;

  for (const auto &other : sketch.entities()) {
    if (other->id() == entity->id())
      continue;

    auto intersections = findIntersections(entity, other);
    for (const auto &pt : intersections) {
      // Calculate parameter along entity
      double param = 0.0;
      if (entity->type() == EntityType::Line) {
        auto line = std::dynamic_pointer_cast<SketchLine>(entity);
        if (line) {
          double dx = line->endPoint().X() - line->startPoint().X();
          double dy = line->endPoint().Y() - line->startPoint().Y();
          double len = std::sqrt(dx * dx + dy * dy);
          if (len > 1e-10) {
            param = ((pt.X() - line->startPoint().X()) * dx +
                     (pt.Y() - line->startPoint().Y()) * dy) /
                    (len * len);
          }
        }
      }
      intersectionsWithParam.push_back({pt, param});
    }
  }

  if (intersectionsWithParam.empty()) {
    result.error = "No intersections found";
    return result;
  }

  // Sort by parameter
  std::sort(intersectionsWithParam.begin(), intersectionsWithParam.end(),
            [](const auto &a, const auto &b) { return a.second < b.second; });

  // Determine which segment to remove based on click point
  if (entity->type() == EntityType::Line) {
    auto line = std::dynamic_pointer_cast<SketchLine>(entity);
    if (line && !intersectionsWithParam.empty()) {
      // Simple case: trim to nearest intersection
      gp_Pnt2d nearestInt = intersectionsWithParam[0].first;
      double minDist = std::numeric_limits<double>::max();

      for (const auto &intPt : intersectionsWithParam) {
        double dist = std::sqrt(std::pow(clickPoint.X() - intPt.first.X(), 2) +
                                std::pow(clickPoint.Y() - intPt.first.Y(), 2));
        if (dist < minDist) {
          minDist = dist;
          nearestInt = intPt.first;
        }
      }

      // Determine which side of intersection the click is on
      double clickDist =
          std::sqrt(std::pow(clickPoint.X() - line->startPoint().X(), 2) +
                    std::pow(clickPoint.Y() - line->startPoint().Y(), 2));
      double intDist =
          std::sqrt(std::pow(nearestInt.X() - line->startPoint().X(), 2) +
                    std::pow(nearestInt.Y() - line->startPoint().Y(), 2));

      if (clickDist < intDist) {
        // Click is between start and intersection - keep end side
        line->setStartPoint(nearestInt);
      } else {
        // Click is between intersection and end - keep start side
        line->setEndPoint(nearestInt);
      }

      result.success = true;
      result.modifiedEntity = line;
    }
  }

  return result;
}

TrimResult
SketchTrimExtend::trimLine(SketchLine::Ptr line,
                           const std::vector<SketchEntity::Ptr> &boundaries,
                           const gp_Pnt2d &clickPoint) {
  TrimResult result;
  // Implementation similar to trim() but for specific line
  result.success = false;
  result.error = "Not implemented";
  return result;
}

TrimResult
SketchTrimExtend::trimArc(SketchArc::Ptr arc,
                          const std::vector<SketchEntity::Ptr> &boundaries,
                          const gp_Pnt2d &clickPoint) {
  TrimResult result;
  result.success = false;
  result.error = "Not implemented";
  return result;
}

ExtendResult SketchTrimExtend::extend(Sketch &sketch, SketchEntity::Ptr entity,
                                      int endpoint) {
  ExtendResult result;

  if (!entity) {
    result.error = "No entity provided";
    return result;
  }

  // Find nearest boundary to extend to
  double minDist = std::numeric_limits<double>::max();
  gp_Pnt2d extendTo;
  bool found = false;

  for (const auto &other : sketch.entities()) {
    if (other->id() == entity->id())
      continue;

    auto intersections = findIntersections(entity, other);
    for (const auto &pt : intersections) {
      gp_Pnt2d refPoint;
      if (entity->type() == EntityType::Line) {
        auto line = std::dynamic_pointer_cast<SketchLine>(entity);
        if (line) {
          refPoint = (endpoint == 0) ? line->startPoint() : line->endPoint();
        }
      }

      double dist = std::sqrt(std::pow(pt.X() - refPoint.X(), 2) +
                              std::pow(pt.Y() - refPoint.Y(), 2));
      if (dist < minDist &&
          dist > 1e-6) { // Must be different from current endpoint
        minDist = dist;
        extendTo = pt;
        found = true;
      }
    }
  }

  if (!found) {
    result.error = "No boundary found to extend to";
    return result;
  }

  // Extend the entity
  if (entity->type() == EntityType::Line) {
    auto line = std::dynamic_pointer_cast<SketchLine>(entity);
    if (line) {
      if (endpoint == 0) {
        line->setStartPoint(extendTo);
      } else {
        line->setEndPoint(extendTo);
      }
      result.success = true;
      result.modifiedEntity = line;
    }
  }

  return result;
}

ExtendResult
SketchTrimExtend::extendLine(SketchLine::Ptr line,
                             const std::vector<SketchEntity::Ptr> &boundaries,
                             int endpoint) {
  ExtendResult result;
  result.success = false;
  result.error = "Not implemented";
  return result;
}

} // namespace sketch
} // namespace opencad
