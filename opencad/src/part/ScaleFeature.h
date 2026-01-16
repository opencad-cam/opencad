/**
 * @file ScaleFeature.h
 * @brief Scale feature - scales a body uniformly or non-uniformly
 *
 * OpenCAD - Modular CAD/CAE Platform
 */
#pragma once

#include <TopoDS_Shape.hxx>
#include <gp_Pnt.hxx>
#include <string>

namespace opencad {
namespace part {

/**
 * @enum ScaleType
 * @brief Type of scaling
 */
enum class ScaleType {
  Uniform,   // Same scale in all directions
  NonUniform // Different scale in X, Y, Z
};

/**
 * @struct ScaleParams
 * @brief Parameters for scale operation
 */
struct ScaleParams {
  double scaleFactor = 2.0; // Uniform scale factor
  double scaleX = 1.0;      // X scale (for non-uniform)
  double scaleY = 1.0;      // Y scale
  double scaleZ = 1.0;      // Z scale
  ScaleType type = ScaleType::Uniform;
  bool aboutCentroid = true; // Scale about centroid or origin
};

/**
 * @class ScaleFeature
 * @brief Scales a solid body by a factor
 */
class ScaleFeature {
public:
  ScaleFeature() = default;

  /**
   * @brief Scale shape uniformly about centroid
   * @param shape Input shape
   * @param factor Scale factor (>1 = enlarge, <1 = shrink)
   * @return Scaled shape
   */
  TopoDS_Shape execute(const TopoDS_Shape &shape, double factor);

  /**
   * @brief Scale shape about a specific point
   * @param shape Input shape
   * @param factor Scale factor
   * @param point Scale center point
   * @return Scaled shape
   */
  TopoDS_Shape execute(const TopoDS_Shape &shape, double factor,
                       const gp_Pnt &point);

  /**
   * @brief Scale with full parameters
   * @param shape Input shape
   * @param params Scale parameters
   * @return Scaled shape
   */
  TopoDS_Shape execute(const TopoDS_Shape &shape, const ScaleParams &params);

  /**
   * @brief Non-uniform scale
   * @param shape Input shape
   * @param scaleX X scale factor
   * @param scaleY Y scale factor
   * @param scaleZ Z scale factor
   * @return Scaled shape
   */
  TopoDS_Shape executeNonUniform(const TopoDS_Shape &shape, double scaleX,
                                 double scaleY, double scaleZ);

  /**
   * @brief Get last error message
   */
  const std::string &errorMessage() const { return m_error; }

private:
  std::string m_error;

  /**
   * @brief Calculate centroid of shape
   */
  gp_Pnt calculateCentroid(const TopoDS_Shape &shape) const;
};

} // namespace part
} // namespace opencad
