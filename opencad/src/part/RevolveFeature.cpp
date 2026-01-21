/**
 * @file RevolveFeature.cpp
 * @brief Implementation of revolve feature
 *
 * OpenCAD - Modular CAD/CAE Platform
 */

#include "RevolveFeature.h"
#include "../sketch/Sketch.h"

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <TopoDS.hxx>
#include <gp_Ax1.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>


#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace opencad {
namespace part {

double RevolveFeature::toRadians(double degrees) const {
  return degrees * M_PI / 180.0;
}

gp_Ax1 RevolveFeature::getAxisFromSketch(const sketch::Sketch &sketch) const {
  // Get the sketch plane
  const auto &plane = sketch.plane();
  gp_Pnt origin = plane.origin();
  gp_Dir normal = plane.normal();

  // Default axis is along Y direction of the sketch plane
  // For XY plane, this means revolving around Y axis
  gp_Dir yDir = plane.yDirection();

  switch (m_axisType) {
  case RevolveAxisType::XAxis:
    return gp_Ax1(origin, gp_Dir(1, 0, 0));
  case RevolveAxisType::YAxis:
    return gp_Ax1(origin, gp_Dir(0, 1, 0));
  case RevolveAxisType::ZAxis:
    return gp_Ax1(origin, gp_Dir(0, 0, 1));
  case RevolveAxisType::SketchLine:
    // Use Y direction of sketch plane as axis
    return gp_Ax1(origin, yDir);
  case RevolveAxisType::CustomAxis:
    return m_customAxis;
  default:
    return gp_Ax1(origin, gp_Dir(0, 1, 0));
  }
}

TopoDS_Shape RevolveFeature::execute(const sketch::Sketch &sketch,
                                     double angleDeg, bool symmetric) {
  RevolveParams params;
  params.angle = angleDeg;
  params.symmetric = symmetric;
  return execute(sketch, params);
}

TopoDS_Shape RevolveFeature::execute(const sketch::Sketch &sketch,
                                     const RevolveParams &params) {
  m_error.clear();

  try {
    // Build face from sketch
    // Use sketch plane for robust face creation
    const gp_Pln& plane = sketch.plane().plane();
    TopoDS_Wire wire = sketch.buildWire();

    if (wire.IsNull()) {
      m_error = "Failed to build wire from sketch";
      return TopoDS_Shape();
    }

    // Try building face using the sketch plane
    TopoDS_Face face;
    try {
      BRepBuilderAPI_MakeFace makeFace(plane, wire, true);
      if (makeFace.IsDone()) {
        face = makeFace.Face();
      } else {
        // Fallback: Try without plane (legacy behavior)
        BRepBuilderAPI_MakeFace makeFaceLegacy(wire, true);
        if (makeFaceLegacy.IsDone()) {
          face = makeFaceLegacy.Face();
        }
      }
    } catch (...) {
       m_error = "Exception during face creation";
       return TopoDS_Shape();
    }

    if (face.IsNull()) {
        m_error = "Failed to build face from wire";
        return TopoDS_Shape();
    }

    // Get rotation axis
    gp_Ax1 axis = getAxisFromSketch(sketch);

    // Calculate angle
    double angle = params.angle;
    if (params.reversed) {
      angle = -angle;
    }

    if (params.symmetric) {
      // For symmetric, we need to do two revolves and fuse them
      double halfAngle = angle / 2.0;

      BRepPrimAPI_MakeRevol revolve1(face, axis, toRadians(halfAngle), true);
      if (!revolve1.IsDone()) {
        m_error = "Failed to create first symmetric revolve";
        return TopoDS_Shape();
      }

      // Revolve in opposite direction
      BRepPrimAPI_MakeRevol revolve2(face, axis, toRadians(-halfAngle), true);
      if (!revolve2.IsDone()) {
        m_error = "Failed to create second symmetric revolve";
        return TopoDS_Shape();
      }

      // Fuse the two halves
      BRepAlgoAPI_Fuse fuse(revolve1.Shape(), revolve2.Shape());
      if (!fuse.IsDone()) {
        m_error = "Failed to fuse symmetric revolve halves";
        return TopoDS_Shape();
      }

      return fuse.Shape();
    } else {
      // Simple single-direction revolve
      BRepPrimAPI_MakeRevol revolve(face, axis, toRadians(angle), true);
      if (!revolve.IsDone()) {
        m_error = "Failed to create revolve";
        return TopoDS_Shape();
      }

      return revolve.Shape();
    }
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  } catch (const std::exception &e) {
    m_error = "Error: ";
    m_error += e.what();
    return TopoDS_Shape();
  }
}

TopoDS_Shape RevolveFeature::execute(const sketch::Sketch &sketch,
                                     const gp_Ax1 &axis, double angleDeg) {
  m_error.clear();

  try {
    // Use sketch plane for robust face creation
    const gp_Pln& plane = sketch.plane().plane();
    TopoDS_Wire wire = sketch.buildWire();

    if (wire.IsNull()) {
      m_error = "Failed to build wire from sketch";
      return TopoDS_Shape();
    }

    TopoDS_Face face;
    BRepBuilderAPI_MakeFace makeFace(plane, wire, true);
    if (makeFace.IsDone()) {
      face = makeFace.Face();
    } else {
        m_error = "Failed to build face from wire";
        return TopoDS_Shape();
    }

    BRepPrimAPI_MakeRevol revolve(face, axis, toRadians(angleDeg), true);
    if (!revolve.IsDone()) {
      m_error = "Failed to create revolve";
      return TopoDS_Shape();
    }

    return revolve.Shape();
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  }
}

TopoDS_Shape RevolveFeature::executeWire(const TopoDS_Wire &profile,
                                         const gp_Ax1 &axis, double angleDeg) {
  m_error.clear();

  try {
    BRepPrimAPI_MakeRevol revolve(profile, axis, toRadians(angleDeg), true);
    if (!revolve.IsDone()) {
      m_error = "Failed to create revolve from wire";
      return TopoDS_Shape();
    }
    return revolve.Shape();
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  }
}

TopoDS_Shape RevolveFeature::executeFace(const TopoDS_Face &profile,
                                         const gp_Ax1 &axis, double angleDeg) {
  m_error.clear();

  try {
    BRepPrimAPI_MakeRevol revolve(profile, axis, toRadians(angleDeg), true);
    if (!revolve.IsDone()) {
      m_error = "Failed to create revolve from face";
      return TopoDS_Shape();
    }
    return revolve.Shape();
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  }
}

TopoDS_Shape RevolveFeature::addTo(const sketch::Sketch &sketch,
                                   const TopoDS_Shape &base, double angleDeg) {
  m_error.clear();

  try {
    TopoDS_Shape revolveShape = execute(sketch, angleDeg, false);
    if (revolveShape.IsNull()) {
      return TopoDS_Shape();
    }

    BRepAlgoAPI_Fuse fuse(base, revolveShape);
    if (!fuse.IsDone()) {
      m_error = "Failed to fuse revolve with base";
      return TopoDS_Shape();
    }

    return fuse.Shape();
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  }
}

TopoDS_Shape RevolveFeature::cutFrom(const sketch::Sketch &sketch,
                                     const TopoDS_Shape &base,
                                     double angleDeg) {
  m_error.clear();

  try {
    TopoDS_Shape revolveShape = execute(sketch, angleDeg, false);
    if (revolveShape.IsNull()) {
      return TopoDS_Shape();
    }

    BRepAlgoAPI_Cut cut(base, revolveShape);
    if (!cut.IsDone()) {
      m_error = "Failed to cut revolve from base";
      return TopoDS_Shape();
    }

    return cut.Shape();
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  }
}

} // namespace part
} // namespace opencad
