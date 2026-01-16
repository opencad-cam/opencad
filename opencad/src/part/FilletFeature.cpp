/**
 * @file FilletFeature.cpp
 * @brief Fillet feature implementation using OpenCASCADE
 */

#include "FilletFeature.h"

#include <BRepFilletAPI_MakeFillet.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TColgp_Array1OfPnt2d.hxx>
#include <gp_Pnt2d.hxx>

namespace opencad {
namespace part {

TopoDS_Shape FilletFeature::execute(const TopoDS_Shape& shape,
                                     const std::vector<TopoDS_Edge>& edges,
                                     double radius) {
    m_error.clear();
    
    if (shape.IsNull()) {
        m_error = "Input shape is null";
        return TopoDS_Shape();
    }
    
    if (edges.empty()) {
        m_error = "No edges selected for fillet";
        return TopoDS_Shape();
    }
    
    if (radius <= 0) {
        m_error = "Radius must be positive";
        return TopoDS_Shape();
    }
    
    try {
        BRepFilletAPI_MakeFillet fillet(shape);
        
        // Add all selected edges with the same radius
        for (const auto& edge : edges) {
            if (!edge.IsNull()) {
                fillet.Add(radius, edge);
            }
        }
        
        fillet.Build();
        
        if (fillet.IsDone()) {
            return fillet.Shape();
        } else {
            m_error = "Fillet operation failed. Radius may be too large for the geometry.";
            return TopoDS_Shape();
        }
    } catch (const Standard_Failure& e) {
        m_error = "Exception: " + std::string(e.GetMessageString());
        return TopoDS_Shape();
    } catch (...) {
        m_error = "Unknown exception during fillet";
        return TopoDS_Shape();
    }
}

TopoDS_Shape FilletFeature::executeVariable(const TopoDS_Shape& shape,
                                             const TopoDS_Edge& edge,
                                             double radius1,
                                             double radius2) {
    m_error.clear();
    
    if (shape.IsNull() || edge.IsNull()) {
        m_error = "Invalid input";
        return TopoDS_Shape();
    }
    
    try {
        BRepFilletAPI_MakeFillet fillet(shape);
        
        // Variable radius fillet
        TColgp_Array1OfPnt2d radii(1, 2);
        radii.SetValue(1, gp_Pnt2d(0.0, radius1));  // Start parameter, radius
        radii.SetValue(2, gp_Pnt2d(1.0, radius2));  // End parameter, radius
        
        fillet.Add(radii, edge);
        fillet.Build();
        
        if (fillet.IsDone()) {
            return fillet.Shape();
        } else {
            m_error = "Variable fillet failed";
            return TopoDS_Shape();
        }
    } catch (...) {
        m_error = "Exception during variable fillet";
        return TopoDS_Shape();
    }
}

} // namespace part
} // namespace opencad
