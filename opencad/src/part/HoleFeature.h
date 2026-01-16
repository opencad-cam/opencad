/**
 * @file HoleFeature.h
 * @brief Hole Wizard - creates various hole types
 *
 * OpenCAD - Modular CAD/CAE Platform
 */
#pragma once

#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <string>
#include <vector>


namespace opencad {
namespace part {

/**
 * @enum HoleType
 * @brief Type of hole
 */
enum class HoleType {
  Simple,      // Simple cylindrical hole
  Counterbore, // Hole with counterbore (flat bottom recess)
  Countersink, // Hole with countersink (angled recess)
  TapDrill,    // Tapped hole (threaded)
  TaperedTap,  // Tapered tap hole
  LegacyHole,  // Basic hole (just diameter and depth)
  Slot         // Slotted hole
};

/**
 * @enum HoleEndCondition
 * @brief How the hole terminates
 */
enum class HoleEndCondition {
  Blind,       // Hole to specific depth
  ThroughAll,  // Hole through entire part
  UpToSurface, // Hole up to selected surface
  UpToVertex   // Hole up to selected point
};

/**
 * @struct HolePosition
 * @brief Position and direction of a hole
 */
struct HolePosition {
  gp_Pnt location;  // Center point of hole
  gp_Dir direction; // Direction of hole (typically face normal)

  HolePosition() : location(0, 0, 0), direction(0, 0, -1) {}
  HolePosition(const gp_Pnt &loc, const gp_Dir &dir)
      : location(loc), direction(dir) {}
};

/**
 * @struct HoleParams
 * @brief Parameters for hole creation
 */
struct HoleParams {
  HoleType type = HoleType::Simple;
  HoleEndCondition endCondition = HoleEndCondition::Blind;

  // Basic hole dimensions
  double diameter = 10.0; // Main hole diameter
  double depth = 20.0;    // Hole depth (for blind holes)

  // Counterbore parameters
  double cboreDiameter = 18.0; // Counterbore diameter
  double cboreDepth = 5.0;     // Counterbore depth

  // Countersink parameters
  double csinkDiameter = 18.0; // Countersink outer diameter
  double csinkAngle = 82.0;    // Countersink angle (degrees)

  // Thread parameters
  std::string threadSize = "M10"; // Thread specification
  double threadPitch = 1.5;       // Thread pitch
  double threadDepth = 15.0;      // Thread depth
  bool threadCosmetic = true;     // Cosmetic (display only) vs modeled thread

  // Hole bottom
  double tipAngle = 118.0; // Drill tip angle for blind holes
  bool flatBottom = false; // Flat bottom instead of drill tip
};

/**
 * @class HoleFeature
 * @brief Creates holes with various configurations (Hole Wizard)
 */
class HoleFeature {
public:
  HoleFeature() = default;

  /**
   * @brief Create a simple hole
   * @param base Base shape to cut hole from
   * @param position Hole position and direction
   * @param diameter Hole diameter
   * @param depth Hole depth
   * @return Shape with hole cut
   */
  TopoDS_Shape createSimpleHole(const TopoDS_Shape &base,
                                const HolePosition &position, double diameter,
                                double depth);

  /**
   * @brief Create a counterbore hole
   * @param base Base shape
   * @param position Hole position
   * @param params Hole parameters
   * @return Shape with counterbore hole
   */
  TopoDS_Shape createCounterbore(const TopoDS_Shape &base,
                                 const HolePosition &position,
                                 const HoleParams &params);

  /**
   * @brief Create a countersink hole
   * @param base Base shape
   * @param position Hole position
   * @param params Hole parameters
   * @return Shape with countersink hole
   */
  TopoDS_Shape createCountersink(const TopoDS_Shape &base,
                                 const HolePosition &position,
                                 const HoleParams &params);

  /**
   * @brief Create a tapped (threaded) hole
   * @param base Base shape
   * @param position Hole position
   * @param params Hole parameters
   * @return Shape with tapped hole
   */
  TopoDS_Shape createTappedHole(const TopoDS_Shape &base,
                                const HolePosition &position,
                                const HoleParams &params);

  /**
   * @brief Create hole with full parameters
   * @param base Base shape
   * @param position Hole position
   * @param params Hole parameters
   * @return Shape with hole
   */
  TopoDS_Shape execute(const TopoDS_Shape &base, const HolePosition &position,
                       const HoleParams &params);

  /**
   * @brief Create multiple holes
   * @param base Base shape
   * @param positions Multiple hole positions
   * @param params Shared hole parameters
   * @return Shape with all holes
   */
  TopoDS_Shape executeMultiple(const TopoDS_Shape &base,
                               const std::vector<HolePosition> &positions,
                               const HoleParams &params);

  /**
   * @brief Get last error message
   */
  const std::string &errorMessage() const { return m_error; }

private:
  std::string m_error;

  /**
   * @brief Create cylinder for cutting
   */
  TopoDS_Shape createCylinder(const gp_Pnt &center, const gp_Dir &direction,
                              double radius, double height);

  /**
   * @brief Create cone for countersink
   */
  TopoDS_Shape createCone(const gp_Pnt &center, const gp_Dir &direction,
                          double topRadius, double bottomRadius, double height);

  /**
   * @brief Create drill tip (cone at bottom of hole)
   */
  TopoDS_Shape createDrillTip(const gp_Pnt &bottomCenter,
                              const gp_Dir &direction, double radius,
                              double tipAngle);
};

} // namespace part
} // namespace opencad
