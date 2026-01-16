/**
 * @file MirrorFeature.cpp
 * @brief Mirror feature implementation
 */

#include "MirrorFeature.h"

#include <BRepBuilderAPI_Transform.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <gp_Trsf.hxx>

namespace opencad {
namespace part {

TopoDS_Shape MirrorFeature::execute(const TopoDS_Shape& shape,
                                     const gp_Pln& plane,
                                     bool keepOriginal) {
    m_error.clear();
    
    if (shape.IsNull()) {
        m_error = "Input shape is null";
        return TopoDS_Shape();
    }
    
    try {
        gp_Trsf transform;
        transform.SetMirror(plane.Position().Ax2());
        
        BRepBuilderAPI_Transform builder(shape, transform, true);
        
        if (builder.IsDone()) {
            TopoDS_Shape mirrored = builder.Shape();
            
            if (keepOriginal) {
                BRepAlgoAPI_Fuse fuse(shape, mirrored);
                fuse.Build();
                if (fuse.IsDone()) {
                    return fuse.Shape();
                }
            }
            
            return mirrored;
        } else {
            m_error = "Mirror transform failed";
            return TopoDS_Shape();
        }
    } catch (const Standard_Failure& e) {
        m_error = "Exception: " + std::string(e.GetMessageString());
        return TopoDS_Shape();
    } catch (...) {
        m_error = "Unknown exception during mirror";
        return TopoDS_Shape();
    }
}

TopoDS_Shape MirrorFeature::executeAxis(const TopoDS_Shape& shape,
                                         const gp_Ax2& axis,
                                         bool keepOriginal) {
    m_error.clear();
    
    if (shape.IsNull()) {
        m_error = "Input shape is null";
        return TopoDS_Shape();
    }
    
    try {
        gp_Trsf transform;
        transform.SetMirror(axis);
        
        BRepBuilderAPI_Transform builder(shape, transform, true);
        
        if (builder.IsDone()) {
            TopoDS_Shape mirrored = builder.Shape();
            
            if (keepOriginal) {
                BRepAlgoAPI_Fuse fuse(shape, mirrored);
                fuse.Build();
                if (fuse.IsDone()) {
                    return fuse.Shape();
                }
            }
            
            return mirrored;
        } else {
            m_error = "Mirror transform failed";
            return TopoDS_Shape();
        }
    } catch (...) {
        m_error = "Exception during mirror";
        return TopoDS_Shape();
    }
}

} // namespace part
} // namespace opencad
