#pragma once
/**
 * @file WireBuilder.h
 * @brief Builds TopoDS_Wire from sketch entities
 *
 * OpenCAD - Modular CAD/CAE Platform
 */

#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <memory>
#include <vector>

namespace opencad {
namespace sketch {
class Sketch;
class SketchEntity;
} // namespace sketch
namespace part {

/**
 * @class WireBuilder
 * @brief Converts 2D sketch entities to 3D wire/face
 */
class WireBuilder {
public:
  /// Build wire from entire sketch (closed profile)
  static TopoDS_Wire buildWire(const sketch::Sketch &sketch);

  /// Build wire from specific entities
  static TopoDS_Wire
  buildWire(const std::vector<const sketch::SketchEntity *> &entities);

  /// Build a face from closed wire
  static TopoDS_Face buildFace(const TopoDS_Wire &wire);

  /// Build face from sketch (convenience)
  static TopoDS_Face buildFace(const sketch::Sketch &sketch);

  /// Check if wire is closed
  static bool isClosed(const TopoDS_Wire &wire);

  /// Build faces from multiple wires (supports holes)
  static TopoDS_Shape buildFaces(const std::vector<TopoDS_Wire> &wires);
};

} // namespace part
} // namespace opencad
