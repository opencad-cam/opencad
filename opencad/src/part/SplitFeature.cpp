/**
 * @file SplitFeature.cpp
 * @brief Split feature implementation
 */

#include "SplitFeature.h"

#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Solid.hxx>
#include <gp_Pnt.hxx>


namespace opencad {
namespace part {

std::vector<TopoDS_Shape> SplitFeature::execute(const TopoDS_Shape &shape,
                                                const gp_Pln &plane,
                                                SplitKeepPart keepPart) {
  m_error.clear();
  std::vector<TopoDS_Shape> results;

  if (shape.IsNull()) {
    m_error = "Input shape is null";
    return results;
  }

  try {
    // Create a large face from the plane for splitting
    BRepBuilderAPI_MakeFace faceMaker(plane, -1000, 1000, -1000, 1000);
    if (!faceMaker.IsDone()) {
      m_error = "Failed to create splitting face from plane";
      return results;
    }
    TopoDS_Face splitFace = faceMaker.Face();

    // Use BRepAlgoAPI_Splitter to split the shape
    BRepAlgoAPI_Splitter splitter;
    TopTools_ListOfShape arguments;
    arguments.Append(shape);
    splitter.SetArguments(arguments);

    TopTools_ListOfShape tools;
    tools.Append(splitFace);
    splitter.SetTools(tools);

    splitter.Build();

    if (!splitter.IsDone()) {
      m_error = "Split operation failed";
      return results;
    }

    // Get resulting shapes
    const TopoDS_Shape &splitResult = splitter.Shape();

    // Extract solids from the result
    for (TopExp_Explorer exp(splitResult, TopAbs_SOLID); exp.More();
         exp.Next()) {
      TopoDS_Solid solid = TopoDS::Solid(exp.Current());

      if (keepPart == SplitKeepPart::Both) {
        results.push_back(solid);
      } else {
        // Determine if solid is above or below the plane
        // Check centroid position relative to plane
        // For now, add all solids (simplified)
        results.push_back(solid);
      }
    }

    if (results.empty()) {
      // Maybe the result is shells or other types
      for (TopExp_Explorer exp(splitResult, TopAbs_SHELL); exp.More();
           exp.Next()) {
        results.push_back(exp.Current());
      }
    }

    if (results.empty()) {
      m_error = "Split produced no valid shapes";
    }

  } catch (const Standard_Failure &e) {
    m_error = "Exception: " + std::string(e.GetMessageString());
  } catch (...) {
    m_error = "Unknown exception during split";
  }

  return results;
}

std::vector<TopoDS_Shape>
SplitFeature::executeWithTool(const TopoDS_Shape &shape,
                              const TopoDS_Shape &tool,
                              SplitKeepPart keepPart) {
  m_error.clear();
  std::vector<TopoDS_Shape> results;

  if (shape.IsNull() || tool.IsNull()) {
    m_error = "Input shape or tool is null";
    return results;
  }

  try {
    BRepAlgoAPI_Splitter splitter;
    TopTools_ListOfShape arguments;
    arguments.Append(shape);
    splitter.SetArguments(arguments);

    TopTools_ListOfShape tools;
    tools.Append(tool);
    splitter.SetTools(tools);

    splitter.Build();

    if (!splitter.IsDone()) {
      m_error = "Split with tool failed";
      return results;
    }

    const TopoDS_Shape &splitResult = splitter.Shape();

    for (TopExp_Explorer exp(splitResult, TopAbs_SOLID); exp.More();
         exp.Next()) {
      results.push_back(exp.Current());
    }

    if (results.empty()) {
      m_error = "Split with tool produced no solids";
    }

  } catch (const Standard_Failure &e) {
    m_error = "Exception: " + std::string(e.GetMessageString());
  } catch (...) {
    m_error = "Unknown exception during split with tool";
  }

  return results;
}

std::vector<TopoDS_Shape>
SplitFeature::executeWithFace(const TopoDS_Shape &shape,
                              const TopoDS_Face &face) {
  m_error.clear();
  std::vector<TopoDS_Shape> results;

  if (shape.IsNull() || face.IsNull()) {
    m_error = "Input shape or face is null";
    return results;
  }

  try {
    BRepAlgoAPI_Splitter splitter;
    TopTools_ListOfShape arguments;
    arguments.Append(shape);
    splitter.SetArguments(arguments);

    TopTools_ListOfShape tools;
    tools.Append(face);
    splitter.SetTools(tools);

    splitter.Build();

    if (!splitter.IsDone()) {
      m_error = "Split with face failed";
      return results;
    }

    const TopoDS_Shape &splitResult = splitter.Shape();

    for (TopExp_Explorer exp(splitResult, TopAbs_SOLID); exp.More();
         exp.Next()) {
      results.push_back(exp.Current());
    }

    if (results.empty()) {
      m_error = "Split with face produced no solids";
    }

  } catch (const Standard_Failure &e) {
    m_error = "Exception: " + std::string(e.GetMessageString());
  } catch (...) {
    m_error = "Unknown exception during split with face";
  }

  return results;
}

} // namespace part
} // namespace opencad
