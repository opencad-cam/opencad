/**
 * @file DomeFeature.h
 * @brief Dome feature - adds a spherical dome to a face
 *
 * OpenCAD - Modular CAD/CAE Platform
 */
#pragma once

#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <string>

namespace opencad {
namespace part {

/**
 * @enum DomeType
 * @brief Type of dome geometry
 */
enum class DomeType {
  Spherical,  // Spherical cap
  Elliptical, // Elliptical dome
  Continuous  // Continuous tangent dome
};

/**
 * @struct DomeParams
 * @brief Parameters for dome operation
 */
struct DomeParams {
  double height = 10.0;       // Dome height
  double ellipticRatio = 1.0; // Ratio for elliptical dome (1.0 = spherical)
  DomeType type = DomeType::Spherical;
  bool reversed = false; // Dome direction (inward if true)
};

/**
 * @class DomeFeature
 * @brief Creates a dome (spherical cap) on a planar face
 *
 * Similar to SolidWorks Dome feature. Adds a curved surface
 * to a planar face, creating a spherical or elliptical cap.
 */
class DomeFeature {
public:
  DomeFeature() = default;

  /**
   * @brief Add spherical dome to a face
   * @param shape Base shape containing the face
   * @param face Planar face to dome
   * @param height Dome height
   * @return Shape with dome added
   */
  TopoDS_Shape execute(const TopoDS_Shape &shape, const TopoDS_Face &face,
                       double height);

  /**
   * @brief Add dome with full parameters
   * @param shape Base shape
   * @param face Face to dome
   * @param params Dome parameters
   * @return Shape with dome
   */
  TopoDS_Shape execute(const TopoDS_Shape &shape, const TopoDS_Face &face,
                       const DomeParams &params);

  /**
   * @brief Create standalone dome shape from a face
   * @param face Base face for dome
   * @param height Dome height
   * @return Dome solid shape
   */
  TopoDS_Shape createDome(const TopoDS_Face &face, double height);

  /**
   * @brief Get last error message
   */
  const std::string &errorMessage() const { return m_error; }

private:
  std::string m_error;

  /**
   * @brief Calculate dome center and radius from face and height
   */
  void calculateDomeGeometry(const TopoDS_Face &face, double height,
                             double &radius, double &centerZ) const;
};

} // namespace part
} // namespace opencad
