/**
 * @file ShellFeature.cpp
 * @brief Shell feature implementation
 */

#include "ShellFeature.h"

#include <BRepOffsetAPI_MakeThickSolid.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>

namespace opencad {
namespace part {

TopoDS_Shape ShellFeature::execute(const TopoDS_Shape& shape,
                                    const std::vector<TopoDS_Face>& facesToRemove,
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
    
    try {
        // Build list of faces to remove
        TopTools_ListOfShape faceList;
        for (const auto& face : facesToRemove) {
            if (!face.IsNull()) {
                faceList.Append(face);
            }
        }
        
        BRepOffsetAPI_MakeThickSolid shell;
        shell.MakeThickSolidByJoin(shape, faceList, thickness, 1e-6);
        shell.Build();
        
        if (shell.IsDone()) {
            return shell.Shape();
        } else {
            m_error = "Shell operation failed. Thickness may be too large.";
            return TopoDS_Shape();
        }
    } catch (const Standard_Failure& e) {
        m_error = "Exception: " + std::string(e.GetMessageString());
        return TopoDS_Shape();
    } catch (...) {
        m_error = "Unknown exception during shell";
        return TopoDS_Shape();
    }
}

} // namespace part
} // namespace opencad
