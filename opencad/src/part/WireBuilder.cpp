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
#include <GC_MakeArcOfCircle.hxx>
#include <GC_MakeCircle.hxx>
#include <GC_MakeSegment.hxx>
#include <GProp_GProps.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <ShapeAnalysis_Wire.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>

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

TopoDS_Shape WireBuilder::buildFaces(const std::vector<TopoDS_Wire> &wires, const gp_Pln* plane) {
  if (wires.empty())
    return TopoDS_Shape();

  // Build a face for each wire independently (no hole detection)
  TopoDS_Compound resultCompound;
  BRep_Builder builder;
  builder.MakeCompound(resultCompound);
  bool addedAny = false;

  for (const auto &wire : wires) {
    if (wire.IsNull())
      continue;

    try {
      TopoDS_Face face;
      if (plane) {
        BRepBuilderAPI_MakeFace faceMaker(*plane, wire, true);
        if (faceMaker.IsDone()) {
          face = faceMaker.Face();
        }
      } else {
        BRepBuilderAPI_MakeFace faceMaker(wire, true);
        if (faceMaker.IsDone()) {
          face = faceMaker.Face();
        }
      }

      if (!face.IsNull()) {
        builder.Add(resultCompound, face);
        addedAny = true;
      }
    } catch (...) {
      continue;
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
