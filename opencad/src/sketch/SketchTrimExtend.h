/**
 * @file SketchTrimExtend.h
 * @brief Trim and Extend operations for sketch entities
 *
 * OpenCAD - Modular CAD/CAE Platform
 */
#pragma once

#include "entities/SketchArc.h"
#include "entities/SketchCircle.h"
#include "entities/SketchEntity.h"
#include "entities/SketchLine.h"
#include <gp_Pnt2d.hxx>
#include <memory>
#include <vector>

namespace opencad {
namespace sketch {

class Sketch;

/**
 * @enum TrimMode
 * @brief Trim operation mode
 */
enum class TrimMode {
  TrimToNearest, // Trim to nearest intersection
  TrimBetween,   // Trim portion between two intersections
  PowerTrim      // Click and drag to trim multiple entities
};

/**
 * @enum ExtendMode
 * @brief Extend operation mode
 */
enum class ExtendMode {
  ExtendToNearest, // Extend to nearest boundary
  ExtendToEntity   // Extend to specific entity
};

/**
 * @struct TrimResult
 * @brief Result of a trim operation
 */
struct TrimResult {
  bool success = false;
  SketchEntity::Ptr modifiedEntity;           // Modified entity
  std::vector<SketchEntity::Ptr> newEntities; // Created entities (split parts)
  std::vector<SketchEntity::Ptr> removedEntities; // Removed entities
  std::string error;
};

/**
 * @struct ExtendResult
 * @brief Result of an extend operation
 */
struct ExtendResult {
  bool success = false;
  SketchEntity::Ptr modifiedEntity;
  std::string error;
};

/**
 * @class SketchTrimExtend
 * @brief Provides trim and extend functionality for sketch entities
 */
class SketchTrimExtend {
public:
  SketchTrimExtend() = default;

  /**
   * @brief Trim entity at a point
   * @param sketch The sketch containing entities
   * @param entity Entity to trim
   * @param clickPoint Point where user clicked (determines which portion to
   * remove)
   * @return Trim result with created/removed entities
   */
  TrimResult trim(Sketch &sketch, SketchEntity::Ptr entity,
                  const gp_Pnt2d &clickPoint);

  /**
   * @brief Trim line to boundaries
   * @param line Line to trim
   * @param boundaries Boundary entities
   * @param clickPoint Click point to determine trim side
   * @return Trim result
   */
  TrimResult trimLine(SketchLine::Ptr line,
                      const std::vector<SketchEntity::Ptr> &boundaries,
                      const gp_Pnt2d &clickPoint);

  /**
   * @brief Trim arc to boundaries
   * @param arc Arc to trim
   * @param boundaries Boundary entities
   * @param clickPoint Click point to determine trim side
   * @return Trim result
   */
  TrimResult trimArc(SketchArc::Ptr arc,
                     const std::vector<SketchEntity::Ptr> &boundaries,
                     const gp_Pnt2d &clickPoint);

  /**
   * @brief Extend entity to boundary
   * @param sketch The sketch containing entities
   * @param entity Entity to extend
   * @param endpoint Which endpoint to extend (0 = start, 1 = end)
   * @return Extend result
   */
  ExtendResult extend(Sketch &sketch, SketchEntity::Ptr entity, int endpoint);

  /**
   * @brief Extend line to nearest boundary
   * @param line Line to extend
   * @param boundaries Boundary entities
   * @param endpoint Which endpoint (0 = start, 1 = end)
   * @return Extend result
   */
  ExtendResult extendLine(SketchLine::Ptr line,
                          const std::vector<SketchEntity::Ptr> &boundaries,
                          int endpoint);

  /**
   * @brief Find intersections between two entities
   * @param entity1 First entity
   * @param entity2 Second entity
   * @return List of intersection points
   */
  std::vector<gp_Pnt2d> findIntersections(SketchEntity::Ptr entity1,
                                          SketchEntity::Ptr entity2);

private:
  // Line-Line intersection
  bool lineLineIntersection(const gp_Pnt2d &l1Start, const gp_Pnt2d &l1End,
                            const gp_Pnt2d &l2Start, const gp_Pnt2d &l2End,
                            gp_Pnt2d &intersection);

  // Line-Circle intersection
  std::vector<gp_Pnt2d> lineCircleIntersection(const gp_Pnt2d &lineStart,
                                               const gp_Pnt2d &lineEnd,
                                               const gp_Pnt2d &center,
                                               double radius);

  // Circle-Circle intersection
  std::vector<gp_Pnt2d> circleCircleIntersection(const gp_Pnt2d &c1, double r1,
                                                 const gp_Pnt2d &c2, double r2);

  // Find closest point on entity to given point
  gp_Pnt2d closestPointOnEntity(SketchEntity::Ptr entity,
                                const gp_Pnt2d &point);

  // Distance from point to entity
  double distanceToEntity(SketchEntity::Ptr entity, const gp_Pnt2d &point);
};

} // namespace sketch
} // namespace opencad
