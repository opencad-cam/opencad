/**
 * @file PatternFeature.cpp
 * @brief Pattern feature implementation
 */

#include "PatternFeature.h"

#include <BRepBuilderAPI_Transform.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace opencad {
namespace part {

TopoDS_Shape PatternFeature::linearPattern(const TopoDS_Shape& shape,
                                            const gp_Dir& direction,
                                            int count,
                                            double spacing) {
    m_error.clear();
    
    if (shape.IsNull()) {
        m_error = "Input shape is null";
        return TopoDS_Shape();
    }
    
    if (count < 2) {
        m_error = "Count must be at least 2";
        return shape; // Return original
    }
    
    try {
        TopoDS_Shape result = shape;
        gp_Vec vec(direction);
        vec.Normalize();
        
        for (int i = 1; i < count; ++i) {
            gp_Trsf transform;
            transform.SetTranslation(vec * (spacing * i));
            
            BRepBuilderAPI_Transform builder(shape, transform, true);
            if (builder.IsDone()) {
                TopoDS_Shape copy = builder.Shape();
                
                BRepAlgoAPI_Fuse fuse(result, copy);
                fuse.Build();
                if (fuse.IsDone()) {
                    result = fuse.Shape();
                }
            }
        }
        
        return result;
    } catch (...) {
        m_error = "Exception during linear pattern";
        return TopoDS_Shape();
    }
}

TopoDS_Shape PatternFeature::linearPattern2D(const TopoDS_Shape& shape,
                                              const gp_Dir& dir1, int count1, double spacing1,
                                              const gp_Dir& dir2, int count2, double spacing2) {
    m_error.clear();
    
    if (shape.IsNull()) {
        m_error = "Input shape is null";
        return TopoDS_Shape();
    }
    
    try {
        TopoDS_Shape result = shape;
        gp_Vec vec1(dir1);
        gp_Vec vec2(dir2);
        vec1.Normalize();
        vec2.Normalize();
        
        for (int i = 0; i < count1; ++i) {
            for (int j = 0; j < count2; ++j) {
                if (i == 0 && j == 0) continue; // Skip original
                
                gp_Trsf transform;
                gp_Vec offset = vec1 * (spacing1 * i) + vec2 * (spacing2 * j);
                transform.SetTranslation(offset);
                
                BRepBuilderAPI_Transform builder(shape, transform, true);
                if (builder.IsDone()) {
                    TopoDS_Shape copy = builder.Shape();
                    
                    BRepAlgoAPI_Fuse fuse(result, copy);
                    fuse.Build();
                    if (fuse.IsDone()) {
                        result = fuse.Shape();
                    }
                }
            }
        }
        
        return result;
    } catch (...) {
        m_error = "Exception during 2D pattern";
        return TopoDS_Shape();
    }
}

TopoDS_Shape PatternFeature::circularPattern(const TopoDS_Shape& shape,
                                              const gp_Ax1& axis,
                                              int count,
                                              double angle,
                                              bool equalSpacing) {
    m_error.clear();
    
    if (shape.IsNull()) {
        m_error = "Input shape is null";
        return TopoDS_Shape();
    }
    
    if (count < 2) {
        m_error = "Count must be at least 2";
        return shape;
    }
    
    try {
        TopoDS_Shape result = shape;
        double angleRad = angle * M_PI / 180.0;
        double stepAngle = equalSpacing ? angleRad / count : angleRad / (count - 1);
        
        for (int i = 1; i < count; ++i) {
            gp_Trsf transform;
            transform.SetRotation(axis, stepAngle * i);
            
            BRepBuilderAPI_Transform builder(shape, transform, true);
            if (builder.IsDone()) {
                TopoDS_Shape copy = builder.Shape();
                
                BRepAlgoAPI_Fuse fuse(result, copy);
                fuse.Build();
                if (fuse.IsDone()) {
                    result = fuse.Shape();
                }
            }
        }
        
        return result;
    } catch (...) {
        m_error = "Exception during circular pattern";
        return TopoDS_Shape();
    }
}

} // namespace part
} // namespace opencad
