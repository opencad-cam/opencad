/**
 * @file SketchPattern.cpp
 * @brief Implementation of sketch pattern functionality
 *
 * OpenCAD - Modular CAD/CAE Platform
 */

#include "SketchPattern.h"
#include "Sketch.h"
#include "entities/SketchArc.h"
#include "entities/SketchCircle.h"
#include "entities/SketchLine.h"
#include "entities/SketchPoint.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace opencad {
namespace sketch {

gp_Pnt2d SketchPattern::rotatePoint(const gp_Pnt2d &point,
                                    const gp_Pnt2d &center, double angleRad) {
  double dx = point.X() - center.X();
  double dy = point.Y() - center.Y();

  double cosA = std::cos(angleRad);
  double sinA = std::sin(angleRad);

  double newX = center.X() + dx * cosA - dy * sinA;
  double newY = center.Y() + dx * sinA + dy * cosA;

  return gp_Pnt2d(newX, newY);
}

SketchEntity::Ptr SketchPattern::translateEntity(SketchEntity::Ptr entity,
                                                 double dx, double dy) {
  if (!entity)
    return nullptr;

  switch (entity->type()) {
  case EntityType::Point: {
    auto point = std::dynamic_pointer_cast<SketchPoint>(entity);
    if (point) {
      return std::make_shared<SketchPoint>(point->x() + dx, point->y() + dy);
    }
    break;
  }

  case EntityType::Line: {
    auto line = std::dynamic_pointer_cast<SketchLine>(entity);
    if (line) {
      gp_Pnt2d newStart(line->startPoint().X() + dx,
                        line->startPoint().Y() + dy);
      gp_Pnt2d newEnd(line->endPoint().X() + dx, line->endPoint().Y() + dy);
      auto newLine = std::make_shared<SketchLine>(newStart, newEnd);
      newLine->setConstruction(line->isConstruction());
      return newLine;
    }
    break;
  }

  case EntityType::Circle: {
    auto circle = std::dynamic_pointer_cast<SketchCircle>(entity);
    if (circle) {
      gp_Pnt2d newCenter(circle->center().X() + dx, circle->center().Y() + dy);
      return std::make_shared<SketchCircle>(newCenter, circle->radius());
    }
    break;
  }

  case EntityType::Arc: {
    auto arc = std::dynamic_pointer_cast<SketchArc>(entity);
    if (arc) {
      gp_Pnt2d newCenter(arc->center().X() + dx, arc->center().Y() + dy);
      return std::make_shared<SketchArc>(newCenter, arc->radius(),
                                         arc->startAngle(), arc->endAngle());
    }
    break;
  }

  default:
    break;
  }

  return nullptr;
}

SketchEntity::Ptr SketchPattern::rotateEntity(SketchEntity::Ptr entity,
                                              const gp_Pnt2d &center,
                                              double angleRad) {
  if (!entity)
    return nullptr;

  switch (entity->type()) {
  case EntityType::Point: {
    auto point = std::dynamic_pointer_cast<SketchPoint>(entity);
    if (point) {
      gp_Pnt2d rotated =
          rotatePoint(gp_Pnt2d(point->x(), point->y()), center, angleRad);
      return std::make_shared<SketchPoint>(rotated.X(), rotated.Y());
    }
    break;
  }

  case EntityType::Line: {
    auto line = std::dynamic_pointer_cast<SketchLine>(entity);
    if (line) {
      gp_Pnt2d newStart = rotatePoint(line->startPoint(), center, angleRad);
      gp_Pnt2d newEnd = rotatePoint(line->endPoint(), center, angleRad);
      auto newLine = std::make_shared<SketchLine>(newStart, newEnd);
      newLine->setConstruction(line->isConstruction());
      return newLine;
    }
    break;
  }

  case EntityType::Circle: {
    auto circle = std::dynamic_pointer_cast<SketchCircle>(entity);
    if (circle) {
      gp_Pnt2d newCenter = rotatePoint(circle->center(), center, angleRad);
      return std::make_shared<SketchCircle>(newCenter, circle->radius());
    }
    break;
  }

  case EntityType::Arc: {
    auto arc = std::dynamic_pointer_cast<SketchArc>(entity);
    if (arc) {
      gp_Pnt2d newCenter = rotatePoint(arc->center(), center, angleRad);
      // Also rotate the arc's angular position
      double newStartAngle = arc->startAngle() + angleRad;
      double newEndAngle = arc->endAngle() + angleRad;
      return std::make_shared<SketchArc>(newCenter, arc->radius(),
                                         newStartAngle, newEndAngle);
    }
    break;
  }

  default:
    break;
  }

  return nullptr;
}

PatternResult
SketchPattern::linearPattern(Sketch &sketch,
                             const std::vector<SketchEntity::Ptr> &entities,
                             const LinearPatternParams &params) {
  PatternResult result;

  if (entities.empty()) {
    result.error = "No entities to pattern";
    return result;
  }

  if (params.count < 2) {
    result.error = "Count must be at least 2";
    return result;
  }

  // Normalize direction
  double len = std::sqrt(params.directionX * params.directionX +
                         params.directionY * params.directionY);
  if (len < 1e-10) {
    result.error = "Invalid direction";
    return result;
  }

  double dirX = params.directionX / len;
  double dirY = params.directionY / len;

  // Create copies in primary direction
  for (int i = 1; i < params.count; ++i) {
    double dx = dirX * params.spacing * i;
    double dy = dirY * params.spacing * i;

    for (const auto &entity : entities) {
      SketchEntity::Ptr copy = translateEntity(entity, dx, dy);
      if (copy) {
        sketch.addEntity(copy);
        result.createdEntities.push_back(copy);
      }
    }
  }

  // If bidirectional, also create in opposite direction
  if (params.bidirectional && params.count2 > 1) {
    for (int i = 1; i < params.count2; ++i) {
      double dx = -dirX * params.spacing * i;
      double dy = -dirY * params.spacing * i;

      for (const auto &entity : entities) {
        SketchEntity::Ptr copy = translateEntity(entity, dx, dy);
        if (copy) {
          sketch.addEntity(copy);
          result.createdEntities.push_back(copy);
        }
      }
    }
  }

  result.success = !result.createdEntities.empty();
  return result;
}

PatternResult
SketchPattern::circularPattern(Sketch &sketch,
                               const std::vector<SketchEntity::Ptr> &entities,
                               const CircularPatternParams &params) {
  PatternResult result;

  if (entities.empty()) {
    result.error = "No entities to pattern";
    return result;
  }

  if (params.count < 2) {
    result.error = "Count must be at least 2";
    return result;
  }

  gp_Pnt2d center(params.centerX, params.centerY);

  // Calculate angle step
  double angleStep;
  if (params.equalSpacing) {
    angleStep = (params.totalAngle * M_PI / 180.0) / params.count;
  } else {
    angleStep = params.angle * M_PI / 180.0;
  }

  // Create rotated copies
  for (int i = 1; i < params.count; ++i) {
    double angle = angleStep * i;

    for (const auto &entity : entities) {
      SketchEntity::Ptr copy = rotateEntity(entity, center, angle);
      if (copy) {
        sketch.addEntity(copy);
        result.createdEntities.push_back(copy);
      }
    }
  }

  result.success = !result.createdEntities.empty();
  return result;
}

PatternResult SketchPattern::linearPattern(
    Sketch &sketch, const std::vector<SketchEntity::Ptr> &entities, double dirX,
    double dirY, double spacing, int count) {
  LinearPatternParams params;
  params.directionX = dirX;
  params.directionY = dirY;
  params.spacing = spacing;
  params.count = count;
  return linearPattern(sketch, entities, params);
}

PatternResult
SketchPattern::circularPattern(Sketch &sketch,
                               const std::vector<SketchEntity::Ptr> &entities,
                               double centerX, double centerY, int count) {
  CircularPatternParams params;
  params.centerX = centerX;
  params.centerY = centerY;
  params.count = count;
  params.totalAngle = 360.0;
  params.equalSpacing = true;
  return circularPattern(sketch, entities, params);
}

} // namespace sketch
} // namespace opencad
