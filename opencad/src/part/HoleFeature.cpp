/**
 * @file HoleFeature.cpp
 * @brief Implementation of Hole Wizard feature
 *
 * OpenCAD - Modular CAD/CAE Platform
 */

#include "HoleFeature.h"

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <gp_Ax2.hxx>


#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace opencad {
namespace part {

TopoDS_Shape HoleFeature::createCylinder(const gp_Pnt &center,
                                         const gp_Dir &direction, double radius,
                                         double height) {
  try {
    gp_Ax2 axis(center, direction);
    BRepPrimAPI_MakeCylinder cylinder(axis, radius, height);
    if (cylinder.IsDone()) {
      return cylinder.Shape();
    }
  } catch (...) {
  }
  return TopoDS_Shape();
}

TopoDS_Shape HoleFeature::createCone(const gp_Pnt &center,
                                     const gp_Dir &direction, double topRadius,
                                     double bottomRadius, double height) {
  try {
    gp_Ax2 axis(center, direction);
    BRepPrimAPI_MakeCone cone(axis, bottomRadius, topRadius, height);
    if (cone.IsDone()) {
      return cone.Shape();
    }
  } catch (...) {
  }
  return TopoDS_Shape();
}

TopoDS_Shape HoleFeature::createDrillTip(const gp_Pnt &bottomCenter,
                                         const gp_Dir &direction, double radius,
                                         double tipAngle) {
  // Calculate cone height based on tip angle
  double halfAngleRad = (tipAngle / 2.0) * M_PI / 180.0;
  double height = radius / std::tan(halfAngleRad);

  // Create cone at the bottom of hole
  gp_Pnt tipPoint = bottomCenter.Translated(gp_Vec(direction) * (-height));
  return createCone(tipPoint, direction, 0.0, radius, height);
}

TopoDS_Shape HoleFeature::createSimpleHole(const TopoDS_Shape &base,
                                           const HolePosition &position,
                                           double diameter, double depth) {
  m_error.clear();

  try {
    double radius = diameter / 2.0;

    // Create cutting cylinder
    TopoDS_Shape cylinder =
        createCylinder(position.location, position.direction, radius, depth);
    if (cylinder.IsNull()) {
      m_error = "Failed to create hole cylinder";
      return base;
    }

    // Cut from base
    BRepAlgoAPI_Cut cut(base, cylinder);
    if (!cut.IsDone()) {
      m_error = "Failed to cut hole from base";
      return base;
    }

    return cut.Shape();
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return base;
  }
}

TopoDS_Shape HoleFeature::createCounterbore(const TopoDS_Shape &base,
                                            const HolePosition &position,
                                            const HoleParams &params) {
  m_error.clear();

  try {
    double mainRadius = params.diameter / 2.0;
    double cboreRadius = params.cboreDiameter / 2.0;

    // Create counterbore cylinder (larger, shallower)
    TopoDS_Shape cboreCylinder = createCylinder(
        position.location, position.direction, cboreRadius, params.cboreDepth);

    // Create main hole cylinder (extends from bottom of counterbore)
    gp_Pnt holeStart = position.location.Translated(gp_Vec(position.direction) *
                                                    params.cboreDepth);
    double holeDepth = params.depth - params.cboreDepth;

    TopoDS_Shape holeCylinder =
        createCylinder(holeStart, position.direction, mainRadius, holeDepth);

    if (cboreCylinder.IsNull() || holeCylinder.IsNull()) {
      m_error = "Failed to create counterbore geometry";
      return base;
    }

    // Combine the two cylinders
    BRepAlgoAPI_Fuse fuse(cboreCylinder, holeCylinder);
    if (!fuse.IsDone()) {
      m_error = "Failed to fuse counterbore geometry";
      return base;
    }

    // Cut from base
    BRepAlgoAPI_Cut cut(base, fuse.Shape());
    if (!cut.IsDone()) {
      m_error = "Failed to cut counterbore from base";
      return base;
    }

    return cut.Shape();
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return base;
  }
}

TopoDS_Shape HoleFeature::createCountersink(const TopoDS_Shape &base,
                                            const HolePosition &position,
                                            const HoleParams &params) {
  m_error.clear();

  try {
    double mainRadius = params.diameter / 2.0;
    double csinkRadius = params.csinkDiameter / 2.0;

    // Calculate countersink depth based on angle
    double halfAngleRad = (params.csinkAngle / 2.0) * M_PI / 180.0;
    double csinkDepth = (csinkRadius - mainRadius) / std::tan(halfAngleRad);

    // Create countersink cone
    TopoDS_Shape csinkCone = createCone(position.location, position.direction,
                                        csinkRadius, mainRadius, csinkDepth);

    // Create main hole cylinder (extends from bottom of countersink)
    gp_Pnt holeStart =
        position.location.Translated(gp_Vec(position.direction) * csinkDepth);
    double holeDepth = params.depth - csinkDepth;

    TopoDS_Shape holeCylinder =
        createCylinder(holeStart, position.direction, mainRadius, holeDepth);

    if (csinkCone.IsNull() || holeCylinder.IsNull()) {
      m_error = "Failed to create countersink geometry";
      return base;
    }

    // Combine the cone and cylinder
    BRepAlgoAPI_Fuse fuse(csinkCone, holeCylinder);
    if (!fuse.IsDone()) {
      m_error = "Failed to fuse countersink geometry";
      return base;
    }

    // Cut from base
    BRepAlgoAPI_Cut cut(base, fuse.Shape());
    if (!cut.IsDone()) {
      m_error = "Failed to cut countersink from base";
      return base;
    }

    return cut.Shape();
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return base;
  }
}

TopoDS_Shape HoleFeature::createTappedHole(const TopoDS_Shape &base,
                                           const HolePosition &position,
                                           const HoleParams &params) {
  m_error.clear();

  // For cosmetic threads, just create the tap drill hole
  // Actual thread geometry is complex and usually displayed cosmetically

  try {
    // Tap drill diameter is approximately (major diameter - pitch)
    double tapDrillDiameter = params.diameter - params.threadPitch;
    double radius = tapDrillDiameter / 2.0;

    TopoDS_Shape cylinder = createCylinder(
        position.location, position.direction, radius, params.threadDepth);

    if (cylinder.IsNull()) {
      m_error = "Failed to create tap drill hole";
      return base;
    }

    BRepAlgoAPI_Cut cut(base, cylinder);
    if (!cut.IsDone()) {
      m_error = "Failed to cut tap drill hole";
      return base;
    }

    return cut.Shape();

    // TODO: Add cosmetic thread representation or modeled helix for thread
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return base;
  }
}

TopoDS_Shape HoleFeature::execute(const TopoDS_Shape &base,
                                  const HolePosition &position,
                                  const HoleParams &params) {
  switch (params.type) {
  case HoleType::Simple:
  case HoleType::LegacyHole:
    return createSimpleHole(base, position, params.diameter, params.depth);

  case HoleType::Counterbore:
    return createCounterbore(base, position, params);

  case HoleType::Countersink:
    return createCountersink(base, position, params);

  case HoleType::TapDrill:
  case HoleType::TaperedTap:
    return createTappedHole(base, position, params);

  default:
    m_error = "Unsupported hole type";
    return base;
  }
}

TopoDS_Shape
HoleFeature::executeMultiple(const TopoDS_Shape &base,
                             const std::vector<HolePosition> &positions,
                             const HoleParams &params) {
  m_error.clear();

  if (positions.empty()) {
    m_error = "No hole positions specified";
    return base;
  }

  TopoDS_Shape result = base;
  for (const auto &position : positions) {
    result = execute(result, position, params);
    if (result.IsNull()) {
      return base; // Return original if any hole fails
    }
  }

  return result;
}

} // namespace part
} // namespace opencad
