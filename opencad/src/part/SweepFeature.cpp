/**
 * @file SweepFeature.cpp
 * @brief Sweep feature implementation
 */

#include "SweepFeature.h"

#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <BRepTools.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>


namespace opencad {
namespace part {

TopoDS_Shape SweepFeature::execute(const TopoDS_Shape &profile,
                                   const TopoDS_Wire &path, bool closedPath) {
  m_error.clear();

  if (profile.IsNull()) {
    m_error = "Profile is null";
    return TopoDS_Shape();
  }

  if (path.IsNull()) {
    m_error = "Path is null";
    return TopoDS_Shape();
  }

  try {
    // If profile is a wire, make it a face first
    TopoDS_Shape profileShape = profile;
    if (profile.ShapeType() == TopAbs_WIRE) {
      BRepBuilderAPI_MakeFace faceMaker(TopoDS::Wire(profile));
      if (faceMaker.IsDone()) {
        profileShape = faceMaker.Face();
      }
    }

    if (closedPath) {
      // Use MakePipeShell for closed path sweep
      BRepOffsetAPI_MakePipeShell pipeShell(path);

      // Add profile
      if (profileShape.ShapeType() == TopAbs_WIRE) {
        pipeShell.Add(TopoDS::Wire(profileShape));
      } else if (profileShape.ShapeType() == TopAbs_FACE) {
        TopoDS_Wire wire = BRepTools::OuterWire(TopoDS::Face(profileShape));
        pipeShell.Add(wire);
      }

      pipeShell.Build();

      if (pipeShell.IsDone()) {
        pipeShell.MakeSolid();
        return pipeShell.Shape();
      } else {
        m_error = "Closed path sweep failed";
        return TopoDS_Shape();
      }
    } else {
      // Standard pipe for open path
      BRepOffsetAPI_MakePipe pipe(path, profileShape);
      pipe.Build();

      if (pipe.IsDone()) {
        return pipe.Shape();
      } else {
        m_error = "Sweep operation failed";
        return TopoDS_Shape();
      }
    }
  } catch (const Standard_Failure &e) {
    m_error = "Exception: " + std::string(e.GetMessageString());
    return TopoDS_Shape();
  } catch (...) {
    m_error = "Unknown exception during sweep";
    return TopoDS_Shape();
  }
}

TopoDS_Shape SweepFeature::executeWithGuide(const TopoDS_Shape &profile,
                                            const TopoDS_Wire &path,
                                            const TopoDS_Wire &guide) {
  m_error.clear();

  if (profile.IsNull() || path.IsNull() || guide.IsNull()) {
    m_error = "Invalid input";
    return TopoDS_Shape();
  }

  try {
    BRepOffsetAPI_MakePipeShell pipeShell(path);

    // Add profile
    if (profile.ShapeType() == TopAbs_WIRE) {
      pipeShell.Add(TopoDS::Wire(profile));
    } else if (profile.ShapeType() == TopAbs_FACE) {
      // Get outer wire from face
      TopoDS_Wire wire = BRepTools::OuterWire(TopoDS::Face(profile));
      pipeShell.Add(wire);
    }

    // Set auxiliary spine (guide curve)
    pipeShell.SetMode(guide, false);

    pipeShell.Build();
    pipeShell.MakeSolid();

    if (pipeShell.IsDone()) {
      return pipeShell.Shape();
    } else {
      m_error = "Guided sweep failed";
      return TopoDS_Shape();
    }
  } catch (...) {
    m_error = "Exception during guided sweep";
    return TopoDS_Shape();
  }
}

} // namespace part
} // namespace opencad
