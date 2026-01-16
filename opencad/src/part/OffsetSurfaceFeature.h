/**
 * @file OffsetSurfaceFeature.h
 * @brief Offset Surface feature - creates offset surfaces
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
 * @struct OffsetSurfaceParams
 * @brief Parameters for offset surface operation
 */
struct OffsetSurfaceParams {
  double distance = 1.0;     // Offset distance (positive = outward)
  bool keepOriginal = false; // Keep original surface
  double tolerance = 0.01;   // Geometric tolerance
};

/**
 * @class OffsetSurfaceFeature
 * @brief Creates surfaces offset from existing faces
 *
 * Useful for creating mold cavities, clearances, and shell-like geometries.
 */
class OffsetSurfaceFeature {
public:
  OffsetSurfaceFeature() = default;

  /**
   * @brief Offset a single face
   * @param face Face to offset
   * @param distance Offset distance (positive = outward, negative = inward)
   * @return Offset surface shape
   */
  TopoDS_Shape execute(const TopoDS_Face &face, double distance);

  /**
   * @brief Offset multiple faces
   * @param faces Faces to offset
   * @param distance Offset distance
   * @return Compound of offset surfaces
   */
  TopoDS_Shape execute(const std::vector<TopoDS_Face> &faces, double distance);

  /**
   * @brief Offset all faces of a shape
   * @param shape Input shape
   * @param distance Offset distance
   * @return Offset shell/solid
   */
  TopoDS_Shape executeAll(const TopoDS_Shape &shape, double distance);

  /**
   * @brief Offset with full parameters
   * @param shape Input shape
   * @param params Offset parameters
   * @return Offset shape
   */
  TopoDS_Shape execute(const TopoDS_Shape &shape,
                       const OffsetSurfaceParams &params);

  /**
   * @brief Get last error message
   */
  const std::string &errorMessage() const { return m_error; }

private:
  std::string m_error;
};

} // namespace part
} // namespace opencad
