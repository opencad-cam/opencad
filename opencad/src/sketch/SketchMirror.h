/**
 * @file SketchMirror.h
 * @brief Mirror entities within a sketch
 *
 * OpenCAD - Modular CAD/CAE Platform
 */
#pragma once

#include "entities/SketchEntity.h"
#include "entities/SketchLine.h"
#include <gp_Pnt2d.hxx>
#include <memory>
#include <vector>


namespace opencad {
namespace sketch {

class Sketch;

/**
 * @struct MirrorResult
 * @brief Result of mirror operation
 */
struct MirrorResult {
  bool success = false;
  std::vector<SketchEntity::Ptr> mirroredEntities;
  std::string error;
};

/**
 * @class SketchMirror
 * @brief Provides mirror functionality for sketch entities
 *
 * Mirrors selected entities about a line (construction or regular).
 */
class SketchMirror {
public:
  SketchMirror() = default;

  /**
   * @brief Mirror entities about a line
   * @param sketch The sketch to add mirrored entities to
   * @param entities Entities to mirror
   * @param mirrorLine Line to mirror about
   * @param copyEntities If true, keep original entities; if false, move them
   * @return Mirror result with created entities
   */
  MirrorResult mirror(Sketch &sketch,
                      const std::vector<SketchEntity::Ptr> &entities,
                      SketchLine::Ptr mirrorLine, bool copyEntities = true);

  /**
   * @brief Mirror entities about a vertical axis at given X
   * @param sketch The sketch
   * @param entities Entities to mirror
   * @param axisX X coordinate of vertical mirror axis
   * @return Mirror result
   */
  MirrorResult mirrorVertical(Sketch &sketch,
                              const std::vector<SketchEntity::Ptr> &entities,
                              double axisX);

  /**
   * @brief Mirror entities about a horizontal axis at given Y
   * @param sketch The sketch
   * @param entities Entities to mirror
   * @param axisY Y coordinate of horizontal mirror axis
   * @return Mirror result
   */
  MirrorResult mirrorHorizontal(Sketch &sketch,
                                const std::vector<SketchEntity::Ptr> &entities,
                                double axisY);

  /**
   * @brief Mirror entities about origin (point reflection)
   * @param sketch The sketch
   * @param entities Entities to mirror
   * @param origin Origin point for reflection
   * @return Mirror result
   */
  MirrorResult mirrorAboutPoint(Sketch &sketch,
                                const std::vector<SketchEntity::Ptr> &entities,
                                const gp_Pnt2d &origin);

private:
  /**
   * @brief Reflect a point about a line
   */
  gp_Pnt2d reflectPoint(const gp_Pnt2d &point, const gp_Pnt2d &lineStart,
                        const gp_Pnt2d &lineEnd) const;

  /**
   * @brief Mirror a single entity
   */
  SketchEntity::Ptr mirrorEntity(SketchEntity::Ptr entity,
                                 const gp_Pnt2d &lineStart,
                                 const gp_Pnt2d &lineEnd);
};

} // namespace sketch
} // namespace opencad
