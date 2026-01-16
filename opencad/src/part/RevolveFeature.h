/**
 * @file RevolveFeature.h
 * @brief Revolve feature - revolve sketch profile around an axis
 *
 * OpenCAD - Modular CAD/CAE Platform
 */
#pragma once

#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Ax1.hxx>
#include <memory>
#include <string>


namespace opencad {
namespace sketch {
class Sketch;
}
namespace part {

/**
 * @struct RevolveParams
 * @brief Parameters for revolve operation
 */
struct RevolveParams {
  double angle = 360.0;   // Revolve angle in degrees (default full revolution)
  double angle2 = 0.0;    // Secondary angle for bidirectional revolve
  bool symmetric = false; // Symmetric revolve (angle/2 each direction)
  bool reversed = false;  // Reverse revolve direction
  bool thinFeature = false;   // Create thin wall feature
  double thinThickness = 1.0; // Thin wall thickness
  bool solidResult = true;    // Create solid (false = surface)
};

/**
 * @enum RevolveAxisType
 * @brief Type of axis for revolve operation
 */
enum class RevolveAxisType {
  XAxis,      // Global X axis
  YAxis,      // Global Y axis
  ZAxis,      // Global Z axis
  SketchLine, // Line from sketch (construction line)
  CustomAxis  // User-defined axis
};

/**
 * @class RevolveFeature
 * @brief Creates a solid by revolving a sketch profile around an axis
 *
 * Similar to SolidWorks Revolved Boss/Cut feature.
 */
class RevolveFeature {
public:
  RevolveFeature() = default;

  /**
   * @brief Revolve a sketch profile around the Y axis (default)
   * @param sketch Source sketch
   * @param angleDeg Revolve angle in degrees
   * @param symmetric If true, revolve angle/2 in each direction
   * @return Revolved solid shape
   */
  TopoDS_Shape execute(const sketch::Sketch &sketch, double angleDeg,
                       bool symmetric = false);

  /**
   * @brief Revolve with full parameters
   * @param sketch Source sketch
   * @param params Revolve parameters
   * @return Revolved solid shape
   */
  TopoDS_Shape execute(const sketch::Sketch &sketch,
                       const RevolveParams &params);

  /**
   * @brief Revolve around a custom axis
   * @param sketch Source sketch
   * @param axis Rotation axis
   * @param angleDeg Revolve angle in degrees
   * @return Revolved solid shape
   */
  TopoDS_Shape execute(const sketch::Sketch &sketch, const gp_Ax1 &axis,
                       double angleDeg);

  /**
   * @brief Revolve a wire profile around an axis
   * @param profile Profile wire to revolve
   * @param axis Rotation axis
   * @param angleDeg Revolve angle in degrees
   * @return Revolved shape
   */
  TopoDS_Shape executeWire(const TopoDS_Wire &profile, const gp_Ax1 &axis,
                           double angleDeg);

  /**
   * @brief Revolve a face profile around an axis
   * @param profile Profile face to revolve
   * @param axis Rotation axis
   * @param angleDeg Revolve angle in degrees
   * @return Revolved solid shape
   */
  TopoDS_Shape executeFace(const TopoDS_Face &profile, const gp_Ax1 &axis,
                           double angleDeg);

  /**
   * @brief Revolve and add to existing shape (Boss)
   * @param sketch Source sketch
   * @param base Base shape to add to
   * @param angleDeg Revolve angle in degrees
   * @return Combined shape
   */
  TopoDS_Shape addTo(const sketch::Sketch &sketch, const TopoDS_Shape &base,
                     double angleDeg);

  /**
   * @brief Revolve and cut from existing shape (Cut)
   * @param sketch Source sketch
   * @param base Base shape to cut from
   * @param angleDeg Revolve angle in degrees
   * @return Shape with cut applied
   */
  TopoDS_Shape cutFrom(const sketch::Sketch &sketch, const TopoDS_Shape &base,
                       double angleDeg);

  /**
   * @brief Set the axis type for revolve
   * @param axisType Type of axis
   */
  void setAxisType(RevolveAxisType axisType) { m_axisType = axisType; }

  /**
   * @brief Set custom axis
   * @param axis Custom rotation axis
   */
  void setCustomAxis(const gp_Ax1 &axis) { m_customAxis = axis; }

  /**
   * @brief Get last error message
   */
  const std::string &errorMessage() const { return m_error; }

private:
  RevolveAxisType m_axisType = RevolveAxisType::YAxis;
  gp_Ax1 m_customAxis;
  std::string m_error;

  /**
   * @brief Get axis from sketch plane
   */
  gp_Ax1 getAxisFromSketch(const sketch::Sketch &sketch) const;

  /**
   * @brief Convert angle to radians
   */
  double toRadians(double degrees) const;
};

} // namespace part
} // namespace opencad
