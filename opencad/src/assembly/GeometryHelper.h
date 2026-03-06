#pragma once

#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Circ.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Lin.hxx>
#include <gp_Pln.hxx>

namespace opencad {
namespace assembly {

class GeometryHelper {
public:
  static bool getPlane(const TopoDS_Shape &shape, gp_Pln &plane) {
    if (shape.ShapeType() != TopAbs_FACE)
      return false;
    BRepAdaptor_Surface surf(TopoDS::Face(shape));
    if (surf.GetType() != GeomAbs_Plane)
      return false;
    plane = surf.Plane();
    return true;
  }

  static bool getCylinder(const TopoDS_Shape &shape, gp_Cylinder &cylinder) {
    if (shape.ShapeType() != TopAbs_FACE)
      return false;
    BRepAdaptor_Surface surf(TopoDS::Face(shape));
    if (surf.GetType() != GeomAbs_Cylinder)
      return false;
    cylinder = surf.Cylinder();
    return true;
  }

  static bool getLine(const TopoDS_Shape &shape, gp_Lin &line) {
    if (shape.ShapeType() != TopAbs_EDGE)
      return false;
    BRepAdaptor_Curve curve(TopoDS::Edge(shape));
    if (curve.GetType() != GeomAbs_Line)
      return false;
    line = curve.Line();
    return true;
  }

  static bool getCircle(const TopoDS_Shape &shape, gp_Circ &circle) {
    if (shape.ShapeType() != TopAbs_EDGE)
      return false;
    BRepAdaptor_Curve curve(TopoDS::Edge(shape));
    if (curve.GetType() != GeomAbs_Circle)
      return false;
    circle = curve.Circle();
    return true;
  }

  // Get axis from various shapes (Cylinder, Cone, Circle, Line)
  static bool getAxis(const TopoDS_Shape &shape, gp_Ax1 &axis) {
    if (shape.ShapeType() == TopAbs_FACE) {
      BRepAdaptor_Surface surf(TopoDS::Face(shape));
      if (surf.GetType() == GeomAbs_Cylinder) {
        axis = surf.Cylinder().Axis();
        return true;
      } else if (surf.GetType() == GeomAbs_Cone) {
        axis = surf.Cone().Axis();
        return true;
      } else if (surf.GetType() == GeomAbs_Torus) {
        axis = surf.Torus().Axis();
        return true;
      }
    } else if (shape.ShapeType() == TopAbs_EDGE) {
      BRepAdaptor_Curve curve(TopoDS::Edge(shape));
      if (curve.GetType() == GeomAbs_Circle) {
        axis = curve.Circle().Axis();
        return true;
      } else if (curve.GetType() == GeomAbs_Line) {
        axis = curve.Line().Position();
        return true;
      }
    }
    return false;
  }
};

} // namespace assembly
} // namespace opencad
