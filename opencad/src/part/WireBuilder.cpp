/**
 * @file WireBuilder.cpp
 * @brief Builds TopoDS_Wire from sketch entities
 */

#include "WireBuilder.h"
#include "sketch/Sketch.h"
#include "sketch/SketchPlane.h"
#include "sketch/entities/SketchArc.h"
#include "sketch/entities/SketchCircle.h"
#include "sketch/entities/SketchEntity.h"
#include "sketch/entities/SketchLine.h"
#include "sketch/entities/SketchRectangle.h"

#include <BRepAlgo.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepClass_FaceClassifier.hxx>
#include <BRepGProp.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <GC_MakeCircle.hxx>
#include <GC_MakeSegment.hxx>
#include <GProp_GProps.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <ShapeAnalysis_Wire.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Vertex.hxx>
#include <algorithm>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace opencad {
namespace part {

TopoDS_Wire WireBuilder::buildWire(const sketch::Sketch &sketch) {
  BRepBuilderAPI_MakeWire wireBuilder;
  const sketch::SketchPlane &plane = sketch.plane();

  for (const auto &entity : sketch.entities()) {
    if (!entity)
      continue;

    switch (entity->type()) {
    case sketch::EntityType::Line: {
      auto *line = static_cast<sketch::SketchLine *>(entity.get());
      // Use plane.to3D() for correct transformation
      gp_Pnt p1 = plane.to3D(line->startPoint());
      gp_Pnt p2 = plane.to3D(line->endPoint());

      if (p1.Distance(p2) > 1e-6) {
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(p1, p2);
        wireBuilder.Add(edge);
      }
      break;
    }

    case sketch::EntityType::Circle: {
      auto *circle = static_cast<sketch::SketchCircle *>(entity.get());
      gp_Pnt center = plane.to3D(circle->center());
      gp_Dir normal = plane.normal();
      gp_Ax2 axis(center, normal);
      gp_Circ gCircle(axis, circle->radius());

      TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(gCircle);
      wireBuilder.Add(edge);
      break;
    }

    case sketch::EntityType::Arc: {
      auto *arc = static_cast<sketch::SketchArc *>(entity.get());
      gp_Pnt center = plane.to3D(arc->center());
      gp_Dir normal = plane.normal();
      gp_Ax2 axis(center, normal);
      gp_Circ gCircle(axis, arc->radius());

      // Start and end points from angles
      double r = arc->radius();
      double sa = arc->startAngle();
      double ea = arc->endAngle();

      gp_Pnt2d start2d(arc->center().X() + r * cos(sa),
                       arc->center().Y() + r * sin(sa));
      gp_Pnt2d end2d(arc->center().X() + r * cos(ea),
                     arc->center().Y() + r * sin(ea));
      gp_Pnt startPt = plane.to3D(start2d);
      gp_Pnt endPt = plane.to3D(end2d);

      GC_MakeArcOfCircle arcMaker(gCircle, startPt, endPt, true);
      if (arcMaker.IsDone()) {
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(arcMaker.Value());
        wireBuilder.Add(edge);
      }
      break;
    }

    case sketch::EntityType::Rectangle: {
      auto *rect = static_cast<sketch::SketchRectangle *>(entity.get());
      gp_Pnt c1 = plane.to3D(rect->corner1());
      gp_Pnt c2 =
          plane.to3D(gp_Pnt2d(rect->corner2().X(), rect->corner1().Y()));
      gp_Pnt c3 = plane.to3D(rect->corner2());
      gp_Pnt c4 =
          plane.to3D(gp_Pnt2d(rect->corner1().X(), rect->corner2().Y()));

      wireBuilder.Add(BRepBuilderAPI_MakeEdge(c1, c2));
      wireBuilder.Add(BRepBuilderAPI_MakeEdge(c2, c3));
      wireBuilder.Add(BRepBuilderAPI_MakeEdge(c3, c4));
      wireBuilder.Add(BRepBuilderAPI_MakeEdge(c4, c1));
      break;
    }

    default:
      // Skip unsupported entities
      break;
    }
  }

  if (wireBuilder.IsDone()) {
    return wireBuilder.Wire();
  }

  return TopoDS_Wire();
}

// Legacy overload - kept for compatibility (uses XY plane)
TopoDS_Wire WireBuilder::buildWire(
    const std::vector<const sketch::SketchEntity *> &entities) {
  BRepBuilderAPI_MakeWire wireBuilder;

  for (const auto *entity : entities) {
    if (!entity)
      continue;

    switch (entity->type()) {
    case sketch::EntityType::Line: {
      auto *line = static_cast<const sketch::SketchLine *>(entity);
      gp_Pnt p1(line->startPoint().X(), line->startPoint().Y(), 0.0);
      gp_Pnt p2(line->endPoint().X(), line->endPoint().Y(), 0.0);

      if (p1.Distance(p2) > 1e-6) {
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(p1, p2);
        wireBuilder.Add(edge);
      }
      break;
    }

    case sketch::EntityType::Circle: {
      auto *circle = static_cast<const sketch::SketchCircle *>(entity);
      gp_Pnt center(circle->center().X(), circle->center().Y(), 0.0);
      gp_Ax2 axis(center, gp_Dir(0, 0, 1));
      gp_Circ gCircle(axis, circle->radius());

      TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(gCircle);
      wireBuilder.Add(edge);
      break;
    }

    case sketch::EntityType::Arc: {
      auto *arc = static_cast<const sketch::SketchArc *>(entity);
      gp_Pnt center(arc->center().X(), arc->center().Y(), 0.0);
      gp_Ax2 axis(center, gp_Dir(0, 0, 1));
      gp_Circ gCircle(axis, arc->radius());

      double r = arc->radius();
      double sa = arc->startAngle();
      double ea = arc->endAngle();

      gp_Pnt startPt(center.X() + r * cos(sa), center.Y() + r * sin(sa), 0);
      gp_Pnt endPt(center.X() + r * cos(ea), center.Y() + r * sin(ea), 0);

      GC_MakeArcOfCircle arcMaker(gCircle, startPt, endPt, true);
      if (arcMaker.IsDone()) {
        TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(arcMaker.Value());
        wireBuilder.Add(edge);
      }
      break;
    }

    case sketch::EntityType::Rectangle: {
      auto *rect = static_cast<const sketch::SketchRectangle *>(entity);
      gp_Pnt c1(rect->corner1().X(), rect->corner1().Y(), 0.0);
      gp_Pnt c2(rect->corner2().X(), rect->corner1().Y(), 0.0);
      gp_Pnt c3(rect->corner2().X(), rect->corner2().Y(), 0.0);
      gp_Pnt c4(rect->corner1().X(), rect->corner2().Y(), 0.0);

      wireBuilder.Add(BRepBuilderAPI_MakeEdge(c1, c2));
      wireBuilder.Add(BRepBuilderAPI_MakeEdge(c2, c3));
      wireBuilder.Add(BRepBuilderAPI_MakeEdge(c3, c4));
      wireBuilder.Add(BRepBuilderAPI_MakeEdge(c4, c1));
      break;
    }

    default:
      break;
    }
  }

  if (wireBuilder.IsDone()) {
    return wireBuilder.Wire();
  }

  return TopoDS_Wire();
}

TopoDS_Face WireBuilder::buildFace(const TopoDS_Wire &wire) {
  if (wire.IsNull()) {
    return TopoDS_Face();
  }

  BRepBuilderAPI_MakeFace faceMaker(wire, true);
  if (faceMaker.IsDone()) {
    return faceMaker.Face();
  }

  return TopoDS_Face();
}

TopoDS_Face WireBuilder::buildFace(const sketch::Sketch &sketch) {
  TopoDS_Wire wire = buildWire(sketch);
  return buildFace(wire);
}

bool WireBuilder::isClosed(const TopoDS_Wire &wire) {
  if (wire.IsNull())
    return false;
  return BRepAlgo::IsValid(wire) && wire.Closed();
}

TopoDS_Shape WireBuilder::buildFaces(const std::vector<TopoDS_Wire> &wires) {
  if (wires.empty())
    return TopoDS_Shape();

  // 1. Create planar face for each wire to check areas and containment
  struct WireInfo {
    TopoDS_Wire wire;
    TopoDS_Face tempFace;
    double area;
    bool used;
  };

  std::vector<WireInfo> wireInfos;
  for (const auto &wire : wires) {
    if (wire.IsNull())
      continue;

    BRepBuilderAPI_MakeFace mkFace(wire, true);
    // true = only planar
    if (mkFace.IsDone()) {
      TopoDS_Face face = mkFace.Face();
      GProp_GProps props;
      BRepGProp::SurfaceProperties(face, props);
      wireInfos.push_back({wire, face, props.Mass(), false});
    }
  }

  // 2. Sort by Area Descending (Largest first)
  std::sort(
      wireInfos.begin(), wireInfos.end(),
      [](const WireInfo &a, const WireInfo &b) { return a.area > b.area; });

  TopoDS_Compound resultCompound;
  BRep_Builder builder;
  builder.MakeCompound(resultCompound);
  bool addedAny = false;

  // 3. Process wires: Largest are Outers, smaller inside them are Holes
  for (size_t i = 0; i < wireInfos.size(); ++i) {
    if (wireInfos[i].used)
      continue;

    // Current largest is Outer
    WireInfo &outer = wireInfos[i];
    outer.used = true;

    BRepBuilderAPI_MakeFace faceBuilder(outer.wire, true);

    // Check remaining wires to see if they are holes inside this outer
    for (size_t j = i + 1; j < wireInfos.size(); ++j) {
      if (wireInfos[j].used)
        continue;

      WireInfo &potentialHole = wireInfos[j];

      // Check if hole is inside outer
      // Use a point from hole's wire/face
      // Get center or a vertex? BRepClass_FaceClassifier checks a point.
      // Let's use a point on the wire of the hole.

      // Or easier: Vertex of hole
      TopExp_Explorer exp(potentialHole.wire, TopAbs_VERTEX);
      if (exp.More()) {
        TopoDS_Vertex v = TopoDS::Vertex(exp.Current());
        gp_Pnt p = BRep_Tool::Pnt(v);

        BRepClass_FaceClassifier classifier(outer.tempFace, p, 1e-6);
        if (classifier.State() == TopAbs_IN) {
          // It is a hole!
          faceBuilder.Add(potentialHole.wire);
          potentialHole.used = true;
        }
      }
    }

    if (faceBuilder.IsDone()) {
      builder.Add(resultCompound, faceBuilder.Face());
      addedAny = true;
    }
  }

  if (!addedAny)
    return TopoDS_Shape();

  // If only one face in compound, return the face directly
  TopExp_Explorer exp(resultCompound, TopAbs_FACE);
  if (exp.More()) {
    TopoDS_Shape firstFace = exp.Current();
    exp.Next();
    if (!exp.More()) {
      return firstFace;
    }
  }

  return resultCompound;
}

} // namespace part
} // namespace opencad
