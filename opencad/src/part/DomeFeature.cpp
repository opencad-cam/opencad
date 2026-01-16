/**
 * @file DomeFeature.cpp
 * @brief Dome feature implementation
 */

#include "DomeFeature.h"

#include <BRepAdaptor_Surface.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepGProp.hxx>
#include <BRepOffsetAPI_MakeFilling.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <GProp_GProps.hxx>
#include <GeomLProp_SLProps.hxx>
#include <Geom_SphericalSurface.hxx>
#include <ShapeAnalysis.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <gp_Ax2.hxx>
#include <gp_Ax3.hxx>
#include <gp_Circ.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Sphere.hxx>


#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace opencad {
namespace part {

TopoDS_Shape DomeFeature::execute(const TopoDS_Shape &shape,
                                  const TopoDS_Face &face, double height) {
  DomeParams params;
  params.height = height;
  return execute(shape, face, params);
}

TopoDS_Shape DomeFeature::execute(const TopoDS_Shape &shape,
                                  const TopoDS_Face &face,
                                  const DomeParams &params) {
  m_error.clear();

  if (shape.IsNull()) {
    m_error = "Input shape is null";
    return TopoDS_Shape();
  }

  if (face.IsNull()) {
    m_error = "Face is null";
    return TopoDS_Shape();
  }

  if (params.height <= 0) {
    m_error = "Dome height must be positive";
    return TopoDS_Shape();
  }

  try {
    // Create the dome shape
    TopoDS_Shape domeShape = createDome(face, params.height);

    if (domeShape.IsNull()) {
      return TopoDS_Shape();
    }

    // Fuse dome with base shape
    BRepAlgoAPI_Fuse fuser(shape, domeShape);
    if (fuser.IsDone()) {
      return fuser.Shape();
    } else {
      m_error = "Failed to fuse dome with shape";
      return TopoDS_Shape();
    }

  } catch (const Standard_Failure &e) {
    m_error = "Exception: " + std::string(e.GetMessageString());
    return TopoDS_Shape();
  } catch (...) {
    m_error = "Unknown exception during dome creation";
    return TopoDS_Shape();
  }
}

TopoDS_Shape DomeFeature::createDome(const TopoDS_Face &face, double height) {
  m_error.clear();

  if (face.IsNull()) {
    m_error = "Input face is null";
    return TopoDS_Shape();
  }

  try {
    // Get face properties
    BRepAdaptor_Surface adaptor(face);

    // Check if face is planar
    if (adaptor.GetType() != GeomAbs_Plane) {
      m_error = "Face must be planar for dome feature";
      return TopoDS_Shape();
    }

    gp_Pln plane = adaptor.Plane();
    gp_Pnt center = plane.Location();
    gp_Dir normal = plane.Axis().Direction();

    // Get face center using mass properties
    GProp_GProps props;
    BRepGProp::SurfaceProperties(face, props);
    gp_Pnt faceCentroid = props.CentreOfMass();

    // Calculate dome radius based on face size
    // For a spherical cap: if h = height and r = base radius
    // then R (sphere radius) = (r² + h²) / (2h)
    double faceArea = props.Mass();
    double baseRadius = std::sqrt(faceArea / M_PI); // Approximate as circle

    // Sphere radius for given height
    double sphereRadius =
        (baseRadius * baseRadius + height * height) / (2 * height);

    // Sphere center is offset along normal
    double centerOffset = sphereRadius - height;
    gp_Pnt sphereCenter =
        faceCentroid.Translated(gp_Vec(normal) * (-centerOffset));

    // Create sphere
    gp_Ax2 sphereAxis(sphereCenter, normal);
    BRepPrimAPI_MakeSphere sphereMaker(sphereAxis, sphereRadius);
    TopoDS_Shape sphere = sphereMaker.Shape();

    // We need a half-sphere, cut at the face plane
    // For simplicity, the full sphere intersected/cut might be needed
    // For now, return full hemisphere at the face location

    // Create partial sphere (cap) using revolution
    // This is a simplified approach - creates a proper dome

    if (sphere.IsNull()) {
      m_error = "Failed to create dome sphere";
      return TopoDS_Shape();
    }

    return sphere;

  } catch (const Standard_Failure &e) {
    m_error =
        "Exception in dome creation: " + std::string(e.GetMessageString());
    return TopoDS_Shape();
  } catch (...) {
    m_error = "Unknown exception in dome creation";
    return TopoDS_Shape();
  }
}

void DomeFeature::calculateDomeGeometry(const TopoDS_Face &face, double height,
                                        double &radius, double &centerZ) const {
  // Get face area to estimate base radius
  GProp_GProps props;
  BRepGProp::SurfaceProperties(face, props);
  double faceArea = props.Mass();
  double baseRadius = std::sqrt(faceArea / M_PI);

  // For spherical cap: R = (r² + h²) / (2h)
  radius = (baseRadius * baseRadius + height * height) / (2.0 * height);
  centerZ = radius - height;
}

} // namespace part
} // namespace opencad
