#pragma once
/**
 * @file ExtrudeFeature.h
 * @brief Extrude sketch profile to create solid
 *
 * OpenCAD - Modular CAD/CAE Platform
 */

#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <memory>


namespace opencad {
namespace sketch {
class Sketch;
}
namespace part {

/**
 * @struct ExtrudeParams
 * @brief Parameters for extrude operation
 */
struct ExtrudeParams {
  double depth = 10.0;      // Primary extrusion depth
  double depth2 = 0.0;      // Secondary depth (for mid-plane or asymmetric)
  double draftAngle = 0.0;  // Draft/taper angle in degrees (positive = outward)
  bool symmetric = false;   // Mid-plane extrude (half depth each direction)
  bool reversed = false;    // Reverse extrusion direction
  bool thinFeature = false; // Create thin wall feature
  double thinThickness = 1.0; // Thin wall thickness
};

/**
 * @class ExtrudeFeature
 * @brief Creates a solid by extruding a sketch profile
 */
class ExtrudeFeature {
public:
  ExtrudeFeature() = default;

  /// Extrude a sketch profile with basic parameters
  TopoDS_Shape execute(const sketch::Sketch &sketch, double depth,
                       bool symmetric = false);

  /// Extrude with full parameters
  TopoDS_Shape execute(const sketch::Sketch &sketch,
                       const ExtrudeParams &params);

  /// Extrude a wire profile with draft angle
  TopoDS_Shape executeWithDraft(const TopoDS_Wire &profile, double depth,
                                double draftAngleDeg);

  /// Extrude a face profile with draft angle
  TopoDS_Shape executeWithDraft(const TopoDS_Face &profile, double depth,
                                double draftAngleDeg);

  /// Extrude and add to existing shape (Boss/Pad)
  TopoDS_Shape addTo(const sketch::Sketch &sketch, const TopoDS_Shape &base,
                     double depth, bool symmetric = false);

  /// Extrude and add with full parameters
  TopoDS_Shape addTo(const sketch::Sketch &sketch, const TopoDS_Shape &base,
                     const ExtrudeParams &params);

  /// Get last error message
  const std::string &errorMessage() const { return m_error; }

private:
  std::string m_error;

  // Internal: apply draft to prism
  TopoDS_Shape applyDraft(const TopoDS_Shape &shape, double angleDeg);
};

} // namespace part
} // namespace opencad
