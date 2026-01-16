/**
 * @file DraftFeature.cpp
 * @brief Implementation of draft feature
 *
 * OpenCAD - Modular CAD/CAE Platform
 */

#include "DraftFeature.h"

#include <BRepAdaptor_Surface.hxx>
#include <BRepOffsetAPI_DraftAngle.hxx>
#include <BRep_Tool.hxx>
#include <GeomAdaptor_Surface.hxx>
#include <Geom_Surface.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>


#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace opencad {
namespace part {

std::vector<TopoDS_Face>
DraftFeature::findDraftableFaces(const TopoDS_Shape &shape,
                                 const gp_Dir &direction) const {
  std::vector<TopoDS_Face> draftableFaces;

  TopExp_Explorer explorer(shape, TopAbs_FACE);
  while (explorer.More()) {
    TopoDS_Face face = TopoDS::Face(explorer.Current());

    // Check if face is roughly parallel to the draft direction
    BRepAdaptor_Surface adaptor(face);

    // Get face normal at center
    double u = (adaptor.FirstUParameter() + adaptor.LastUParameter()) / 2.0;
    double v = (adaptor.FirstVParameter() + adaptor.LastVParameter()) / 2.0;

    gp_Pnt pnt;
    gp_Vec du, dv;
    adaptor.D1(u, v, pnt, du, dv);

    gp_Vec normal = du.Crossed(dv);
    if (normal.Magnitude() > 1e-10) {
      normal.Normalize();

      // Face is draftable if its normal is roughly perpendicular to pull
      // direction
      double dot = std::abs(normal.Dot(gp_Vec(direction)));
      if (dot < 0.9) { // Not parallel to pull direction
        draftableFaces.push_back(face);
      }
    }

    explorer.Next();
  }

  return draftableFaces;
}

TopoDS_Shape DraftFeature::execute(const TopoDS_Shape &shape,
                                   const std::vector<TopoDS_Face> &faces,
                                   const gp_Pln &neutralPlane,
                                   const gp_Dir &direction, double angleDeg) {
  m_error.clear();

  if (faces.empty()) {
    m_error = "No faces selected for draft";
    return shape;
  }

  try {
    BRepOffsetAPI_DraftAngle draftMaker(shape);

    double angleRad = angleDeg * M_PI / 180.0;

    for (const auto &face : faces) {
      draftMaker.Add(face, direction, angleRad, neutralPlane);
    }

    draftMaker.Build();

    if (!draftMaker.IsDone()) {
      m_error = "Draft operation failed";
      return shape;
    }

    return draftMaker.Shape();
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return shape;
  } catch (const std::exception &e) {
    m_error = "Error: ";
    m_error += e.what();
    return shape;
  }
}

TopoDS_Shape DraftFeature::executeAll(const TopoDS_Shape &shape,
                                      const gp_Dir &direction,
                                      const gp_Pln &neutralPlane,
                                      double angleDeg) {
  m_error.clear();

  try {
    // Find all draftable faces
    std::vector<TopoDS_Face> faces = findDraftableFaces(shape, direction);

    if (faces.empty()) {
      m_error = "No draftable faces found";
      return shape;
    }

    return execute(shape, faces, neutralPlane, direction, angleDeg);
  } catch (const std::exception &e) {
    m_error = "Error: ";
    m_error += e.what();
    return shape;
  }
}

TopoDS_Shape DraftFeature::execute(const TopoDS_Shape &shape,
                                   const std::vector<TopoDS_Face> &faces,
                                   const gp_Dir &direction,
                                   const DraftParams &params) {
  m_error.clear();

  // Create neutral plane at origin perpendicular to direction
  gp_Pnt origin(0, 0, 0);
  gp_Pln neutralPlane(origin, direction);

  double angle = params.angle;
  if (params.reversed) {
    angle = -angle;
  }

  return execute(shape, faces, neutralPlane, direction, angle);
}

} // namespace part
} // namespace opencad
