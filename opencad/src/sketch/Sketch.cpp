/**
 * @file Sketch.cpp
 * @brief Implementation of Sketch class
 */

#include "Sketch.h"
#include "constraints/CoincidentConstraint.h"
#include "constraints/DimensionConstraint.h"
#include "constraints/HorizontalConstraint.h"
#include "constraints/VerticalConstraint.h"

#include "constraints/FixConstraint.h"
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeEdge2d.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <ElSLib.hxx>
#include <GC_MakeArcOfCircle.hxx>
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
  gp_Dir planeNormal = m_plane.normal();

  // Step 1: Collect all edges from all entities (except construction)
  TopTools_ListOfShape allEdges;
  std::set<uint64_t> processedEntities; // Track which entities we've processed

  for (const auto &entity : m_entities) {
    if (entity->isConstruction())
      continue;

    try {
      // Handle single-entity closed profiles first (Circle, Ellipse, Rectangle)
      if (entity->type() == EntityType::Circle) {
        auto *circle = static_cast<const SketchCircle *>(entity.get());
        gp_Pnt center =
            m_plane.to3D(circle->center().X(), circle->center().Y());
        gp_Ax2 ax(center, planeNormal);
        gp_Circ circ(ax, circle->radius());
        BRepBuilderAPI_MakeEdge edgeBuilder(circ);
        if (edgeBuilder.IsDone()) {
          BRepBuilderAPI_MakeWire wireBuilder(edgeBuilder.Edge());
          if (wireBuilder.IsDone() && wireBuilder.Wire().Closed()) {
            closedWires.push_back(wireBuilder.Wire());
            processedEntities.insert(entity->id());
            continue; // Skip adding to allEdges
          }
        }
      } else if (entity->type() == EntityType::Ellipse) {
        auto *ellipse = static_cast<const SketchEllipse *>(entity.get());
        gp_Pnt center =
            m_plane.to3D(ellipse->center().X(), ellipse->center().Y());
        gp_Ax2 ax(center, planeNormal);
        gp_Elips elips(ax, ellipse->majorRadius(), ellipse->minorRadius());
        BRepBuilderAPI_MakeEdge edgeBuilder(elips);
        if (edgeBuilder.IsDone()) {
          BRepBuilderAPI_MakeWire wireBuilder(edgeBuilder.Edge());
          if (wireBuilder.IsDone() && wireBuilder.Wire().Closed()) {
            closedWires.push_back(wireBuilder.Wire());
            processedEntities.insert(entity->id());
            continue; // Skip adding to allEdges
          }
        }
      } else if (entity->type() == EntityType::Rectangle) {
        auto *rect = static_cast<const SketchRectangle *>(entity.get());
        gp_Pnt c1 = m_plane.to3D(rect->corner1().X(), rect->corner1().Y());
        gp_Pnt c2 = m_plane.to3D(rect->corner2().X(), rect->corner1().Y());
        gp_Pnt c3 = m_plane.to3D(rect->corner2().X(), rect->corner2().Y());
        gp_Pnt c4 = m_plane.to3D(rect->corner1().X(), rect->corner2().Y());

        BRepBuilderAPI_MakeWire wireBuilder;
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

        if (wireBuilder.IsDone() && wireBuilder.Wire().Closed()) {
          closedWires.push_back(wireBuilder.Wire());
          processedEntities.insert(entity->id());
          continue; // Skip adding to allEdges
        }
      } else if (entity->type() == EntityType::Polygon) {
        // Handle Polygon as a closed profile
        auto *polygon = static_cast<const SketchPolygon *>(entity.get());
        const auto vertices = polygon->getVertices();

        if (vertices.size() >= 3) {
          try {
            BRepBuilderAPI_MakeWire wireBuilder;

            // Create edges between consecutive vertices
            for (size_t i = 0; i < vertices.size(); ++i) {
              size_t nextIdx = (i + 1) % vertices.size();
              gp_Pnt p1 = m_plane.to3D(vertices[i].X(), vertices[i].Y());
              gp_Pnt p2 =
                  m_plane.to3D(vertices[nextIdx].X(), vertices[nextIdx].Y());

              TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(p1, p2);
              wireBuilder.Add(edge);
            }

            if (wireBuilder.IsDone() && wireBuilder.Wire().Closed()) {
              closedWires.push_back(wireBuilder.Wire());
              processedEntities.insert(entity->id());
              continue; // Skip adding to allEdges
            }
          } catch (...) {
            // Fall through to edge-based detection
          }
        }
      } else if (entity->type() == EntityType::Slot) {
        // Handle Slot as a closed profile (elongated hole with rounded ends)
        auto *slot = static_cast<const SketchSlot *>(entity.get());
        gp_Pnt2d c1_2d = slot->center1();
        gp_Pnt2d c2_2d = slot->center2();
        double width = slot->width();
        double halfWidth = width / 2.0;

        qDebug() << "Slot profile detection: width =" << width;

        try {
          // Calculate perpendicular in 2D first
          gp_Vec2d dir2d(c2_2d.X() - c1_2d.X(), c2_2d.Y() - c1_2d.Y());
          double length = dir2d.Magnitude();

          if (length > 1e-6) {
            dir2d.Normalize();

            // Perpendicular vector in 2D (rotate 90 degrees)
            gp_Vec2d perpDir2d(-dir2d.Y(), dir2d.X());
            perpDir2d.Scale(halfWidth);

            // Calculate corner points in 2D
            gp_Pnt2d p1_2d(c1_2d.X() + perpDir2d.X(),
                           c1_2d.Y() + perpDir2d.Y());
            gp_Pnt2d p2_2d(c1_2d.X() - perpDir2d.X(),
                           c1_2d.Y() - perpDir2d.Y());
            gp_Pnt2d p3_2d(c2_2d.X() - perpDir2d.X(),
                           c2_2d.Y() - perpDir2d.Y());
            gp_Pnt2d p4_2d(c2_2d.X() + perpDir2d.X(),
                           c2_2d.Y() + perpDir2d.Y());

            // Convert to 3D using sketch plane
            gp_Pnt c1 = m_plane.to3D(c1_2d.X(), c1_2d.Y());
            gp_Pnt c2 = m_plane.to3D(c2_2d.X(), c2_2d.Y());
            gp_Pnt p1 = m_plane.to3D(p1_2d.X(), p1_2d.Y());
            gp_Pnt p2 = m_plane.to3D(p2_2d.X(), p2_2d.Y());
            gp_Pnt p3 = m_plane.to3D(p3_2d.X(), p3_2d.Y());
            gp_Pnt p4 = m_plane.to3D(p4_2d.X(), p4_2d.Y());

            BRepBuilderAPI_MakeWire wireBuilder;

            // Build wire with OUTER arcs (convex, facing outward)
            // Path: p1 -> (outer arc) -> p2 -> p3 -> (outer arc) -> p4 -> p1
            try {
              // Arc at c1 (from p1 to p2) - OUTER semicircle (convex)
              gp_Circ circ1(gp_Ax2(c1, planeNormal), halfWidth);
              TopoDS_Edge edge1 =
                  BRepBuilderAPI_MakeEdge(circ1, p1, p2); // p1->p2 (outer)
              wireBuilder.Add(edge1);

              // Line 1: p2 to p3
              TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(p2, p3);
              wireBuilder.Add(edge2);

              // Arc at c2 (from p3 to p4) - OUTER semicircle (convex)
              gp_Circ circ2(gp_Ax2(c2, planeNormal), halfWidth);
              TopoDS_Edge edge3 =
                  BRepBuilderAPI_MakeEdge(circ2, p3, p4); // p3->p4 (outer)
              wireBuilder.Add(edge3);

              // Line 2: p4 to p1
              TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(p4, p1);
              wireBuilder.Add(edge4);

              qDebug() << "  Wire builder done:" << wireBuilder.IsDone();
              if (wireBuilder.IsDone()) {
                TopoDS_Wire wire = wireBuilder.Wire();
                bool isClosed = wire.Closed();

                qDebug() << "  Wire closed (built-in):" << isClosed;

                // Manual closure check if not closed
                if (!isClosed) {
                  try {
                    TopExp_Explorer vertexExp(wire, TopAbs_VERTEX);
                    if (vertexExp.More()) {
                      TopoDS_Vertex firstVertex =
                          static_cast<const TopoDS_Vertex &>(
                              vertexExp.Current());
                      TopoDS_Vertex lastVertex = firstVertex;

                      while (vertexExp.More()) {
                        lastVertex = static_cast<const TopoDS_Vertex &>(
                            vertexExp.Current());
                        vertexExp.Next();
                      }

                      gp_Pnt firstPt = BRep_Tool::Pnt(firstVertex);
                      gp_Pnt lastPt = BRep_Tool::Pnt(lastVertex);
                      double distance = firstPt.Distance(lastPt);

                      qDebug() << "  Manual endpoint distance:" << distance;

                      if (distance < 1e-3) {
                        isClosed = true;
                        qDebug() << "  -> Manually detected as closed!";
                      }
                    }
                  } catch (...) {
                    qDebug() << "  -> Failed to check endpoints manually";
                  }
                }

                if (isClosed) {
                  closedWires.push_back(wire);
                  processedEntities.insert(entity->id());
                  qDebug() << "  -> Slot added as closed profile!";
                  continue; // Skip adding to allEdges
                } else {
                  qDebug() << "  -> Slot wire is NOT closed, skipping";
                }
              } else {
                qDebug() << "  -> Wire builder failed for slot";
              }
            } catch (const std::exception &e) {
              qDebug() << "  -> Slot edge building failed:" << e.what();
            }
          }
        } catch (const std::exception &e) {
          qDebug() << "  Slot wire building failed:" << e.what();
        } catch (...) {
          qDebug() << "  Slot wire building failed with unknown error";
        }
      } else if (entity->type() == EntityType::Spline) {
        // Handle closed Spline as a closed profile
        auto *spline = static_cast<const SketchSpline *>(entity.get());

        if (spline->isClosed()) {
          try {
            Handle(Geom2d_Curve) curve2d = spline->curve();
            if (!curve2d.IsNull()) {
              // Convert 2D curve to 3D using sketch plane
              gp_Pln plane = m_plane.plane();
              Handle(Geom_Surface) planeSurf = new Geom_Plane(plane);

              // Create 3D edge from 2D curve on the plane
              BRepBuilderAPI_MakeEdge edgeBuilder(curve2d, planeSurf);
              if (edgeBuilder.IsDone()) {
                BRepBuilderAPI_MakeWire wireBuilder(edgeBuilder.Edge());
                if (wireBuilder.IsDone() && wireBuilder.Wire().Closed()) {
                  closedWires.push_back(wireBuilder.Wire());
                  processedEntities.insert(entity->id());
                  continue; // Skip adding to allEdges
                }
              }
            }
          } catch (...) {
            // Fall through to edge-based detection
          }
        }
      }

      // For other entities (Line, Arc, Spline), add their edges to the pool
      if (processedEntities.find(entity->id()) == processedEntities.end()) {
        switch (entity->type()) {
        case EntityType::Line: {
          auto *line = static_cast<const SketchLine *>(entity.get());
          gp_Pnt p1 =
              m_plane.to3D(line->startPoint().X(), line->startPoint().Y());
          gp_Pnt p2 = m_plane.to3D(line->endPoint().X(), line->endPoint().Y());
          if (p1.Distance(p2) > 1e-6) {
            BRepBuilderAPI_MakeEdge edgeBuilder(p1, p2);
            if (edgeBuilder.IsDone()) {
              allEdges.Append(edgeBuilder.Edge());
            }
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
              allEdges.Append(arcEdge);
            }
          } else {
            gp_Pnt p1 =
                m_plane.to3D(arc->startPoint().X(), arc->startPoint().Y());
            gp_Pnt p2 = m_plane.to3D(arc->midPoint().X(), arc->midPoint().Y());
            gp_Pnt p3 = m_plane.to3D(arc->endPoint().X(), arc->endPoint().Y());
            TopoDS_Edge arcEdge = CreateArc3Points(p1, p2, p3);
            if (!arcEdge.IsNull()) {
              allEdges.Append(arcEdge);
            }
          }
          break;
        }
        case EntityType::Spline: {
          Handle(Geom2d_Curve) curve2d = entity->curve();
          if (!curve2d.IsNull()) {
            BRepBuilderAPI_MakeEdge2d edgeBuilder(curve2d);
            if (edgeBuilder.IsDone()) {
              allEdges.Append(edgeBuilder.Edge());
            }
          }
          break;
        }
        default:
          break;
        }
      }
    } catch (...) {
      continue;
    }
  }

  // Step 2: Try to build a wire from all edges using BRepBuilderAPI_MakeWire
  // This will automatically connect edges if they share endpoints
  if (!allEdges.IsEmpty()) {
    qDebug() << "detectClosedProfiles: Found" << allEdges.Extent()
             << "edges from Line/Arc/Spline";

    try {
      BRepBuilderAPI_MakeWire wireBuilder;

      // Add all edges to the wire builder
      for (TopTools_ListIteratorOfListOfShape it(allEdges); it.More();
           it.Next()) {
        const TopoDS_Shape &shape = it.Value();
        if (shape.ShapeType() == TopAbs_EDGE) {
          TopoDS_Edge edge = static_cast<const TopoDS_Edge &>(shape);
          wireBuilder.Add(edge);
        }
      }

      if (wireBuilder.IsDone()) {
        TopoDS_Wire wire = wireBuilder.Wire();

        // Check if wire is closed - use both built-in check and manual endpoint
        // check
        bool isClosed = wire.Closed();

        qDebug() << "BRepBuilderAPI_MakeWire: Built-in Closed:" << isClosed;

        // Manual check: compare first and last vertex positions
        if (!isClosed) {
          try {
            TopExp_Explorer vertexExp(wire, TopAbs_VERTEX);
            if (vertexExp.More()) {
              TopoDS_Vertex firstVertex =
                  static_cast<const TopoDS_Vertex &>(vertexExp.Current());
              TopoDS_Vertex lastVertex = firstVertex;

              // Find last vertex
              while (vertexExp.More()) {
                lastVertex =
                    static_cast<const TopoDS_Vertex &>(vertexExp.Current());
                vertexExp.Next();
              }

              gp_Pnt firstPt = BRep_Tool::Pnt(firstVertex);
              gp_Pnt lastPt = BRep_Tool::Pnt(lastVertex);
              double distance = firstPt.Distance(lastPt);

              qDebug() << "  Manual endpoint distance:" << distance;

              // Consider closed if endpoints are within tolerance
              if (distance < 1e-3) { // 0.001 units tolerance
                isClosed = true;
                qDebug() << "  -> Manually detected as closed!";
              }
            }
          } catch (...) {
            qDebug() << "  -> Failed to check endpoints manually";
          }
        }

        if (isClosed) {
          closedWires.push_back(wire);
          qDebug() << "  -> Added to closed profiles!";
        }
      } else {
        qDebug()
            << "BRepBuilderAPI_MakeWire failed - edges might not be connected";
      }
    } catch (const std::exception &e) {
      qDebug() << "Wire building failed:" << e.what();
    } catch (...) {
      qDebug() << "Wire building failed with unknown error";
    }
  }

  qDebug() << "detectClosedProfiles: Returning" << closedWires.size()
           << "closed profiles";
  return closedWires;
}

std::vector<TopoDS_Face> Sketch::buildProfileFaces() const {
  std::vector<TopoDS_Face> faces;

  auto closedWires = detectClosedProfiles();
  for (const auto &wire : closedWires) {
    try {
      BRepBuilderAPI_MakeFace faceBuilder(wire, true);
      if (faceBuilder.IsDone()) {
        faces.push_back(faceBuilder.Face());
      }
    } catch (...) {
      continue;
    }
  }

  return faces;
}

int Sketch::closedProfileCount() const {
  return static_cast<int>(detectClosedProfiles().size());
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
