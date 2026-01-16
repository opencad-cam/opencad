/**
 * @file ThickenFeature.h
 * @brief Thicken feature - converts surfaces to solids
 *
 * OpenCAD - Modular CAD/CAE Platform
 */
#pragma once

#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <string>
#include <vector>


namespace opencad {
namespace part {

/**
 * @enum ThickenDirection
 * @brief Direction for thickening
 */
enum class ThickenDirection {
  Normal,  // Thicken along face normal
  Reverse, // Thicken against face normal
  Both     // Thicken equally in both directions (mid-surface)
};

/**
 * @struct ThickenParams
 * @brief Parameters for thicken operation
 */
struct ThickenParams {
  double thickness1 = 1.0; // Thickness in normal direction
  double thickness2 = 0.0; // Thickness in reverse direction (for Both)
  ThickenDirection direction = ThickenDirection::Normal;
};

/**
 * @class ThickenFeature
 * @brief Converts surfaces/shells to solid bodies by adding thickness
 *
 * Similar to SolidWorks Thicken feature. Takes a surface and creates
 * a solid by offsetting in perpendicular direction.
 */
class ThickenFeature {
public:
  ThickenFeature() = default;

  /**
   * @brief Thicken a face to create a solid
   * @param face Face to thicken
   * @param thickness Thickness value
   * @param direction Thicken direction
   * @return Solid shape
   */
  TopoDS_Shape execute(const TopoDS_Face &face, double thickness,
                       ThickenDirection direction = ThickenDirection::Normal);

  /**
   * @brief Thicken multiple faces
   * @param faces Faces to thicken
   * @param thickness Thickness value
   * @return Combined solid shape
   */
  TopoDS_Shape execute(const std::vector<TopoDS_Face> &faces, double thickness);

  /**
   * @brief Thicken a shell/surface shape
   * @param shape Surface or shell shape
   * @param thickness Thickness value
   * @return Solid shape
   */
  TopoDS_Shape execute(const TopoDS_Shape &shape, double thickness);

  /**
   * @brief Thicken with full parameters
   * @param shape Input shape
   * @param params Thicken parameters
   * @return Solid shape
   */
  TopoDS_Shape execute(const TopoDS_Shape &shape, const ThickenParams &params);

  /**
   * @brief Get last error message
   */
  const std::string &errorMessage() const { return m_error; }

private:
  std::string m_error;
};

} // namespace part
} // namespace opencad
