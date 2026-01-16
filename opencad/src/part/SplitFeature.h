/**
 * @file SplitFeature.h
 * @brief Split feature - splits a solid using a plane or surface
 *
 * OpenCAD - Modular CAD/CAE Platform
 */
#pragma once

#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Pln.hxx>
#include <string>
#include <vector>

namespace opencad {
namespace part {

/**
 * @enum SplitKeepPart
 * @brief Which parts to keep after split
 */
enum class SplitKeepPart {
  Above, // Keep parts above the plane
  Below, // Keep parts below the plane
  Both   // Keep both parts (separate shapes)
};

/**
 * @struct SplitParams
 * @brief Parameters for split operation
 */
struct SplitParams {
  SplitKeepPart keepPart = SplitKeepPart::Both;
  bool trimToPlane = true; // Trim shape to exactly at plane
};

/**
 * @class SplitFeature
 * @brief Splits a solid body using a plane or another shape
 *
 * Similar to SolidWorks Split feature. Can be used for:
 * - Creating mold halves
 * - Dividing parts for manufacturing
 * - Separating geometry for analysis
 */
class SplitFeature {
public:
  SplitFeature() = default;

  /**
   * @brief Split shape with a plane
   * @param shape Shape to split
   * @param plane Splitting plane
   * @param keepPart Which part(s) to keep
   * @return Vector of resulting shapes (1 or 2 shapes)
   */
  std::vector<TopoDS_Shape>
  execute(const TopoDS_Shape &shape, const gp_Pln &plane,
          SplitKeepPart keepPart = SplitKeepPart::Both);

  /**
   * @brief Split shape with another shape (tool)
   * @param shape Shape to split
   * @param tool Splitting tool shape
   * @param keepPart Which part(s) to keep
   * @return Vector of resulting shapes
   */
  std::vector<TopoDS_Shape>
  executeWithTool(const TopoDS_Shape &shape, const TopoDS_Shape &tool,
                  SplitKeepPart keepPart = SplitKeepPart::Both);

  /**
   * @brief Split shape with a face
   * @param shape Shape to split
   * @param face Splitting face/surface
   * @return Vector of resulting shapes
   */
  std::vector<TopoDS_Shape> executeWithFace(const TopoDS_Shape &shape,
                                            const TopoDS_Face &face);

  /**
   * @brief Get last error message
   */
  const std::string &errorMessage() const { return m_error; }

private:
  std::string m_error;
};

} // namespace part
} // namespace opencad
