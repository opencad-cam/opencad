/**
 * @file CutFeature.cpp
 * @brief Cut/Pocket - remove material by extruding and subtracting sketch
 */

#include "CutFeature.h"
#include "WireBuilder.h"
#include "sketch/Sketch.h"

#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <gp_Vec.hxx>
#include <TopoDS.hxx>

namespace opencad {
namespace part {

TopoDS_Shape CutFeature::execute(const sketch::Sketch& sketch,
                                  const TopoDS_Shape& base,
                                  double depth,
                                  bool throughAll) {
    m_error.clear();
    
    if (base.IsNull()) {
        m_error = "No base shape to cut from";
        return TopoDS_Shape();
    }
    
    // Build face from sketch
    TopoDS_Face face = WireBuilder::buildFace(sketch);
    if (face.IsNull()) {
        m_error = "Failed to build face from sketch";
        return base;
    }
    
    double cutDepth = depth;
    
    if (throughAll) {
        // Calculate bounding box diagonal for through-all
        Bnd_Box box;
        BRepBndLib::Add(base, box);
        double xMin, yMin, zMin, xMax, yMax, zMax;
        box.Get(xMin, yMin, zMin, xMax, yMax, zMax);
        cutDepth = (zMax - zMin) * 2;  // Ensure we cut through
    }
    
    // Extrusion direction (negative Z for cutting down)
    gp_Vec cutVec(0, 0, -cutDepth);
    
    try {
        // Create the cutting tool by extruding the sketch
        BRepPrimAPI_MakePrism prism(face, cutVec);
        prism.Build();
        
        if (!prism.IsDone()) {
            m_error = "Failed to create cutting tool";
            return base;
        }
        
        TopoDS_Shape tool = prism.Shape();
        
        // Cut the tool from base
        BRepAlgoAPI_Cut cut(base, tool);
        cut.Build();
        
        if (cut.IsDone()) {
            return cut.Shape();
        } else {
            m_error = "Cut operation failed";
            return base;
        }
    } catch (...) {
        m_error = "Exception during cut operation";
        return base;
    }
}

} // namespace part
} // namespace opencad
