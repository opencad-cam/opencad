/**
 * @file RibFeature.cpp
 * @brief Implementation of rib feature
 *
 * OpenCAD - Modular CAD/CAE Platform
 */

#include "RibFeature.h"
#include "../sketch/Sketch.h"

#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <TopoDS.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>


#include <cmath>

namespace opencad {
namespace part {

TopoDS_Shape RibFeature::createRibSolid(const TopoDS_Wire &profile,
                                        const gp_Dir &direction,
                                        double thickness, bool symmetric) {
  try {
    // Offset the wire to create thickness
    // We create a face by offsetting the wire

    gp_Vec extrudeVec(direction);
    double halfThickness = symmetric ? thickness / 2.0 : thickness;

    // Create face from wire (planar wire)
    BRepBuilderAPI_MakeFace makeFace(profile, true);
    if (!makeFace.IsDone()) {
      return TopoDS_Shape();
    }
    TopoDS_Face face = makeFace.Face();

    // Extrude the face
    if (symmetric) {
      // Extrude in both directions
      gp_Vec vec1 = extrudeVec * halfThickness;
      gp_Vec vec2 = extrudeVec * (-halfThickness);

      BRepPrimAPI_MakePrism prism1(face, vec1);
      if (!prism1.IsDone())
        return TopoDS_Shape();

      BRepPrimAPI_MakePrism prism2(face, vec2);
      if (!prism2.IsDone())
        return TopoDS_Shape();

      BRepAlgoAPI_Fuse fuse(prism1.Shape(), prism2.Shape());
      if (!fuse.IsDone())
        return TopoDS_Shape();

      return fuse.Shape();
    } else {
      gp_Vec vec = extrudeVec * thickness;
      BRepPrimAPI_MakePrism prism(face, vec);
      if (!prism.IsDone())
        return TopoDS_Shape();
      return prism.Shape();
    }
  } catch (...) {
    return TopoDS_Shape();
  }
}

TopoDS_Shape RibFeature::trimRibToBase(const TopoDS_Shape &rib,
                                       const TopoDS_Shape &base) {
  try {
    // Find common volume between rib and base
    BRepAlgoAPI_Common common(rib, base);
    if (!common.IsDone()) {
      return rib; // Return untrimmed if trim fails
    }
    return common.Shape();
  } catch (...) {
    return rib;
  }
}

TopoDS_Shape RibFeature::execute(const sketch::Sketch &sketch,
                                 const TopoDS_Shape &base, double thickness,
                                 bool symmetric) {
  RibParams params;
  params.thickness = thickness;
  params.symmetric = symmetric;
  return execute(sketch, base, params);
}

TopoDS_Shape RibFeature::execute(const sketch::Sketch &sketch,
                                 const TopoDS_Shape &base,
                                 const RibParams &params) {
  m_error.clear();

  try {
    // Build wire from sketch (open profile preferred for ribs)
    TopoDS_Wire wire = sketch.buildWire();
    if (wire.IsNull()) {
      m_error = "Failed to build wire from sketch";
      return base;
    }

    // Get direction based on rib type
    const auto &plane = sketch.plane();
    gp_Dir direction;

    switch (params.type) {
    case RibType::Parallel:
      // Direction is in the plane (typically Y direction)
      direction = plane.yDirection();
      break;
    case RibType::Normal:
      // Direction is normal to sketch plane
      direction = plane.normal();
      break;
    case RibType::AtAngle: {
      // Rotate normal by angle
      double angleRad = params.angle * M_PI / 180.0;
      gp_Vec normal(plane.normal());
      gp_Vec yDir(plane.yDirection());

      // Rotate around X axis of sketch plane
      gp_Trsf rotation;
      rotation.SetRotation(gp_Ax1(plane.origin(), plane.xDirection()),
                           angleRad);

      normal.Transform(rotation);
      direction = gp_Dir(normal);
      break;
    }
    }

    if (params.flipDirection) {
      direction.Reverse();
    }

    return executeWire(wire, base, direction, params.thickness);
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return base;
  } catch (const std::exception &e) {
    m_error = "Error: ";
    m_error += e.what();
    return base;
  }
}

TopoDS_Shape RibFeature::executeWire(const TopoDS_Wire &profile,
                                     const TopoDS_Shape &base,
                                     const gp_Dir &direction,
                                     double thickness) {
  m_error.clear();

  try {
    // Create the rib solid
    TopoDS_Shape ribSolid = createRibSolid(profile, direction, thickness, true);
    if (ribSolid.IsNull()) {
      m_error = "Failed to create rib solid";
      return base;
    }

    // Fuse with base shape
    BRepAlgoAPI_Fuse fuse(base, ribSolid);
    if (!fuse.IsDone()) {
      m_error = "Failed to fuse rib with base";
      return base;
    }

    return fuse.Shape();
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return base;
  }
}

} // namespace part
} // namespace opencad
