/**
 * @file Sketch.cpp
 * @brief Implementation of Sketch class
 */

#include "Sketch.h"
#include "constraints/CoincidentConstraint.h"
#include "constraints/DimensionConstraint.h"
#include "constraints/HorizontalConstraint.h"
#include "constraints/VerticalConstraint.h"
#include <algorithm>

#include "constraints/FixConstraint.h"
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeEdge2d.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepClass_FaceClassifier.hxx>
#include <BRepGProp.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepOffsetAPI_MakeThickSolid.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <ElSLib.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <GProp_GProps.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <Geom2d_Curve.hxx>
#include <GeomProjLib.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Line.hxx>
#include <Geom_Plane.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <QDebug>
#include <QtGlobal> // Fix for qDebug
#include <ShapeAnalysis_FreeBounds.hxx>
#include <Standard_Failure.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_HSequenceOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Vertex.hxx>
#include <algorithm>
#include <cmath>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Vec.hxx>
#include <gp_Vec2d.hxx>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
// Minimum distance tolerance for arc validation
constexpr double ARC_POINT_TOLERANCE = 1e-6;

/**
 * @brief Validate that three points can form a valid arc
 * @throws Standard_Failure if points are invalid
 */
void ValidateArcPoints(const gp_Pnt &pStart, const gp_Pnt &pMid,
                       const gp_Pnt &pEnd) {
  // Check if any two points are coincident
  if (pStart.Distance(pMid) < ARC_POINT_TOLERANCE) {
    throw Standard_Failure("Start and mid points are too close");
  }
  if (pMid.Distance(pEnd) < ARC_POINT_TOLERANCE) {
    throw Standard_Failure("Mid and end points are too close");
  }
  if (pStart.Distance(pEnd) < ARC_POINT_TOLERANCE) {
    throw Standard_Failure("Start and end points are too close");
  }

  // Check collinearity using cross product
  gp_Vec v1(pStart, pMid);
  gp_Vec v2(pStart, pEnd);
  gp_Vec cross = v1.Crossed(v2);

  if (cross.Magnitude() < ARC_POINT_TOLERANCE) {
    throw Standard_Failure("Points are collinear - cannot form arc");
  }
}

/**
 * @brief Create arc edge from 3 points using GC_MakeArcOfCircle
 *
 * GC_MakeArcOfCircle(pStart, pMid, pEnd) creates arc:
 *   - Starting at pStart
 *   - Passing through pMid (defines the curve direction/shape)
 *   - Ending at pEnd
 *
 * @param pStart Arc start point
 * @param pMid Arc middle point (point on arc)
 * @param pEnd Arc end point
 * @return TopoDS_Edge The created arc edge
 * @throws Standard_Failure if arc creation fails
 */
TopoDS_Edge CreateArc3Points(const gp_Pnt &pStart, const gp_Pnt &pMid,
                             const gp_Pnt &pEnd) {
  // 1️⃣ Geometri doğrulama
  ValidateArcPoints(pStart, pMid, pEnd);

  // 2️⃣ Arc oluştur
  GC_MakeArcOfCircle arcBuilder(pStart, pMid, pEnd);

  if (!arcBuilder.IsDone()) {
    throw Standard_Failure("GC_MakeArcOfCircle başarısız");
  }

  Handle(Geom_TrimmedCurve) arc = arcBuilder.Value();

  // 3️⃣ Edge oluştur
  BRepBuilderAPI_MakeEdge edgeBuilder(arc);

  if (!edgeBuilder.IsDone()) {
    throw Standard_Failure("Arc edge oluşturulamadı");
  }

  return edgeBuilder.Edge();
}
} // anonymous namespace

namespace opencad {
namespace sketch {

Sketch::Sketch()
    : m_name("Sketch1"), m_plane(), m_status(SketchStatus::Open),
      m_visible(true) {}

Sketch::Sketch(const SketchPlane &plane)
    : m_name("Sketch1"), m_plane(plane), m_status(SketchStatus::Open),
      m_visible(true) {}

void Sketch::addEntity(SketchEntity::Ptr entity) {
  m_entities.push_back(entity);
}

void Sketch::addConstraint(Constraint::Ptr constraint) {
  m_constraints.push_back(constraint);
}

// === Entity Creation ===

SketchPoint::Ptr Sketch::addPoint(double x, double y) {
  auto point = std::make_shared<SketchPoint>(x, y);
  addEntity(point);
  return point;
}

SketchLine::Ptr Sketch::addLine(double x1, double y1, double x2, double y2) {
  auto line = std::make_shared<SketchLine>(x1, y1, x2, y2);
  addEntity(line);
  return line;
}

SketchLine::Ptr Sketch::addLine(const gp_Pnt2d &start, const gp_Pnt2d &end) {
  auto line = std::make_shared<SketchLine>(start, end);
  addEntity(line);
  return line;
}

SketchArc::Ptr Sketch::addArc(const gp_Pnt2d &center, double radius,
                              double startAngle, double endAngle) {
  auto arc = std::make_shared<SketchArc>(center, radius, startAngle, endAngle);
  addEntity(arc);
  return arc;
}

SketchArc::Ptr Sketch::addThreePointArc(const gp_Pnt2d &start,
                                        const gp_Pnt2d &end,
                                        const gp_Pnt2d &mid) {
  auto arc = std::make_shared<SketchArc>(start, end, mid);
  addEntity(arc);
  return arc;
}

SketchCircle::Ptr Sketch::addCircle(double cx, double cy, double radius) {
  auto circle = std::make_shared<SketchCircle>(cx, cy, radius);
  addEntity(circle);
  return circle;
}

SketchCircle::Ptr Sketch::addCircle(const gp_Pnt2d &center, double radius) {
  auto circle = std::make_shared<SketchCircle>(center, radius);
  addEntity(circle);
  return circle;
}

SketchRectangle::Ptr Sketch::addRectangle(double x, double y, double width,
                                          double height) {
  auto rect = std::make_shared<SketchRectangle>(x, y, width, height);
  addEntity(rect);
  return rect;
}

SketchSpline::Ptr Sketch::addSpline(const std::vector<gp_Pnt2d> &points,
                                    bool closed) {
  auto spline = std::make_shared<SketchSpline>(points);
  spline->setClosed(closed);
  addEntity(spline);
  return spline;
}

SketchEllipse::Ptr Sketch::addEllipse(const gp_Pnt2d &center, double majorR,
                                      double minorR, double angle) {
  auto ellipse = std::make_shared<SketchEllipse>(center, majorR, minorR, angle);
  addEntity(ellipse);
  return ellipse;
}

// === Entity Removal ===

void Sketch::removeEntity(uint64_t entityId) {
  m_entities.erase(std::remove_if(m_entities.begin(), m_entities.end(),
                                  [entityId](const SketchEntity::Ptr &e) {
                                    return e->id() == entityId;
                                  }),
                   m_entities.end());
}

void Sketch::removeEntity(SketchEntity::Ptr entity) {
  if (entity) {
    removeEntity(entity->id());
  }
}

SketchEntity::Ptr Sketch::getEntity(uint64_t id) const {
  auto it =
      std::find_if(m_entities.begin(), m_entities.end(),
                   [id](const SketchEntity::Ptr &e) { return e->id() == id; });
  return (it != m_entities.end()) ? *it : nullptr;
}

std::vector<SketchEntity::Ptr> Sketch::entitiesOfType(EntityType type) const {
  std::vector<SketchEntity::Ptr> result;
  for (const auto &entity : m_entities) {
    if (entity->type() == type) {
      result.push_back(entity);
    }
  }
  return result;
}

// === Constraint Addition ===

void Sketch::addHorizontal(SketchLine::Ptr line) {
  auto constraint = std::make_shared<HorizontalConstraint>(line);
  addConstraint(constraint);
}

void Sketch::addVertical(SketchLine::Ptr line) {
  auto constraint = std::make_shared<VerticalConstraint>(line);
  addConstraint(constraint);
}

void Sketch::addCoincident(SketchPoint::Ptr p1, SketchPoint::Ptr p2) {
  auto constraint = std::make_shared<CoincidentConstraint>(p1, p2);
  addConstraint(constraint);
}

void Sketch::addCoincident(SketchLine::Ptr line1, int endpoint1,
                           SketchLine::Ptr line2, int endpoint2) {
  auto constraint = std::make_shared<CoincidentConstraint>(line1, endpoint1,
                                                           line2, endpoint2);
  addConstraint(constraint);
}

void Sketch::addDistance(SketchPoint::Ptr p1, SketchPoint::Ptr p2,
                         double distance) {
  auto constraint = DimensionConstraint::createPointToPoint(p1, p2, distance);
  addConstraint(constraint);
}

void Sketch::addLength(SketchLine::Ptr line, double length) {
  auto constraint = DimensionConstraint::createLineLength(line, length);
  addConstraint(constraint);
}

void Sketch::addRadius(SketchCircle::Ptr circle, double radius) {
  auto constraint = DimensionConstraint::createRadius(circle, radius);
  addConstraint(constraint);
}

void Sketch::addRadius(SketchArc::Ptr arc, double radius) {
  auto constraint = DimensionConstraint::createRadius(arc, radius);
  addConstraint(constraint);
}

void Sketch::addAngle(SketchLine::Ptr line1, SketchLine::Ptr line2,
                      double angleDegrees) {
  auto constraint =
      DimensionConstraint::createAngle(line1, line2, angleDegrees);
  addConstraint(constraint);
}

// === Constraint Removal ===

void Sketch::removeConstraint(uint64_t constraintId) {
  m_constraints.erase(std::remove_if(m_constraints.begin(), m_constraints.end(),
                                     [constraintId](const Constraint::Ptr &c) {
                                       return c->id() == constraintId;
                                     }),
                      m_constraints.end());
}

void Sketch::removeConstraint(Constraint::Ptr constraint) {
  if (constraint) {
    removeConstraint(constraint->id());
  }
}

Constraint::Ptr Sketch::getConstraint(uint64_t id) const {
  auto it =
      std::find_if(m_constraints.begin(), m_constraints.end(),
                   [id](const Constraint::Ptr &c) { return c->id() == id; });
  return (it != m_constraints.end()) ? *it : nullptr;
}

// === Associative Constraint Binding ===

void Sketch::updateBoundConstraints(
    const std::map<std::string, double> &parameterValues) {
  for (auto &constraint : m_constraints) {
    if (!constraint->hasDimension())
      continue;

    // Check if bound to a parameter directly
    if (constraint->isBoundToParameter()) {
      const std::string &paramName = constraint->boundParameterName();
      auto it = parameterValues.find(paramName);
      if (it != parameterValues.end()) {
        constraint->setDimension(it->second);
      }
    }
    // TODO: Expression evaluation could be added here
    // e.g., "width * 2" would be parsed and evaluated
  }

  // Re-solve the sketch with updated constraint values
  solve();
}

std::vector<Constraint::Ptr> Sketch::getBoundConstraints() const {
  std::vector<Constraint::Ptr> result;
  for (const auto &constraint : m_constraints) {
    if (constraint->isBoundToParameter() || constraint->hasExpression()) {
      result.push_back(constraint);
    }
  }
  return result;
}

// === Solver ===

SolverStatus Sketch::solve() {
  return m_solver.solve(m_entities, m_constraints);
}

int Sketch::remainingDOF() const {
  return m_solver.remainingDOF(m_entities, m_constraints);
}

bool Sketch::isFullyConstrained() const {
  return m_solver.isFullyConstrained(m_entities, m_constraints);
}

// === Wire/Face Generation ===

TopoDS_Wire Sketch::buildWire() const {
  BRepBuilderAPI_MakeWire wireBuilder;

  for (const auto &entity : m_entities) {
    if (entity->isConstruction())
      continue;

    // Use sketch plane's coordinate system for proper 3D positioning
    gp_Dir planeNormal = m_plane.normal();
    gp_Pnt planeOrigin = m_plane.origin();

    // Get entity edges directly using entity-specific methods
    try {
      switch (entity->type()) {
      case EntityType::Point:
        // Points cannot form edges, skip
        break;

      case EntityType::Line: {
        auto *line = static_cast<const SketchLine *>(entity.get());
        // Convert 2D sketch coordinates to 3D using plane transform
        gp_Pnt p1 =
            m_plane.to3D(line->startPoint().X(), line->startPoint().Y());
        gp_Pnt p2 = m_plane.to3D(line->endPoint().X(), line->endPoint().Y());
        if (p1.Distance(p2) > 1e-6) { // Avoid degenerate edges
          BRepBuilderAPI_MakeEdge edgeBuilder(p1, p2);
          if (edgeBuilder.IsDone()) {
            wireBuilder.Add(edgeBuilder.Edge());
          }
        }
        break;
      }
      case EntityType::Circle: {
        auto *circle = static_cast<const SketchCircle *>(entity.get());
        // Convert center to 3D using plane transform
        gp_Pnt center =
            m_plane.to3D(circle->center().X(), circle->center().Y());
        // Use plane's normal direction for circle axis
        gp_Ax2 ax(center, planeNormal);
        gp_Circ circ(ax, circle->radius());
        BRepBuilderAPI_MakeEdge edgeBuilder(circ);
        if (edgeBuilder.IsDone()) {
          wireBuilder.Add(edgeBuilder.Edge());
        }
        break;
      }
      case EntityType::Arc: {
        auto *arc = static_cast<const SketchArc *>(entity.get());
        // GC_MakeArcOfCircle(p1, p2, p3) creates arc: p1 -> p2 -> p3
        // UI clicks: arcStart(1st), arcEnd(2nd), arcThrough(3rd)
        // Arc should go: start -> through -> end

        if (arc->hasThreePointData()) {
          gp_Pnt pStart =
              m_plane.to3D(arc->arcStart().X(), arc->arcStart().Y());
          gp_Pnt pThrough =
              m_plane.to3D(arc->arcThrough().X(), arc->arcThrough().Y());
          gp_Pnt pEnd = m_plane.to3D(arc->arcEnd().X(), arc->arcEnd().Y());

          // Create arc: start -> through -> end
          TopoDS_Edge arcEdge = CreateArc3Points(pStart, pThrough, pEnd);
          if (!arcEdge.IsNull()) {
            wireBuilder.Add(arcEdge);
          }
        } else {
          // Fallback for angle-based arcs
          gp_Pnt p1 =
              m_plane.to3D(arc->startPoint().X(), arc->startPoint().Y());
          gp_Pnt p2 = m_plane.to3D(arc->midPoint().X(), arc->midPoint().Y());
          gp_Pnt p3 = m_plane.to3D(arc->endPoint().X(), arc->endPoint().Y());

          TopoDS_Edge arcEdge = CreateArc3Points(p1, p2, p3);
          if (!arcEdge.IsNull()) {
            wireBuilder.Add(arcEdge);
          }
        }
        break;
      }
      case EntityType::Ellipse: {
        auto *ellipse = static_cast<const SketchEllipse *>(entity.get());
        gp_Pnt center =
            m_plane.to3D(ellipse->center().X(), ellipse->center().Y());
        gp_Ax2 ax(center, planeNormal);
        gp_Elips elips(ax, ellipse->majorRadius(), ellipse->minorRadius());
        BRepBuilderAPI_MakeEdge edgeBuilder(elips);
        if (edgeBuilder.IsDone()) {
          wireBuilder.Add(edgeBuilder.Edge());
        }
        break;
      }
      case EntityType::Rectangle: {
        auto *rect = static_cast<const SketchRectangle *>(entity.get());
        gp_Pnt c1 = m_plane.to3D(rect->corner1().X(), rect->corner1().Y());
        gp_Pnt c2 = m_plane.to3D(rect->corner2().X(), rect->corner1().Y());
        gp_Pnt c3 = m_plane.to3D(rect->corner2().X(), rect->corner2().Y());
        gp_Pnt c4 = m_plane.to3D(rect->corner1().X(), rect->corner2().Y());

        BRepBuilderAPI_MakeEdge e1(c1, c2);
        BRepBuilderAPI_MakeEdge e2(c2, c3);
        BRepBuilderAPI_MakeEdge e3(c3, c4);
        BRepBuilderAPI_MakeEdge e4(c4, c1);

        if (e1.IsDone())
          wireBuilder.Add(e1.Edge());
        if (e2.IsDone())
          wireBuilder.Add(e2.Edge());
        if (e3.IsDone())
          wireBuilder.Add(e3.Edge());
        if (e4.IsDone())
          wireBuilder.Add(e4.Edge());
        break;
      }
      case EntityType::Spline: {
        // For splines, use the curve() method
        Handle(Geom2d_Curve) curve2d = entity->curve();
        if (!curve2d.IsNull()) {
          BRepBuilderAPI_MakeEdge2d edgeBuilder(curve2d);
          if (edgeBuilder.IsDone()) {
            wireBuilder.Add(edgeBuilder.Edge());
          }
        }
        break;
      }
      default:
        // Generic fallback for any other curve types
        Handle(Geom2d_Curve) curve2d = entity->curve();
        if (!curve2d.IsNull()) {
          BRepBuilderAPI_MakeEdge2d edgeBuilder(curve2d);
          if (edgeBuilder.IsDone()) {
            wireBuilder.Add(edgeBuilder.Edge());
          }
        }
        break;
      }
    } catch (...) {
      continue;
    }
  }

  // Even if not fully closed, try to return what we have
  if (wireBuilder.IsDone()) {
    return wireBuilder.Wire();
  }

  // Try to get partial wire anyway
  if (wireBuilder.Error() == BRepBuilderAPI_WireDone ||
      wireBuilder.Error() == BRepBuilderAPI_DisconnectedWire) {
    try {
      return wireBuilder.Wire();
    } catch (...) {
      // Fall through to return empty wire
    }
  }

  return TopoDS_Wire();
}

TopoDS_Face Sketch::buildFace() const {
  TopoDS_Wire wire = buildWire();
  if (wire.IsNull()) {
    return TopoDS_Face();
  }

  try {
    BRepBuilderAPI_MakeFace faceBuilder(wire, true);
    if (faceBuilder.IsDone()) {
      return faceBuilder.Face();
    }
  } catch (...) {
  }

  return TopoDS_Face();
}

bool Sketch::isClosed() const {
  TopoDS_Wire wire = buildWire();
  return !wire.IsNull() && wire.Closed();
}

std::vector<TopoDS_Wire> Sketch::buildAllWires() const {
  // TODO: Implement detection of multiple closed profiles
  std::vector<TopoDS_Wire> wires;
  TopoDS_Wire wire = buildWire();
  if (!wire.IsNull()) {
    wires.push_back(wire);
  }
  return wires;
}

TopoDS_Compound Sketch::buildCompound() const {
  TopoDS_Compound compound;
  BRep_Builder builder;
  builder.MakeCompound(compound);

  gp_Dir planeNormal = m_plane.normal();

  for (const auto &entity : m_entities) {
    if (entity->isConstruction())
      continue;

    try {
      switch (entity->type()) {
      case EntityType::Point:
        // Skip points
        break;

      case EntityType::Line: {
        auto *line = static_cast<const SketchLine *>(entity.get());
        gp_Pnt p1 =
            m_plane.to3D(line->startPoint().X(), line->startPoint().Y());
        gp_Pnt p2 = m_plane.to3D(line->endPoint().X(), line->endPoint().Y());
        if (p1.Distance(p2) > 1e-6) {
          BRepBuilderAPI_MakeEdge edgeBuilder(p1, p2);
          if (edgeBuilder.IsDone()) {
            builder.Add(compound, edgeBuilder.Edge());
          }
        }
        break;
      }
      case EntityType::Circle: {
        auto *circle = static_cast<const SketchCircle *>(entity.get());
        gp_Pnt center =
            m_plane.to3D(circle->center().X(), circle->center().Y());
        gp_Ax2 ax(center, planeNormal);
        gp_Circ circ(ax, circle->radius());
        BRepBuilderAPI_MakeEdge edgeBuilder(circ);
        if (edgeBuilder.IsDone()) {
          builder.Add(compound, edgeBuilder.Edge());
        }
        break;
      }
      case EntityType::Arc: {
        auto *arc = static_cast<const SketchArc *>(entity.get());

        if (arc->hasThreePointData()) {
          gp_Pnt pStart =
              m_plane.to3D(arc->arcStart().X(), arc->arcStart().Y());
          gp_Pnt pThrough =
              m_plane.to3D(arc->arcThrough().X(), arc->arcThrough().Y());
          gp_Pnt pEnd = m_plane.to3D(arc->arcEnd().X(), arc->arcEnd().Y());

          TopoDS_Edge arcEdge = CreateArc3Points(pStart, pThrough, pEnd);
          if (!arcEdge.IsNull()) {
            builder.Add(compound, arcEdge);
          }
        } else {
          gp_Pnt p1 =
              m_plane.to3D(arc->startPoint().X(), arc->startPoint().Y());
          gp_Pnt p2 = m_plane.to3D(arc->midPoint().X(), arc->midPoint().Y());
          gp_Pnt p3 = m_plane.to3D(arc->endPoint().X(), arc->endPoint().Y());

          TopoDS_Edge arcEdge = CreateArc3Points(p1, p2, p3);
          if (!arcEdge.IsNull()) {
            builder.Add(compound, arcEdge);
          }
        }
        break;
      }
      case EntityType::Ellipse: {
        auto *ellipse = static_cast<const SketchEllipse *>(entity.get());
        gp_Pnt center =
            m_plane.to3D(ellipse->center().X(), ellipse->center().Y());
        gp_Ax2 ax(center, planeNormal);
        gp_Elips elips(ax, ellipse->majorRadius(), ellipse->minorRadius());
        BRepBuilderAPI_MakeEdge edgeBuilder(elips);
        if (edgeBuilder.IsDone()) {
          builder.Add(compound, edgeBuilder.Edge());
        }
        break;
      }
      case EntityType::Rectangle: {
        auto *rect = static_cast<const SketchRectangle *>(entity.get());
        gp_Pnt c1 = m_plane.to3D(rect->corner1().X(), rect->corner1().Y());
        gp_Pnt c2 = m_plane.to3D(rect->corner2().X(), rect->corner1().Y());
        gp_Pnt c3 = m_plane.to3D(rect->corner2().X(), rect->corner2().Y());
        gp_Pnt c4 = m_plane.to3D(rect->corner1().X(), rect->corner2().Y());

        BRepBuilderAPI_MakeEdge e1(c1, c2);
        BRepBuilderAPI_MakeEdge e2(c2, c3);
        BRepBuilderAPI_MakeEdge e3(c3, c4);
        BRepBuilderAPI_MakeEdge e4(c4, c1);

        if (e1.IsDone())
          builder.Add(compound, e1.Edge());
        if (e2.IsDone())
          builder.Add(compound, e2.Edge());
        if (e3.IsDone())
          builder.Add(compound, e3.Edge());
        if (e4.IsDone())
          builder.Add(compound, e4.Edge());
        break;
      }
      case EntityType::Spline: {
        // For splines, try to use the 3D curve from the edge
        Handle(Geom2d_Curve) curve2d = entity->curve();
        if (!curve2d.IsNull()) {
          // Convert 2D curve to 3D using plane
          gp_Pln occ_plane = m_plane.plane();
          Handle(Geom_Surface) planeSurf = new Geom_Plane(occ_plane);
          Handle(Geom2dAdaptor_Curve) adaptor2d =
              new Geom2dAdaptor_Curve(curve2d);

          // Sample points along the spline and create edges
          int numSamples = 20;
          double u1 = curve2d->FirstParameter();
          double u2 = curve2d->LastParameter();

          for (int i = 0; i < numSamples; ++i) {
            double t1 = u1 + (u2 - u1) * i / numSamples;
            double t2 = u1 + (u2 - u1) * (i + 1) / numSamples;
            gp_Pnt2d pt1, pt2;
            curve2d->D0(t1, pt1);
            curve2d->D0(t2, pt2);
            gp_Pnt p1 = m_plane.to3D(pt1.X(), pt1.Y());
            gp_Pnt p2 = m_plane.to3D(pt2.X(), pt2.Y());
            if (p1.Distance(p2) > 1e-6) {
              BRepBuilderAPI_MakeEdge edgeBuilder(p1, p2);
              if (edgeBuilder.IsDone()) {
                builder.Add(compound, edgeBuilder.Edge());
              }
            }
          }
        }
        break;
      }
      case EntityType::Polygon: {
        auto *polygon = static_cast<const SketchPolygon *>(entity.get());
        const auto vertices = polygon->getVertices();
        if (vertices.size() >= 3) {
          for (size_t i = 0; i < vertices.size(); ++i) {
            size_t nextIdx = (i + 1) % vertices.size();
            gp_Pnt p1 = m_plane.to3D(vertices[i].X(), vertices[i].Y());
            gp_Pnt p2 =
                m_plane.to3D(vertices[nextIdx].X(), vertices[nextIdx].Y());
            if (p1.Distance(p2) > 1e-6) {
              BRepBuilderAPI_MakeEdge edgeBuilder(p1, p2);
              if (edgeBuilder.IsDone()) {
                builder.Add(compound, edgeBuilder.Edge());
              }
            }
          }
        }
        break;
      }
      case EntityType::Slot: {
        auto *slot = static_cast<const SketchSlot *>(entity.get());
        gp_Pnt2d c1_2d = slot->center1();
        gp_Pnt2d c2_2d = slot->center2();
        double halfWidth = slot->width() / 2.0;

        gp_Vec2d dir2d(c2_2d.X() - c1_2d.X(), c2_2d.Y() - c1_2d.Y());
        if (dir2d.Magnitude() > 1e-6) {
          dir2d.Normalize();
          gp_Vec2d perpDir2d(-dir2d.Y(), dir2d.X());
          perpDir2d.Scale(halfWidth);

          gp_Pnt2d p1_2d(c1_2d.X() + perpDir2d.X(), c1_2d.Y() + perpDir2d.Y());
          gp_Pnt2d p2_2d(c1_2d.X() - perpDir2d.X(), c1_2d.Y() - perpDir2d.Y());
          gp_Pnt2d p3_2d(c2_2d.X() - perpDir2d.X(), c2_2d.Y() - perpDir2d.Y());
          gp_Pnt2d p4_2d(c2_2d.X() + perpDir2d.X(), c2_2d.Y() + perpDir2d.Y());

          gp_Pnt c1 = m_plane.to3D(c1_2d.X(), c1_2d.Y());
          gp_Pnt c2 = m_plane.to3D(c2_2d.X(), c2_2d.Y());
          gp_Pnt p1 = m_plane.to3D(p1_2d.X(), p1_2d.Y());
          gp_Pnt p2 = m_plane.to3D(p2_2d.X(), p2_2d.Y());
          gp_Pnt p3 = m_plane.to3D(p3_2d.X(), p3_2d.Y());
          gp_Pnt p4 = m_plane.to3D(p4_2d.X(), p4_2d.Y());

          try {
            gp_Circ circ1(gp_Ax2(c1, planeNormal), halfWidth);
            TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(circ1, p1, p2);
            if (!edge1.IsNull())
              builder.Add(compound, edge1);

            BRepBuilderAPI_MakeEdge edge2(p2, p3);
            if (edge2.IsDone())
              builder.Add(compound, edge2.Edge());

            gp_Circ circ2(gp_Ax2(c2, planeNormal), halfWidth);
            TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(circ2, p3, p4);
            if (!edge3.IsNull())
              builder.Add(compound, edge3);

            BRepBuilderAPI_MakeEdge edge4(p4, p1);
            if (edge4.IsDone())
              builder.Add(compound, edge4.Edge());
          } catch (...) {
          }
        }
        break;
      }
      default:
        break;
      }
    } catch (...) {
      continue;
    }
  }

  return compound;
}

// === Multiple Closed Profile Detection ===

std::vector<TopoDS_Wire> Sketch::detectClosedProfiles() const {
  std::vector<TopoDS_Wire> closedWires;

  // Use the robust face builder which handles all intersections and entity
  // types
  std::vector<TopoDS_Face> faces = buildProfileFaces();

  for (const auto &face : faces) {
    TopExp_Explorer wireExp(face, TopAbs_WIRE);
    // Usually the first wire is the outer bounding wire of the face
    if (wireExp.More()) {
      TopoDS_Wire wire = TopoDS::Wire(wireExp.Current());
      if (wire.Closed()) {
        closedWires.push_back(wire);
      }
    }
  }

  qDebug() << "detectClosedProfiles: Returning" << closedWires.size()
           << "closed profiles from faces";
  return closedWires;
}

std::vector<TopoDS_Face> Sketch::buildProfileFaces() const {
  std::vector<TopoDS_Face> finalFaces;

  // 1. Get all valid edges from sketch compound
  TopTools_ListOfShape sketchEdges;
  bool hasEdges = false;

  TopoDS_Compound compound = buildCompound();
  TopExp_Explorer edgeExp(compound, TopAbs_EDGE);
  while (edgeExp.More()) {
    sketchEdges.Append(edgeExp.Current());
    hasEdges = true;
    edgeExp.Next();
  }

  if (!hasEdges)
    return finalFaces;

  // 2. Create a Substrate face to split
  double minX = std::numeric_limits<double>::max();
  double minY = std::numeric_limits<double>::max();
  double maxX = std::numeric_limits<double>::lowest();
  double maxY = std::numeric_limits<double>::lowest();
  bool hasBounds = false;

  for (const auto &entity : m_entities) {
    if (entity->isConstruction() || entity->type() == EntityType::Point)
      continue;

    try {
      gp_Pnt2d p1 = entity->startPoint();
      gp_Pnt2d p2 = entity->endPoint();
      gp_Pnt2d p3 = entity->midPoint();
      minX = std::min({minX, p1.X(), p2.X(), p3.X()});
      minY = std::min({minY, p1.Y(), p2.Y(), p3.Y()});
      maxX = std::max({maxX, p1.X(), p2.X(), p3.X()});
      maxY = std::max({maxY, p1.Y(), p2.Y(), p3.Y()});
      hasBounds = true;
    } catch (...) {
    }
  }

  if (!hasBounds) {
    minX = -100;
    minY = -100;
    maxX = 100;
    maxY = 100;
  }

  // Large margin to ensure entire sketch fits safely inside substrate
  double margin = std::max({maxX - minX, maxY - minY, 100.0}) * 10.0;
  double subWidth = (maxX + margin) - (minX - margin);
  double subHeight = (maxY + margin) - (minY - margin);
  double subArea = subWidth * subHeight;

  gp_Pnt p1 = m_plane.to3D(minX - margin, minY - margin);
  gp_Pnt p2 = m_plane.to3D(maxX + margin, minY - margin);
  gp_Pnt p3 = m_plane.to3D(maxX + margin, maxY + margin);
  gp_Pnt p4 = m_plane.to3D(minX - margin, maxY + margin);

  TopoDS_Wire substrateWire =
      BRepBuilderAPI_MakePolygon(p1, p2, p3, p4, true).Wire();
  TopoDS_Face substrateFace = BRepBuilderAPI_MakeFace(substrateWire).Face();

  // 3. Run Splitter
  BRepAlgoAPI_Splitter splitter;
  TopTools_ListOfShape arguments;
  arguments.Append(substrateFace);
  splitter.SetArguments(arguments);
  splitter.SetTools(sketchEdges);
  splitter.Build();

  if (!splitter.IsDone())
    return finalFaces;

  // 4. Filter faces
  const TopoDS_Shape &result = splitter.Shape();
  TopExp_Explorer faceExp(result, TopAbs_FACE);

  while (faceExp.More()) {
    TopoDS_Face face = TopoDS::Face(faceExp.Current());

    GProp_GProps props;
    try {
      BRepGProp::SurfaceProperties(face, props);
      double faceArea = props.Mass();

      // If the face area is massive (approaching the substrate size), it's the
      // outer cutaway piece
      if (faceArea > subArea * 0.9) {
        faceExp.Next();
        continue;
      }
    } catch (...) {
    }

    finalFaces.push_back(face);
    faceExp.Next();
  }

  // 5. Sort deterministically
  struct FaceProps {
    TopoDS_Face face;
    gp_Pnt center;
    double area;
  };

  std::vector<FaceProps> propsList;
  propsList.reserve(finalFaces.size());

  for (const auto &face : finalFaces) {
    GProp_GProps props;
    try {
      BRepGProp::SurfaceProperties(face, props);
      propsList.push_back({face, props.CentreOfMass(), props.Mass()});
    } catch (...) {
      propsList.push_back({face, gp_Pnt(0, 0, 0), 0.0});
    }
  }

  std::sort(propsList.begin(), propsList.end(),
            [](const FaceProps &a, const FaceProps &b) {
              if (std::abs(a.center.X() - b.center.X()) > 1e-6)
                return a.center.X() < b.center.X();
              if (std::abs(a.center.Y() - b.center.Y()) > 1e-6)
                return a.center.Y() < b.center.Y();
              return a.area < b.area;
            });

  finalFaces.clear();
  for (const auto &p : propsList) {
    finalFaces.push_back(p.face);
  }

  qDebug() << "Sketch::buildProfileFaces: Generated" << finalFaces.size()
           << "topologically perfect disjoint faces.";
  return finalFaces;
}

int Sketch::closedProfileCount() const {
  return static_cast<int>(detectClosedProfiles().size());
}

std::vector<TopoDS_Shape> Sketch::extractProfiles(bool includeOpenWires) const {
  std::vector<TopoDS_Shape> result;

  // 1. Always get closed profile faces
  auto faces = buildProfileFaces();
  for (const auto &f : faces) {
    result.push_back(f);
  }

  // 2. If requested, extract open wires
  if (includeOpenWires) {
    // If the sketch hasn't formed any closed faces but has entities,
    // we try to build a continuous wire out of it.
    // For Sweep, you usually just have a single open wire or multiple
    // disconnected ones. For simplicity, we can use detectClosedProfiles logic
    // or buildAllWires. Actually, we'll try to build the main wire.
    TopoDS_Wire mainWire = buildWire();
    if (!mainWire.IsNull() && !mainWire.Closed()) {
      // It's an open wire! Add it to profiles.
      result.push_back(mainWire);
    } else if (faces.empty()) {
      // If buildWire failed or was closed but didn't form a face (rare),
      // let's just expose every non-construction entity as an individual
      // wire/edge so the user can at least select them.
      TopExp_Explorer wireExp(mainWire, TopAbs_EDGE);
      if (!wireExp.More()) { // If mainWire was null/empty
        for (const auto &entity : m_entities) {
          if (entity->isConstruction() || entity->type() == EntityType::Point)
            continue;
          try {
            // We use the same compound logic to get edges
            TopoDS_Compound comp = buildCompound();
            TopExp_Explorer compExp(comp, TopAbs_EDGE);
            while (compExp.More()) {
              BRepBuilderAPI_MakeWire mkWire(TopoDS::Edge(compExp.Current()));
              if (mkWire.IsDone()) {
                result.push_back(mkWire.Wire());
              }
              compExp.Next();
            }
            break; // We extracted all edges from the compound
          } catch (...) {
          }
        }
      }
    }
  }

  return result;
}

// === Selection ===

void Sketch::selectEntity(SketchEntity::Ptr entity) {
  if (entity) {
    entity->setSelected(true);
  }
}

void Sketch::deselectEntity(SketchEntity::Ptr entity) {
  if (entity) {
    entity->setSelected(false);
  }
}

void Sketch::clearSelection() {
  for (auto &entity : m_entities) {
    entity->setSelected(false);
  }
}

std::vector<SketchEntity::Ptr> Sketch::selectedEntities() const {
  std::vector<SketchEntity::Ptr> selected;
  for (const auto &entity : m_entities) {
    if (entity->isSelected()) {
      selected.push_back(entity);
    }
  }
  return selected;
}

// === Advanced Sketch Features (FAZ 5) ===

SketchPolygon::Ptr Sketch::addPolygon(const gp_Pnt2d &center,
                                      const gp_Pnt2d &vertex, int sides) {
  auto polygon = std::make_shared<SketchPolygon>(center, vertex, sides);
  addEntity(polygon);
  return polygon;
}

void Sketch::offsetEntities(const std::vector<SketchEntity::Ptr> &entities,
                            double distance) {
  if (entities.empty() || std::abs(distance) < 1e-6)
    return;

  // Process each entity
  for (const auto &entity : entities) {
    switch (entity->type()) {
    case EntityType::Line: {
      auto line = std::dynamic_pointer_cast<SketchLine>(entity);
      if (line) {
        // Calculate perpendicular offset direction
        double dx = line->endPoint().X() - line->startPoint().X();
        double dy = line->endPoint().Y() - line->startPoint().Y();
        double len = std::sqrt(dx * dx + dy * dy);
        if (len > 1e-6) {
          // Perpendicular direction (normalized)
          double nx = -dy / len;
          double ny = dx / len;

          // Create offset line
          gp_Pnt2d newStart(line->startPoint().X() + nx * distance,
                            line->startPoint().Y() + ny * distance);
          gp_Pnt2d newEnd(line->endPoint().X() + nx * distance,
                          line->endPoint().Y() + ny * distance);
          addLine(newStart, newEnd);
        }
      }
      break;
    }
    case EntityType::Circle: {
      auto circle = std::dynamic_pointer_cast<SketchCircle>(entity);
      if (circle) {
        double newRadius = circle->radius() + distance;
        if (newRadius > 0) {
          addCircle(circle->center(), newRadius);
        }
      }
      break;
    }
    case EntityType::Arc: {
      auto arc = std::dynamic_pointer_cast<SketchArc>(entity);
      if (arc) {
        double newRadius = arc->radius() + distance;
        if (newRadius > 0) {
          addArc(arc->center(), newRadius, arc->startAngle(), arc->endAngle());
        }
      }
      break;
    }
    case EntityType::Rectangle: {
      auto rect = std::dynamic_pointer_cast<SketchRectangle>(entity);
      if (rect) {
        // Offset rectangle by creating new one
        double newX = rect->corner1().X() - distance;
        double newY = rect->corner1().Y() - distance;
        double newW = rect->width() + 2 * distance;
        double newH = rect->height() + 2 * distance;
        if (newW > 0 && newH > 0) {
          addRectangle(newX, newY, newW, newH);
        }
      }
      break;
    }
    default:
      // Other entity types not yet supported for offset
      break;
    }
  }
}

std::vector<SketchEntity::Ptr> Sketch::offsetWire(const TopoDS_Wire &wire,
                                                  double distance) {
  std::vector<SketchEntity::Ptr> result;
  // TODO: Implement using BRepOffsetAPI_MakeOffset for proper wire offset
  // For now, return empty - this is a placeholder for advanced implementation
  return result;
}

// === Undo/Redo Implementation ===

void Sketch::saveCheckpoint(const std::string &description) {
  // Remove any redo history beyond current index
  if (m_undoIndex >= 0 &&
      m_undoIndex < static_cast<int>(m_undoHistory.size()) - 1) {
    m_undoHistory.erase(m_undoHistory.begin() + m_undoIndex + 1,
                        m_undoHistory.end());
  }

  // Create snapshot by cloning all entities
  Snapshot snapshot;
  snapshot.description = description;
  for (const auto &entity : m_entities) {
    if (entity) {
      snapshot.entities.push_back(entity->clone());
    }
  }

  // Add to history
  m_undoHistory.push_back(snapshot);
  m_undoIndex = static_cast<int>(m_undoHistory.size()) - 1;

  // Limit history size
  if (m_undoHistory.size() > MaxUndoHistory) {
    m_undoHistory.erase(m_undoHistory.begin());
    m_undoIndex--;
  }
}

bool Sketch::undo() {
  if (!canUndo())
    return false;

  // Save current state to redo stack if this is first undo after edits
  if (m_undoIndex == static_cast<int>(m_undoHistory.size()) - 1) {
    // We're at the latest state, need to save it for potential redo
    Snapshot current;
    current.description = "Current";
    for (const auto &entity : m_entities) {
      if (entity) {
        current.entities.push_back(entity->clone());
      }
    }
    m_undoHistory.push_back(current);
  }

  // Restore previous state
  m_undoIndex--;
  const Snapshot &snapshot = m_undoHistory[m_undoIndex];

  // Clear current entities and restore from snapshot
  m_entities.clear();
  for (const auto &entity : snapshot.entities) {
    if (entity) {
      m_entities.push_back(entity->clone());
    }
  }

  return true;
}

bool Sketch::redo() {
  if (!canRedo())
    return false;

  // Restore next state
  m_undoIndex++;
  const Snapshot &snapshot = m_undoHistory[m_undoIndex];

  // Clear current entities and restore from snapshot
  m_entities.clear();
  for (const auto &entity : snapshot.entities) {
    if (entity) {
      m_entities.push_back(entity->clone());
    }
  }

  return true;
}

// === Linked Geometry ===

std::vector<SketchEntity::Ptr>
Sketch::addProjectedEntity(const TopoDS_Shape &shape) {
  std::vector<SketchEntity::Ptr> createdEntities;

  // Safety check
  if (shape.IsNull())
    return createdEntities;

  // Project based on type
  if (shape.ShapeType() == TopAbs_VERTEX) {
    TopoDS_Vertex v = TopoDS::Vertex(shape);
    gp_Pnt p3d = BRep_Tool::Pnt(v);

    // Project point to plane
    gp_Pln pln = m_plane.plane();

    Standard_Real u, v_param;
    ElSLib::PlaneParameters(pln.Position(), p3d, u, v_param);

    auto point = addPoint(u, v_param);
    point->setConstruction(true); // Mark as reference

    // Fix it so it doesn't move
    addConstraint(std::make_shared<FixConstraint>(point));

    createdEntities.push_back(point);
  } else if (shape.ShapeType() == TopAbs_EDGE) {
    TopoDS_Edge edge = TopoDS::Edge(shape);

    // Use GeomProjLib to project curve to plane
    double f, l;
    Handle(Geom_Curve) curve3d = BRep_Tool::Curve(edge, f, l);
    if (!curve3d.IsNull()) {
      Handle(Geom_Plane) geomPlane = new Geom_Plane(m_plane.plane());
      Handle(Geom_Curve) projectedCurve = GeomProjLib::ProjectOnPlane(
          curve3d, geomPlane, m_plane.normal(), Standard_True);

      if (!projectedCurve.IsNull()) {
        if (projectedCurve->IsKind(STANDARD_TYPE(Geom_Line))) {
          Handle(Geom_Line) gLine = Handle(Geom_Line)::DownCast(projectedCurve);
          // Calculate start/end on projected curve corresponding to f/l
          // Using ElSLib projection or D0
          gp_Pnt p1, p2;
          projectedCurve->D0(f, p1);
          projectedCurve->D0(l, p2);

          gp_Pnt2d p2d1 = m_plane.to2D(p1);
          gp_Pnt2d p2d2 = m_plane.to2D(p2);

          auto line = addLine(p2d1, p2d2);
          line->setConstruction(true);
          addConstraint(std::make_shared<FixConstraint>(line));
          createdEntities.push_back(line);
        } else if (projectedCurve->IsKind(STANDARD_TYPE(Geom_Circle))) {
          Handle(Geom_Circle) gCirc =
              Handle(Geom_Circle)::DownCast(projectedCurve);
          gp_Pnt center = gCirc->Location();
          gp_Pnt2d c2d = m_plane.to2D(center);
          double r = gCirc->Radius();

          auto circle = addCircle(c2d, r);
          circle->setConstruction(true);
          addConstraint(std::make_shared<FixConstraint>(circle));
          createdEntities.push_back(circle);
        }
      }
    }
  }

  // Store the projection link
  if (!createdEntities.empty()) {
    m_projections.push_back({shape, createdEntities});
  }

  return createdEntities;
}

std::vector<SketchEntity::Ptr>
Sketch::addProjectedFace(const TopoDS_Face &face) {
  std::vector<SketchEntity::Ptr> createdEntities;

  if (face.IsNull())
    return createdEntities;

  // Get the outer wire of the face and project its edges
  TopoDS_Wire outerWire = BRepTools::OuterWire(face);
  if (!outerWire.IsNull()) {
    auto wireEntities = addProjectedWire(outerWire);
    createdEntities.insert(createdEntities.end(), wireEntities.begin(),
                           wireEntities.end());
  }

  // Also get inner wires (holes) if any
  TopExp_Explorer wireExp(face, TopAbs_WIRE);
  for (; wireExp.More(); wireExp.Next()) {
    TopoDS_Wire wire = TopoDS::Wire(wireExp.Current());
    if (!wire.IsSame(outerWire)) {
      auto wireEntities = addProjectedWire(wire);
      createdEntities.insert(createdEntities.end(), wireEntities.begin(),
                             wireEntities.end());
    }
  }

  return createdEntities;
}

std::vector<SketchEntity::Ptr>
Sketch::addProjectedWire(const TopoDS_Wire &wire) {
  std::vector<SketchEntity::Ptr> createdEntities;

  if (wire.IsNull())
    return createdEntities;

  // Iterate through all edges in the wire
  TopExp_Explorer edgeExp(wire, TopAbs_EDGE);
  for (; edgeExp.More(); edgeExp.Next()) {
    TopoDS_Edge edge = TopoDS::Edge(edgeExp.Current());

    // Use existing addProjectedEntity for each edge
    auto edgeEntities = addProjectedEntity(edge);
    createdEntities.insert(createdEntities.end(), edgeEntities.begin(),
                           edgeEntities.end());
  }

  return createdEntities;
}

void Sketch::updateLinkedGeometries() {
  for (auto &proj : m_projections) {
    TopoDS_Shape shape = proj.sourceShape;
    if (shape.IsNull())
      continue;

    // Reproject and update existing entities
    if (shape.ShapeType() == TopAbs_VERTEX) {
      TopoDS_Vertex v = TopoDS::Vertex(shape);
      gp_Pnt p3d = BRep_Tool::Pnt(v);

      gp_Pln pln = m_plane.plane();
      Standard_Real u, v_param;
      ElSLib::PlaneParameters(pln.Position(), p3d, u, v_param);

      // Update point entity
      for (auto &entity : proj.targetEntities) {
        if (auto point = std::dynamic_pointer_cast<SketchPoint>(entity)) {
          point->setPosition(u, v_param);
        }
      }
    } else if (shape.ShapeType() == TopAbs_EDGE) {
      TopoDS_Edge edge = TopoDS::Edge(shape);

      double f, l;
      Handle(Geom_Curve) curve3d = BRep_Tool::Curve(edge, f, l);
      if (!curve3d.IsNull()) {
        Handle(Geom_Plane) geomPlane = new Geom_Plane(m_plane.plane());
        Handle(Geom_Curve) projectedCurve = GeomProjLib::ProjectOnPlane(
            curve3d, geomPlane, m_plane.normal(), Standard_True);

        if (!projectedCurve.IsNull()) {
          if (projectedCurve->IsKind(STANDARD_TYPE(Geom_Line))) {
            gp_Pnt p1, p2;
            projectedCurve->D0(f, p1);
            projectedCurve->D0(l, p2);
            gp_Pnt2d p2d1 = m_plane.to2D(p1);
            gp_Pnt2d p2d2 = m_plane.to2D(p2);

            for (auto &entity : proj.targetEntities) {
              if (auto line = std::dynamic_pointer_cast<SketchLine>(entity)) {
                line->setStartPoint(p2d1);
                line->setEndPoint(p2d2);
              }
            }
          } else if (projectedCurve->IsKind(STANDARD_TYPE(Geom_Circle))) {
            Handle(Geom_Circle) gCirc =
                Handle(Geom_Circle)::DownCast(projectedCurve);
            gp_Pnt center = gCirc->Location();
            gp_Pnt2d c2d = m_plane.to2D(center);
            double r = gCirc->Radius();

            for (auto &entity : proj.targetEntities) {
              if (auto circle =
                      std::dynamic_pointer_cast<SketchCircle>(entity)) {
                circle->setCenter(c2d);
                circle->setRadius(r);
              }
            }
          }
        }
      }
    }
  }
}

} // namespace sketch
} // namespace opencad
