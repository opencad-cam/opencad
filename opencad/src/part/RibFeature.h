/**
 * @file RibFeature.h
 * @brief Rib feature - creates structural ribs
 *
 * OpenCAD - Modular CAD/CAE Platform
 */
#pragma once

#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Dir.hxx>
#include <string>

namespace opencad {
namespace sketch {
class Sketch;
}
namespace part {

/**
 * @enum RibType
 * @brief Type of rib to create
 */
enum class RibType {
  Parallel, // Rib parallel to sketch plane
  Normal,   // Rib normal to sketch plane
  AtAngle   // Rib at specified angle
};

/**
 * @struct RibParams
 * @brief Parameters for rib operation
 */
struct RibParams {
  double thickness = 2.0; // Rib thickness
  RibType type = RibType::Parallel;
  bool symmetric = true;      // Extrude thickness equally both sides
  double angle = 0.0;         // Angle for AtAngle type (degrees)
  bool flipDirection = false; // Flip extrusion direction
  double draftAngle = 0.0;    // Optional draft angle (degrees)
};

/**
 * @class RibFeature
 * @brief Creates structural ribs from an open sketch profile
 *
 * Ribs are thin wall features that connect to existing geometry
 * to provide structural support.
 */
class RibFeature {
public:
  RibFeature() = default;

  /**
   * @brief Create a rib from sketch
   * @param sketch Open sketch profile (line or spline)
   * @param base Base shape to connect rib to
   * @param thickness Rib thickness
   * @param symmetric Extrude symmetrically from sketch plane
   * @return Shape with rib added
   */
  TopoDS_Shape execute(const sketch::Sketch &sketch, const TopoDS_Shape &base,
                       double thickness, bool symmetric = true);

  /**
   * @brief Create a rib with full parameters
   * @param sketch Open sketch profile
   * @param base Base shape
   * @param params Rib parameters
   * @return Shape with rib added
   */
  TopoDS_Shape execute(const sketch::Sketch &sketch, const TopoDS_Shape &base,
                       const RibParams &params);

  /**
   * @brief Create a rib from a wire
   * @param profile Open wire profile
   * @param base Base shape
   * @param direction Extrusion direction
   * @param thickness Rib thickness
   * @return Shape with rib added
   */
  TopoDS_Shape executeWire(const TopoDS_Wire &profile, const TopoDS_Shape &base,
                           const gp_Dir &direction, double thickness);

  /**
   * @brief Get last error message
   */
  const std::string &errorMessage() const { return m_error; }

private:
  std::string m_error;

  /**
   * @brief Create the rib solid from profile
   */
  TopoDS_Shape createRibSolid(const TopoDS_Wire &profile,
                              const gp_Dir &direction, double thickness,
                              bool symmetric);

  /**
   * @brief Trim rib to intersect with base shape
   */
  TopoDS_Shape trimRibToBase(const TopoDS_Shape &rib, const TopoDS_Shape &base);
};

} // namespace part
} // namespace opencad
