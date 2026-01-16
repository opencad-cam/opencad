/**
 * @file SketchPattern.h
 * @brief Linear and Circular pattern for sketch entities
 *
 * OpenCAD - Modular CAD/CAE Platform
 */
#pragma once

#include "entities/SketchEntity.h"
#include <gp_Pnt2d.hxx>
#include <memory>
#include <vector>


namespace opencad {
namespace sketch {

class Sketch;

/**
 * @struct LinearPatternParams
 * @brief Parameters for linear pattern
 */
struct LinearPatternParams {
  double directionX = 1.0;    // X component of direction
  double directionY = 0.0;    // Y component of direction
  double spacing = 10.0;      // Spacing between instances
  int count = 3;              // Number of copies (including original)
  bool bidirectional = false; // Create pattern in both directions
  int count2 = 1;             // Count for second direction (if bidirectional)
};

/**
 * @struct CircularPatternParams
 * @brief Parameters for circular pattern
 */
struct CircularPatternParams {
  double centerX = 0.0;      // Center point X
  double centerY = 0.0;      // Center point Y
  double totalAngle = 360.0; // Total angle span (degrees)
  int count = 6;             // Number of copies (including original)
  bool equalSpacing = true;  // Equal spacing (vs. specified angle)
  double angle = 60.0;       // Angle between copies if not equal spacing
};

/**
 * @struct PatternResult
 * @brief Result of pattern operation
 */
struct PatternResult {
  bool success = false;
  std::vector<SketchEntity::Ptr> createdEntities;
  std::string error;
};

/**
 * @class SketchPattern
 * @brief Provides linear and circular pattern functionality for sketch entities
 */
class SketchPattern {
public:
  SketchPattern() = default;

  /**
   * @brief Create linear pattern of entities
   * @param sketch The sketch to add pattern to
   * @param entities Entities to pattern
   * @param params Linear pattern parameters
   * @return Pattern result with created entities
   */
  PatternResult linearPattern(Sketch &sketch,
                              const std::vector<SketchEntity::Ptr> &entities,
                              const LinearPatternParams &params);

  /**
   * @brief Create circular pattern of entities
   * @param sketch The sketch to add pattern to
   * @param entities Entities to pattern
   * @param params Circular pattern parameters
   * @return Pattern result with created entities
   */
  PatternResult circularPattern(Sketch &sketch,
                                const std::vector<SketchEntity::Ptr> &entities,
                                const CircularPatternParams &params);

  /**
   * @brief Create linear pattern with simple parameters
   * @param sketch The sketch
   * @param entities Entities to pattern
   * @param dirX X direction component
   * @param dirY Y direction component
   * @param spacing Spacing between copies
   * @param count Number of copies
   * @return Pattern result
   */
  PatternResult linearPattern(Sketch &sketch,
                              const std::vector<SketchEntity::Ptr> &entities,
                              double dirX, double dirY, double spacing,
                              int count);

  /**
   * @brief Create circular pattern with simple parameters
   * @param sketch The sketch
   * @param entities Entities to pattern
   * @param centerX Center X coordinate
   * @param centerY Center Y coordinate
   * @param count Number of copies around the circle
   * @return Pattern result
   */
  PatternResult circularPattern(Sketch &sketch,
                                const std::vector<SketchEntity::Ptr> &entities,
                                double centerX, double centerY, int count);

private:
  /**
   * @brief Translate an entity by offset
   */
  SketchEntity::Ptr translateEntity(SketchEntity::Ptr entity, double dx,
                                    double dy);

  /**
   * @brief Rotate an entity around a point
   */
  SketchEntity::Ptr rotateEntity(SketchEntity::Ptr entity,
                                 const gp_Pnt2d &center, double angleRad);

  /**
   * @brief Rotate a point around center
   */
  gp_Pnt2d rotatePoint(const gp_Pnt2d &point, const gp_Pnt2d &center,
                       double angleRad);
};

} // namespace sketch
} // namespace opencad
