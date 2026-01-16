/**
 * @file SketchMirror.cpp
 * @brief Implementation of sketch mirror functionality
 *
 * OpenCAD - Modular CAD/CAE Platform
 */

#include "SketchMirror.h"
#include "Sketch.h"
#include "entities/SketchArc.h"
#include "entities/SketchCircle.h"
#include "entities/SketchLine.h"
#include "entities/SketchPoint.h"
#include <cmath>

namespace opencad {
namespace sketch {

gp_Pnt2d SketchMirror::reflectPoint(const gp_Pnt2d &point,
                                    const gp_Pnt2d &lineStart,
                                    const gp_Pnt2d &lineEnd) const {
  // Line direction
  double dx = lineEnd.X() - lineStart.X();
  double dy = lineEnd.Y() - lineStart.Y();
  double lenSq = dx * dx + dy * dy;

  if (lenSq < 1e-10) {
    return point; // Degenerate line
  }

  // Vector from line start to point
  double px = point.X() - lineStart.X();
  double py = point.Y() - lineStart.Y();

  // Project point onto line
  double t = (px * dx + py * dy) / lenSq;
  double projX = lineStart.X() + t * dx;
  double projY = lineStart.Y() + t * dy;

  // Reflect: new point = 2 * projection - original
  return gp_Pnt2d(2.0 * projX - point.X(), 2.0 * projY - point.Y());
}

SketchEntity::Ptr SketchMirror::mirrorEntity(SketchEntity::Ptr entity,
                                             const gp_Pnt2d &lineStart,
                                             const gp_Pnt2d &lineEnd) {
  if (!entity)
    return nullptr;

  switch (entity->type()) {
  case EntityType::Point: {
    auto point = std::dynamic_pointer_cast<SketchPoint>(entity);
    if (point) {
      gp_Pnt2d reflected =
          reflectPoint(gp_Pnt2d(point->x(), point->y()), lineStart, lineEnd);
      return std::make_shared<SketchPoint>(reflected.X(), reflected.Y());
    }
    break;
  }

  case EntityType::Line: {
    auto line = std::dynamic_pointer_cast<SketchLine>(entity);
    if (line) {
      gp_Pnt2d newStart = reflectPoint(line->startPoint(), lineStart, lineEnd);
      gp_Pnt2d newEnd = reflectPoint(line->endPoint(), lineStart, lineEnd);
      auto newLine = std::make_shared<SketchLine>(newStart, newEnd);
      newLine->setConstruction(line->isConstruction());
      return newLine;
    }
    break;
  }

  case EntityType::Circle: {
    auto circle = std::dynamic_pointer_cast<SketchCircle>(entity);
    if (circle) {
      gp_Pnt2d newCenter = reflectPoint(circle->center(), lineStart, lineEnd);
      return std::make_shared<SketchCircle>(newCenter, circle->radius());
    }
    break;
  }

  case EntityType::Arc: {
    auto arc = std::dynamic_pointer_cast<SketchArc>(entity);
    if (arc) {
      gp_Pnt2d newCenter = reflectPoint(arc->center(), lineStart, lineEnd);

      // Mirror angles (flip direction)
      double startAngle = arc->startAngle();
      double endAngle = arc->endAngle();

      // Calculate line angle for proper angle mirroring
      double lineAngle =
          std::atan2(lineEnd.Y() - lineStart.Y(), lineEnd.X() - lineStart.X());

      // Mirror angles about line angle
      double newStart = 2.0 * lineAngle - endAngle;
      double newEnd = 2.0 * lineAngle - startAngle;

      return std::make_shared<SketchArc>(newCenter, arc->radius(), newStart,
                                         newEnd);
    }
    break;
  }

  default:
    break;
  }

  return nullptr;
}

MirrorResult
SketchMirror::mirror(Sketch &sketch,
                     const std::vector<SketchEntity::Ptr> &entities,
                     SketchLine::Ptr mirrorLine, bool copyEntities) {
  MirrorResult result;

  if (!mirrorLine) {
    result.error = "No mirror line provided";
    return result;
  }

  if (entities.empty()) {
    result.error = "No entities to mirror";
    return result;
  }

  gp_Pnt2d lineStart = mirrorLine->startPoint();
  gp_Pnt2d lineEnd = mirrorLine->endPoint();

  for (const auto &entity : entities) {
    // Don't mirror the mirror line itself
    if (entity->id() == mirrorLine->id()) {
      continue;
    }

    SketchEntity::Ptr mirrored = mirrorEntity(entity, lineStart, lineEnd);
    if (mirrored) {
      sketch.addEntity(mirrored);
      result.mirroredEntities.push_back(mirrored);
    }
  }

  result.success = !result.mirroredEntities.empty();
  return result;
}

MirrorResult
SketchMirror::mirrorVertical(Sketch &sketch,
                             const std::vector<SketchEntity::Ptr> &entities,
                             double axisX) {
  MirrorResult result;

  // Create a virtual vertical line at axisX
  gp_Pnt2d lineStart(axisX, 0.0);
  gp_Pnt2d lineEnd(axisX, 1.0);

  for (const auto &entity : entities) {
    SketchEntity::Ptr mirrored = mirrorEntity(entity, lineStart, lineEnd);
    if (mirrored) {
      sketch.addEntity(mirrored);
      result.mirroredEntities.push_back(mirrored);
    }
  }

  result.success = !result.mirroredEntities.empty();
  return result;
}

MirrorResult
SketchMirror::mirrorHorizontal(Sketch &sketch,
                               const std::vector<SketchEntity::Ptr> &entities,
                               double axisY) {
  MirrorResult result;

  // Create a virtual horizontal line at axisY
  gp_Pnt2d lineStart(0.0, axisY);
  gp_Pnt2d lineEnd(1.0, axisY);

  for (const auto &entity : entities) {
    SketchEntity::Ptr mirrored = mirrorEntity(entity, lineStart, lineEnd);
    if (mirrored) {
      sketch.addEntity(mirrored);
      result.mirroredEntities.push_back(mirrored);
    }
  }

  result.success = !result.mirroredEntities.empty();
  return result;
}

MirrorResult
SketchMirror::mirrorAboutPoint(Sketch &sketch,
                               const std::vector<SketchEntity::Ptr> &entities,
                               const gp_Pnt2d &origin) {
  MirrorResult result;

  for (const auto &entity : entities) {
    switch (entity->type()) {
    case EntityType::Point: {
      auto point = std::dynamic_pointer_cast<SketchPoint>(entity);
      if (point) {
        double newX = 2.0 * origin.X() - point->x();
        double newY = 2.0 * origin.Y() - point->y();
        auto mirrored = std::make_shared<SketchPoint>(newX, newY);
        sketch.addEntity(mirrored);
        result.mirroredEntities.push_back(mirrored);
      }
      break;
    }

    case EntityType::Line: {
      auto line = std::dynamic_pointer_cast<SketchLine>(entity);
      if (line) {
        gp_Pnt2d s = line->startPoint();
        gp_Pnt2d e = line->endPoint();
        gp_Pnt2d newStart(2.0 * origin.X() - s.X(), 2.0 * origin.Y() - s.Y());
        gp_Pnt2d newEnd(2.0 * origin.X() - e.X(), 2.0 * origin.Y() - e.Y());
        auto mirrored = std::make_shared<SketchLine>(newStart, newEnd);
        mirrored->setConstruction(line->isConstruction());
        sketch.addEntity(mirrored);
        result.mirroredEntities.push_back(mirrored);
      }
      break;
    }

    case EntityType::Circle: {
      auto circle = std::dynamic_pointer_cast<SketchCircle>(entity);
      if (circle) {
        gp_Pnt2d c = circle->center();
        gp_Pnt2d newCenter(2.0 * origin.X() - c.X(), 2.0 * origin.Y() - c.Y());
        auto mirrored =
            std::make_shared<SketchCircle>(newCenter, circle->radius());
        sketch.addEntity(mirrored);
        result.mirroredEntities.push_back(mirrored);
      }
      break;
    }

    default:
      break;
    }
  }

  result.success = !result.mirroredEntities.empty();
  return result;
}

} // namespace sketch
} // namespace opencad
