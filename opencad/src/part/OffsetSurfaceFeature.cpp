/**
 * @file OffsetSurfaceFeature.cpp
 * @brief Implementation of offset surface feature
 *
 * OpenCAD - Modular CAD/CAE Platform
 */

#include "OffsetSurfaceFeature.h"

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeShell.hxx>
#include <BRepOffsetAPI_MakeOffsetShape.hxx>
#include <BRepOffset_MakeOffset.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <Geom_OffsetSurface.hxx>
#include <Geom_Surface.hxx>
#include <ShapeAnalysis_Surface.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>

namespace opencad {
namespace part {

TopoDS_Shape OffsetSurfaceFeature::execute(const TopoDS_Face &face,
                                           double distance) {
  m_error.clear();

  try {
    // Get the surface from the face
    Handle(Geom_Surface) surface = BRep_Tool::Surface(face);
    if (surface.IsNull()) {
      m_error = "Failed to get surface from face";
      return TopoDS_Shape();
    }

    // Create offset surface using Geom_OffsetSurface
    Handle(Geom_OffsetSurface) offsetSurface =
        new Geom_OffsetSurface(surface, distance);
    if (offsetSurface.IsNull()) {
      m_error = "Failed to create offset surface";
      return TopoDS_Shape();
    }

    // Get the bounds of the original face
    Standard_Real uMin, uMax, vMin, vMax;
    BRepTools::UVBounds(face, uMin, uMax, vMin, vMax);

    // Create face from offset surface
    BRepBuilderAPI_MakeFace makeFace(offsetSurface, uMin, uMax, vMin, vMax,
                                     0.01);
    if (!makeFace.IsDone()) {
      m_error = "Failed to create face from offset surface";
      return TopoDS_Shape();
    }

    return makeFace.Face();
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  }
}

TopoDS_Shape
OffsetSurfaceFeature::execute(const std::vector<TopoDS_Face> &faces,
                              double distance) {
  m_error.clear();

  if (faces.empty()) {
    m_error = "No faces provided";
    return TopoDS_Shape();
  }

  try {
    TopoDS_Compound compound;
    BRep_Builder builder;
    builder.MakeCompound(compound);

    for (const auto &face : faces) {
      TopoDS_Shape offsetFace = execute(face, distance);
      if (!offsetFace.IsNull()) {
        builder.Add(compound, offsetFace);
      }
    }

    return compound;
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  }
}

TopoDS_Shape OffsetSurfaceFeature::executeAll(const TopoDS_Shape &shape,
                                              double distance) {
  m_error.clear();

  try {
    // Use BRepOffsetAPI_MakeOffsetShape for complete shape offset
    BRepOffsetAPI_MakeOffsetShape offsetMaker;
    offsetMaker.PerformBySimple(shape, distance);

    if (!offsetMaker.IsDone()) {
      // Fallback: offset individual faces
      std::vector<TopoDS_Face> faces;
      TopExp_Explorer explorer(shape, TopAbs_FACE);
      while (explorer.More()) {
        faces.push_back(TopoDS::Face(explorer.Current()));
        explorer.Next();
      }
      return execute(faces, distance);
    }

    return offsetMaker.Shape();
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  }
}

TopoDS_Shape OffsetSurfaceFeature::execute(const TopoDS_Shape &shape,
                                           const OffsetSurfaceParams &params) {
  m_error.clear();

  try {
    TopoDS_Shape result = executeAll(shape, params.distance);

    if (params.keepOriginal && !result.IsNull()) {
      // Combine original and offset
      TopoDS_Compound compound;
      BRep_Builder builder;
      builder.MakeCompound(compound);
      builder.Add(compound, shape);
      builder.Add(compound, result);
      return compound;
    }

    return result;
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  }
}

} // namespace part
} // namespace opencad
