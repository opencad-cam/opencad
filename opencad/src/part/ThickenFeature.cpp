/**
 * @file ThickenFeature.cpp
 * @brief Implementation of thicken feature
 *
 * OpenCAD - Modular CAD/CAE Platform
 */

#include "ThickenFeature.h"

#include <BRepAdaptor_Surface.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepOffsetAPI_MakeThickSolid.hxx>
#include <BRepOffset_MakeOffset.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <Geom_Surface.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <gp_Vec.hxx>


namespace opencad {
namespace part {

TopoDS_Shape ThickenFeature::execute(const TopoDS_Face &face, double thickness,
                                     ThickenDirection direction) {
  m_error.clear();

  try {
    // Get the face normal at center
    BRepAdaptor_Surface adaptor(face);

    double uMid = (adaptor.FirstUParameter() + adaptor.LastUParameter()) / 2.0;
    double vMid = (adaptor.FirstVParameter() + adaptor.LastVParameter()) / 2.0;

    gp_Pnt pnt;
    gp_Vec du, dv;
    adaptor.D1(uMid, vMid, pnt, du, dv);

    gp_Vec normal = du.Crossed(dv);
    if (normal.Magnitude() < 1e-10) {
      m_error = "Failed to compute face normal";
      return TopoDS_Shape();
    }
    normal.Normalize();

    // Calculate thicken vectors based on direction
    gp_Vec thickenVec;
    switch (direction) {
    case ThickenDirection::Normal:
      thickenVec = normal * thickness;
      break;
    case ThickenDirection::Reverse:
      thickenVec = normal * (-thickness);
      break;
    case ThickenDirection::Both:
      // For both directions, we need to create two prisms and fuse
      {
        gp_Vec vec1 = normal * (thickness / 2.0);
        gp_Vec vec2 = normal * (-thickness / 2.0);

        BRepPrimAPI_MakePrism prism1(face, vec1);
        if (!prism1.IsDone()) {
          m_error = "Failed to create first thicken prism";
          return TopoDS_Shape();
        }

        BRepPrimAPI_MakePrism prism2(face, vec2);
        if (!prism2.IsDone()) {
          m_error = "Failed to create second thicken prism";
          return TopoDS_Shape();
        }

        BRepAlgoAPI_Fuse fuse(prism1.Shape(), prism2.Shape());
        if (!fuse.IsDone()) {
          m_error = "Failed to fuse thicken halves";
          return TopoDS_Shape();
        }

        return fuse.Shape();
      }
    }

    // Create prism (extrude face along normal)
    BRepPrimAPI_MakePrism prism(face, thickenVec);
    if (!prism.IsDone()) {
      m_error = "Failed to create thicken prism";
      return TopoDS_Shape();
    }

    return prism.Shape();
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  }
}

TopoDS_Shape ThickenFeature::execute(const std::vector<TopoDS_Face> &faces,
                                     double thickness) {
  m_error.clear();

  if (faces.empty()) {
    m_error = "No faces provided";
    return TopoDS_Shape();
  }

  try {
    TopoDS_Shape result;
    bool first = true;

    for (const auto &face : faces) {
      TopoDS_Shape thickened =
          execute(face, thickness, ThickenDirection::Normal);
      if (thickened.IsNull())
        continue;

      if (first) {
        result = thickened;
        first = false;
      } else {
        BRepAlgoAPI_Fuse fuse(result, thickened);
        if (fuse.IsDone()) {
          result = fuse.Shape();
        }
      }
    }

    return result;
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  }
}

TopoDS_Shape ThickenFeature::execute(const TopoDS_Shape &shape,
                                     double thickness) {
  m_error.clear();

  try {
    // Collect all faces from the shape
    std::vector<TopoDS_Face> faces;
    TopExp_Explorer explorer(shape, TopAbs_FACE);
    while (explorer.More()) {
      faces.push_back(TopoDS::Face(explorer.Current()));
      explorer.Next();
    }

    if (faces.empty()) {
      m_error = "No faces found in shape";
      return TopoDS_Shape();
    }

    return execute(faces, thickness);
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  }
}

TopoDS_Shape ThickenFeature::execute(const TopoDS_Shape &shape,
                                     const ThickenParams &params) {
  m_error.clear();

  try {
    if (params.direction == ThickenDirection::Both) {
      // Use thickness1 for normal, thickness2 for reverse
      std::vector<TopoDS_Face> faces;
      TopExp_Explorer explorer(shape, TopAbs_FACE);
      while (explorer.More()) {
        faces.push_back(TopoDS::Face(explorer.Current()));
        explorer.Next();
      }

      TopoDS_Shape result;
      bool first = true;

      for (const auto &face : faces) {
        // Thicken in both directions with different thicknesses
        TopoDS_Shape thick1 =
            execute(face, params.thickness1, ThickenDirection::Normal);
        TopoDS_Shape thick2 =
            execute(face, params.thickness2, ThickenDirection::Reverse);

        TopoDS_Shape combined;
        if (!thick1.IsNull() && !thick2.IsNull()) {
          BRepAlgoAPI_Fuse fuse(thick1, thick2);
          if (fuse.IsDone()) {
            combined = fuse.Shape();
          }
        } else if (!thick1.IsNull()) {
          combined = thick1;
        } else if (!thick2.IsNull()) {
          combined = thick2;
        }

        if (!combined.IsNull()) {
          if (first) {
            result = combined;
            first = false;
          } else {
            BRepAlgoAPI_Fuse fuse(result, combined);
            if (fuse.IsDone()) {
              result = fuse.Shape();
            }
          }
        }
      }

      return result;
    } else {
      // Simple single-direction thicken
      std::vector<TopoDS_Face> faces;
      TopExp_Explorer explorer(shape, TopAbs_FACE);
      while (explorer.More()) {
        faces.push_back(TopoDS::Face(explorer.Current()));
        explorer.Next();
      }

      TopoDS_Shape result;
      bool first = true;

      for (const auto &face : faces) {
        TopoDS_Shape thickened =
            execute(face, params.thickness1, params.direction);
        if (thickened.IsNull())
          continue;

        if (first) {
          result = thickened;
          first = false;
        } else {
          BRepAlgoAPI_Fuse fuse(result, thickened);
          if (fuse.IsDone()) {
            result = fuse.Shape();
          }
        }
      }

      return result;
    }
  } catch (const Standard_Failure &e) {
    m_error = "OpenCASCADE error: ";
    m_error += e.GetMessageString();
    return TopoDS_Shape();
  }
}

} // namespace part
} // namespace opencad
