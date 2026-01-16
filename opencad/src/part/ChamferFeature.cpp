/**
 * @file ChamferFeature.cpp
 * @brief Chamfer feature implementation using OpenCASCADE
 */

#include "ChamferFeature.h"

#include <BRepFilletAPI_MakeChamfer.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedDataMapOfShapeListOfShape.hxx>
#include <TopExp.hxx>
#include <TopoDS.hxx>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace opencad {
namespace part {

TopoDS_Shape ChamferFeature::execute(const TopoDS_Shape& shape,
                                      const std::vector<TopoDS_Edge>& edges,
                                      double distance) {
    m_error.clear();
    
    if (shape.IsNull()) {
        m_error = "Input shape is null";
        return TopoDS_Shape();
    }
    
    if (edges.empty()) {
        m_error = "No edges selected for chamfer";
        return TopoDS_Shape();
    }
    
    if (distance <= 0) {
        m_error = "Distance must be positive";
        return TopoDS_Shape();
    }
    
    try {
        BRepFilletAPI_MakeChamfer chamfer(shape);
        
        // Build edge-face map for chamfer
        TopTools_IndexedDataMapOfShapeListOfShape edgeFaceMap;
        TopExp::MapShapesAndAncestors(shape, TopAbs_EDGE, TopAbs_FACE, edgeFaceMap);
        
        for (const auto& edge : edges) {
            if (!edge.IsNull()) {
                // Find adjacent face for the edge
                if (edgeFaceMap.Contains(edge)) {
                    const TopTools_ListOfShape& faces = edgeFaceMap.FindFromKey(edge);
                    if (!faces.IsEmpty()) {
                        TopoDS_Face face = TopoDS::Face(faces.First());
                        chamfer.Add(distance, distance, edge, face);
                    }
                }
            }
        }
        
        chamfer.Build();
        
        if (chamfer.IsDone()) {
            return chamfer.Shape();
        } else {
            m_error = "Chamfer operation failed";
            return TopoDS_Shape();
        }
    } catch (const Standard_Failure& e) {
        m_error = "Exception: " + std::string(e.GetMessageString());
        return TopoDS_Shape();
    } catch (...) {
        m_error = "Unknown exception during chamfer";
        return TopoDS_Shape();
    }
}

TopoDS_Shape ChamferFeature::executeAsymmetric(const TopoDS_Shape& shape,
                                                const TopoDS_Edge& edge,
                                                const TopoDS_Face& face,
                                                double distance1,
                                                double distance2) {
    m_error.clear();
    
    if (shape.IsNull() || edge.IsNull() || face.IsNull()) {
        m_error = "Invalid input";
        return TopoDS_Shape();
    }
    
    try {
        BRepFilletAPI_MakeChamfer chamfer(shape);
        chamfer.Add(distance1, distance2, edge, face);
        chamfer.Build();
        
        if (chamfer.IsDone()) {
            return chamfer.Shape();
        } else {
            m_error = "Asymmetric chamfer failed";
            return TopoDS_Shape();
        }
    } catch (...) {
        m_error = "Exception during asymmetric chamfer";
        return TopoDS_Shape();
    }
}

TopoDS_Shape ChamferFeature::executeAngle(const TopoDS_Shape& shape,
                                           const TopoDS_Edge& edge,
                                           const TopoDS_Face& face,
                                           double distance,
                                           double angle) {
    m_error.clear();
    
    if (shape.IsNull() || edge.IsNull() || face.IsNull()) {
        m_error = "Invalid input";
        return TopoDS_Shape();
    }
    
    try {
        BRepFilletAPI_MakeChamfer chamfer(shape);
        
        // Convert angle to radians
        double angleRad = angle * M_PI / 180.0;
        chamfer.AddDA(distance, angleRad, edge, face);
        chamfer.Build();
        
        if (chamfer.IsDone()) {
            return chamfer.Shape();
        } else {
            m_error = "Angle chamfer failed";
            return TopoDS_Shape();
        }
    } catch (...) {
        m_error = "Exception during angle chamfer";
        return TopoDS_Shape();
    }
}

} // namespace part
} // namespace opencad
