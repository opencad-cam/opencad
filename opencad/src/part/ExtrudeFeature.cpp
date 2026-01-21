/**
 * @file ExtrudeFeature.cpp
 * @brief Extrude sketch profile to create solid
 */

#include "ExtrudeFeature.h"
#include "WireBuilder.h"
#include "sketch/Sketch.h"
#include "sketch/SketchPlane.h"

#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepOffsetAPI_DraftAngle.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <gp_Dir.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace opencad {
namespace part {

TopoDS_Shape ExtrudeFeature::execute(const sketch::Sketch &sketch, double depth,
                                     bool symmetric) {
  ExtrudeParams params;
  params.depth = depth;
  params.symmetric = symmetric;
  return execute(sketch, params);
}

TopoDS_Shape ExtrudeFeature::execute(const sketch::Sketch &sketch,
                                     const ExtrudeParams &params) {
  m_error.clear();

  // Get all closed profiles from sketch (includes slots, splines, etc.)
  auto profiles = sketch.detectClosedProfiles();
  if (profiles.empty()) {
    m_error = "No closed profiles found. Ensure sketch is closed.";
    return TopoDS_Shape();
  }

  // Build topology (faces with holes) from profiles
  TopoDS_Shape profileShape = WireBuilder::buildFaces(profiles);
  if (profileShape.IsNull()) {
    m_error = "Failed to build valid face architecture from profiles.";
    return TopoDS_Shape();
  }

  // Extrusion direction based on sketch plane normal
  gp_Dir normal = sketch.plane().normal();
  if (params.reversed) {
    normal.Reverse();
  }

  double depth = params.depth;
  gp_Vec extrudeVec(normal.X() * depth, normal.Y() * depth, normal.Z() * depth);

  TopoDS_Shape result;

  try {
    if (params.symmetric) {
      // Mid-plane: extrude half depth in each direction
      gp_Vec halfVec = extrudeVec;
      halfVec.Scale(0.5);

      // Move profile back by half
      gp_Trsf moveBack;
      gp_Vec backVec = halfVec.Reversed();
      moveBack.SetTranslation(backVec);
      BRepBuilderAPI_Transform transform(profileShape, moveBack, true);
      TopoDS_Shape movedProfile = transform.Shape();

      BRepPrimAPI_MakePrism prism(movedProfile, extrudeVec);
      prism.Build();
      if (prism.IsDone()) {
        result = prism.Shape();
      }
    } else {
      BRepPrimAPI_MakePrism prism(profileShape, extrudeVec);
      prism.Build();
      if (prism.IsDone()) {
        result = prism.Shape();
      }
    }

    // Apply draft angle if specified
    if (!result.IsNull() && std::abs(params.draftAngle) > 0.001) {
      // Use sketch plane as neutral plane and extrusion direction as draft direction
      result = applyDraft(result, sketch.plane().plane(), normal, params.draftAngle);
    }

    if (result.IsNull()) {
      m_error = "Extrusion failed. Check sketch profile.";
    }

    return result;
  } catch (...) {
    m_error = "Exception during extrusion";
    return TopoDS_Shape();
  }
}

TopoDS_Shape ExtrudeFeature::executeWithDraft(const TopoDS_Wire &profile,
                                              double depth,
                                              double draftAngleDeg) {
  m_error.clear();

  try {
    // Create face from wire
    BRepBuilderAPI_MakeFace faceMaker(profile, true);
    if (!faceMaker.IsDone()) {
      m_error = "Failed to create face from wire";
      return TopoDS_Shape();
    }

    return executeWithDraft(faceMaker.Face(), depth, draftAngleDeg);
  } catch (...) {
    m_error = "Exception in executeWithDraft";
    return TopoDS_Shape();
  }
}

TopoDS_Shape ExtrudeFeature::executeWithDraft(const TopoDS_Face &profile,
                                              double depth,
                                              double draftAngleDeg) {
  m_error.clear();

  try {
    // Default extrusion direction Z+
    gp_Vec extrudeVec(0, 0, depth);

    BRepPrimAPI_MakePrism prism(profile, extrudeVec);
    prism.Build();

    if (!prism.IsDone()) {
      m_error = "Extrusion failed";
      return TopoDS_Shape();
    }

    TopoDS_Shape result = prism.Shape();

    // Apply draft if needed
    if (std::abs(draftAngleDeg) > 0.001) {
      // Default to Z draft for manual extrusion (assuming profile on XY)
      gp_Pln neutralPlane(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
      gp_Dir draftDir(0, 0, 1);
      result = applyDraft(result, neutralPlane, draftDir, draftAngleDeg);
    }

    return result;
  } catch (...) {
    m_error = "Exception in executeWithDraft";
    return TopoDS_Shape();
  }
}

TopoDS_Shape ExtrudeFeature::applyDraft(const TopoDS_Shape &shape,
                                        const gp_Pln &neutralPlane,
                                        const gp_Dir &pullDir,
                                        double angleDeg) {
  // Convert angle to radians
  double angleRad = angleDeg * M_PI / 180.0;

  try {
    BRepOffsetAPI_DraftAngle draftOp(shape);

    // Apply draft to all lateral faces
    TopExp_Explorer explorer(shape, TopAbs_FACE);
    while (explorer.More()) {
      TopoDS_Face face = TopoDS::Face(explorer.Current());

      // Try to add draft to this face
      try {
        draftOp.Add(face, pullDir, angleRad, neutralPlane);
      } catch (...) {
        // Some faces may not be draftable, skip them
      }

      explorer.Next();
    }

    draftOp.Build();
    if (draftOp.IsDone()) {
      return draftOp.Shape();
    }
  } catch (...) {
    // Draft failed, return original shape
  }

  return shape;
}

TopoDS_Shape ExtrudeFeature::addTo(const sketch::Sketch &sketch,
                                   const TopoDS_Shape &base, double depth,
                                   bool symmetric) {
  ExtrudeParams params;
  params.depth = depth;
  params.symmetric = symmetric;
  return addTo(sketch, base, params);
}

TopoDS_Shape ExtrudeFeature::addTo(const sketch::Sketch &sketch,
                                   const TopoDS_Shape &base,
                                   const ExtrudeParams &params) {
  m_error.clear();

  if (base.IsNull()) {
    // No base shape, just extrude
    return execute(sketch, params);
  }

  // Extrude the sketch
  TopoDS_Shape extruded = execute(sketch, params);
  if (extruded.IsNull()) {
    return TopoDS_Shape();
  }

  // Fuse with base
  try {
    BRepAlgoAPI_Fuse fuse(base, extruded);
    fuse.Build();

    if (fuse.IsDone()) {
      return fuse.Shape();
    } else {
      m_error = "Fuse operation failed";
      return base; // Return original
    }
  } catch (...) {
    m_error = "Exception during fuse";
    return base;
  }
}

} // namespace part
} // namespace opencad
