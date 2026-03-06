/**
 * @file ShellFeature.cpp
 * @brief Shell feature implementation
 */

#include "ShellFeature.h"

#include <BRepCheck_Analyzer.hxx>
#include <BRepOffsetAPI_MakeThickSolid.hxx>
#include <BRepOffset_Mode.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <iostream>

namespace opencad {
namespace part {

TopoDS_Shape
ShellFeature::execute(const TopoDS_Shape &shape,
                      const std::vector<TopoDS_Face> &facesToRemove,
                      double thickness) {
  m_error.clear();

  if (shape.IsNull()) {
    m_error = "Input shape is null";
    return TopoDS_Shape();
  }

  if (facesToRemove.empty()) {
    m_error = "No faces selected to remove";
    return TopoDS_Shape();
  }

  if (thickness == 0) {
    m_error = "Thickness cannot be zero";
    return TopoDS_Shape();
  }

  std::cout << "ShellFeature: Execute called. Thickness=" << thickness
            << std::endl;

  // Check if shape is valid
  BRepCheck_Analyzer analyzer(shape);
  if (!analyzer.IsValid()) {
    std::cout << "ShellFeature: Input shape is invalid!" << std::endl;
  } else {
    std::cout << "ShellFeature: Input shape is valid." << std::endl;
  }

  try {
    // Build list of faces to remove with robust matching
    TopTools_ListOfShape faceList;
    std::cout << "ShellFeature: Faces to remove count: " << facesToRemove.size()
              << std::endl;

    for (const auto &selectedFace : facesToRemove) {
      if (selectedFace.IsNull()) {
        std::cout << "ShellFeature: Skipping null face." << std::endl;
        continue;
      }

      bool found = false;
      int faceCount = 0;
      TopExp_Explorer expl(shape, TopAbs_FACE);
      for (; expl.More(); expl.Next()) {
        faceCount++;
        const TopoDS_Face &shapeFace = TopoDS::Face(expl.Current());
        if (shapeFace.IsSame(selectedFace)) {
          faceList.Append(shapeFace);
          found = true;
          std::cout << "ShellFeature: Face MATCHED!" << std::endl;
          break;
        }
      }
      std::cout << "ShellFeature: Checked " << faceCount << " faces in shape."
                << std::endl;

      if (!found) {
        // Fallback: Just use the selected face and hope
        std::cout << "ShellFeature: Face NOT MATCHED in shape! Using selected "
                     "face directly."
                  << std::endl;
        faceList.Append(selectedFace);
      }
    }

    BRepOffsetAPI_MakeThickSolid shell;
    // Enable Intersection (true) and SelfInter (true) for better robustness
    std::cout << "ShellFeature: Calling MakeThickSolidByJoin..." << std::endl;
    shell.MakeThickSolidByJoin(shape, faceList, thickness, 1e-6,
                               BRepOffset_Skin, Standard_True, Standard_True);
    shell.Build();
    std::cout << "ShellFeature: Build complete. IsDone=" << shell.IsDone()
              << std::endl;

    if (shell.IsDone()) {
      return shell.Shape();
    } else {
      m_error = "Shell operation failed. Thickness may be too large.";
      return TopoDS_Shape();
    }
  } catch (const Standard_Failure &e) {
    m_error = "Exception: " + std::string(e.GetMessageString());
    return TopoDS_Shape();
  } catch (...) {
    m_error = "Unknown exception during shell";
    return TopoDS_Shape();
  }
}

} // namespace part
} // namespace opencad
