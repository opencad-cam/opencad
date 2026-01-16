/**
 * @file SketchSlot.cpp
 * @brief Implementation of slot entity
 *
 * OpenCAD - Modular CAD/CAE Platform
 */

#include "SketchSlot.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <Geom2d_Line.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <TopoDS_Edge.hxx>
#include <cmath>
#include <gp_Circ.hxx>
#include <gp_Pln.hxx>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace opencad {
namespace sketch {

SketchSlot::SketchSlot(const gp_Pnt2d &center1, const gp_Pnt2d &center2,
                       double width)
    : SketchEntity(), m_center1(center1), m_center2(center2), m_width(width),
      m_slotType(SlotType::Straight) {}

SketchSlot::SketchSlot(const gp_Pnt2d &center, double length, double width,
                       double angle)
    : SketchEntity(), m_width(width), m_slotType(SlotType::Centerpoint) {
  // Calculate center1 and center2 from center, length, and angle
  double halfLen =
      (length - width) / 2.0; // Distance from center to arc centers
  double dx = halfLen * std::cos(angle);
  double dy = halfLen * std::sin(angle);

  m_center1 = gp_Pnt2d(center.X() - dx, center.Y() - dy);
  m_center2 = gp_Pnt2d(center.X() + dx, center.Y() + dy);
}

double SketchSlot::length() const {
  double dx = m_center2.X() - m_center1.X();
  double dy = m_center2.Y() - m_center1.Y();
  return std::sqrt(dx * dx + dy * dy);
}

double SketchSlot::slotLength() const { return length() + m_width; }

double SketchSlot::angle() const {
  return std::atan2(m_center2.Y() - m_center1.Y(),
                    m_center2.X() - m_center1.X());
}

gp_Pnt2d SketchSlot::center() const {
  return gp_Pnt2d((m_center1.X() + m_center2.X()) / 2.0,
                  (m_center1.Y() + m_center2.Y()) / 2.0);
}

Handle(Geom2d_Curve) SketchSlot::curve() const {
  // Return centerline as curve
  gp_Pnt2d mid = center();
  gp_Dir2d dir(m_center2.X() - m_center1.X(), m_center2.Y() - m_center1.Y());
  return new Geom2d_Line(mid, dir);
}

double SketchSlot::getParameter(int index) const {
  switch (index) {
  case 0:
    return m_center1.X();
  case 1:
    return m_center1.Y();
  case 2:
    return m_center2.X();
  case 3:
    return m_center2.Y();
  case 4:
    return m_width;
  default:
    return 0.0;
  }
}

void SketchSlot::setParameter(int index, double value) {
  switch (index) {
  case 0:
    m_center1.SetX(value);
    break;
  case 1:
    m_center1.SetY(value);
    break;
  case 2:
    m_center2.SetX(value);
    break;
  case 3:
    m_center2.SetY(value);
    break;
  case 4:
    m_width = value;
    break;
  }
}

SketchEntity::Ptr SketchSlot::clone() const {
  auto cloned = std::make_shared<SketchSlot>(m_center1, m_center2, m_width);
  cloned->setId(id());
  cloned->setConstruction(isConstruction());
  cloned->setSlotType(m_slotType);
  return cloned;
}

std::vector<gp_Pnt2d> SketchSlot::cornerPoints() const {
  std::vector<gp_Pnt2d> points;

  double slotAngle = angle();
  double perpAngle = slotAngle + M_PI / 2.0;
  double radius = m_width / 2.0;

  double dx = radius * std::cos(perpAngle);
  double dy = radius * std::sin(perpAngle);

  // Four corner points (where lines meet arcs)
  points.push_back(gp_Pnt2d(m_center1.X() + dx, m_center1.Y() + dy));
  points.push_back(gp_Pnt2d(m_center2.X() + dx, m_center2.Y() + dy));
  points.push_back(gp_Pnt2d(m_center2.X() - dx, m_center2.Y() - dy));
  points.push_back(gp_Pnt2d(m_center1.X() - dx, m_center1.Y() - dy));

  return points;
}

TopoDS_Wire SketchSlot::buildWire() const {
  try {
    BRepBuilderAPI_MakeWire wireBuilder;

    double slotAngle = angle();
    double perpAngle = slotAngle + M_PI / 2.0;
    double radius = m_width / 2.0;

    double dx = radius * std::cos(perpAngle);
    double dy = radius * std::sin(perpAngle);

    // Corner points
    gp_Pnt p1(m_center1.X() + dx, m_center1.Y() + dy, 0);
    gp_Pnt p2(m_center2.X() + dx, m_center2.Y() + dy, 0);
    gp_Pnt p3(m_center2.X() - dx, m_center2.Y() - dy, 0);
    gp_Pnt p4(m_center1.X() - dx, m_center1.Y() - dy, 0);

    // 3D centers for arcs
    gp_Pnt c1(m_center1.X(), m_center1.Y(), 0);
    gp_Pnt c2(m_center2.X(), m_center2.Y(), 0);

    // Top line (p1 to p2)
    BRepBuilderAPI_MakeEdge topEdge(p1, p2);
    if (topEdge.IsDone()) {
      wireBuilder.Add(topEdge.Edge());
    }

    // Right arc (around center2, from p2 to p3)
    gp_Circ circle2(gp_Ax2(c2, gp_Dir(0, 0, 1)), radius);
    Handle(Geom_TrimmedCurve) arc2 = GC_MakeArcOfCircle(circle2, p2, p3, true);
    if (!arc2.IsNull()) {
      BRepBuilderAPI_MakeEdge arc2Edge(arc2);
      if (arc2Edge.IsDone()) {
        wireBuilder.Add(arc2Edge.Edge());
      }
    }

    // Bottom line (p3 to p4)
    BRepBuilderAPI_MakeEdge bottomEdge(p3, p4);
    if (bottomEdge.IsDone()) {
      wireBuilder.Add(bottomEdge.Edge());
    }

    // Left arc (around center1, from p4 to p1)
    gp_Circ circle1(gp_Ax2(c1, gp_Dir(0, 0, 1)), radius);
    Handle(Geom_TrimmedCurve) arc1 = GC_MakeArcOfCircle(circle1, p4, p1, true);
    if (!arc1.IsNull()) {
      BRepBuilderAPI_MakeEdge arc1Edge(arc1);
      if (arc1Edge.IsDone()) {
        wireBuilder.Add(arc1Edge.Edge());
      }
    }

    if (wireBuilder.IsDone()) {
      return wireBuilder.Wire();
    }
  } catch (...) {
  }

  return TopoDS_Wire();
}

TopoDS_Shape SketchSlot::toShape() const { return buildWire(); }

} // namespace sketch
} // namespace opencad
