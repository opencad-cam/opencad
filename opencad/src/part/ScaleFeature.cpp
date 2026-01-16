/**
 * @file ScaleFeature.cpp
 * @brief Implementation of scale feature
 *
 * OpenCAD - Modular CAD/CAE Platform
 */

#include "ScaleFeature.h"

#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepGProp.hxx>
#include <GProp_GProps.hxx>
#include <gp_GTrsf.hxx>
#include <gp_Mat.hxx>
#include <gp_Trsf.hxx>


namespace opencad {
namespace part {

gp_Pnt ScaleFeature::calculateCentroid(const TopoDS_Shape &shape) const {
  GProp_GProps props;
  BRepGProp::VolumeProperties(shape, props);
  return props.CentreOfMass();
}

TopoDS_Shape ScaleFeature::execute(const TopoDS_Shape &shape, double factor) {
  m_error.clear();

  if (factor <= 0) {
    m_error = "Scale factor must be positive";
    return TopoDS_Shape();
  }

  try {
    // Calculate centroid
    gp_Pnt centroid = calculateCentroid(shape);
    return execute(shape, factor, centroid);
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  }
}

TopoDS_Shape ScaleFeature::execute(const TopoDS_Shape &shape, double factor,
                                   const gp_Pnt &point) {
  m_error.clear();

  if (factor <= 0) {
    m_error = "Scale factor must be positive";
    return TopoDS_Shape();
  }

  try {
    // Create scaling transformation
    gp_Trsf scaleTrsf;
    scaleTrsf.SetScale(point, factor);

    BRepBuilderAPI_Transform transform(shape, scaleTrsf, true);
    if (!transform.IsDone()) {
      m_error = "Scale transformation failed";
      return TopoDS_Shape();
    }

    return transform.Shape();
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  }
}

TopoDS_Shape ScaleFeature::execute(const TopoDS_Shape &shape,
                                   const ScaleParams &params) {
  m_error.clear();

  try {
    gp_Pnt scaleCenter;
    if (params.aboutCentroid) {
      scaleCenter = calculateCentroid(shape);
    } else {
      scaleCenter = gp_Pnt(0, 0, 0);
    }

    if (params.type == ScaleType::Uniform) {
      return execute(shape, params.scaleFactor, scaleCenter);
    } else {
      // Non-uniform scaling requires GTransform
      return executeNonUniform(shape, params.scaleX, params.scaleY,
                               params.scaleZ);
    }
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  }
}

TopoDS_Shape ScaleFeature::executeNonUniform(const TopoDS_Shape &shape,
                                             double scaleX, double scaleY,
                                             double scaleZ) {
  m_error.clear();

  if (scaleX <= 0 || scaleY <= 0 || scaleZ <= 0) {
    m_error = "All scale factors must be positive";
    return TopoDS_Shape();
  }

  try {
    // Calculate centroid for centering
    gp_Pnt centroid = calculateCentroid(shape);

    // First translate to origin
    gp_Trsf toOrigin;
    toOrigin.SetTranslation(gp_Vec(centroid, gp_Pnt(0, 0, 0)));

    // Create non-uniform scaling matrix
    gp_Mat scaleMat(scaleX, 0, 0, 0, scaleY, 0, 0, 0, scaleZ);

    gp_GTrsf gTrsf;
    gTrsf.SetVectorialPart(scaleMat);

    // Translate back
    gp_Trsf fromOrigin;
    fromOrigin.SetTranslation(gp_Vec(gp_Pnt(0, 0, 0), centroid));

    // Apply transformations in sequence
    BRepBuilderAPI_Transform toOriginTransform(shape, toOrigin, true);
    if (!toOriginTransform.IsDone()) {
      m_error = "Translation to origin failed";
      return TopoDS_Shape();
    }

    BRepBuilderAPI_GTransform scaleTransform(toOriginTransform.Shape(), gTrsf,
                                             true);
    if (!scaleTransform.IsDone()) {
      m_error = "Scale transformation failed";
      return TopoDS_Shape();
    }

    BRepBuilderAPI_Transform fromOriginTransform(scaleTransform.Shape(),
                                                 fromOrigin, true);
    if (!fromOriginTransform.IsDone()) {
      m_error = "Translation from origin failed";
      return TopoDS_Shape();
    }

    return fromOriginTransform.Shape();
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  }
}

} // namespace part
} // namespace opencad
