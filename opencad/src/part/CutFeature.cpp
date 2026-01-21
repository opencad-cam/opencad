/**
 * @file CutFeature.cpp
 * @brief Cut/Pocket - remove material by extruding and subtracting sketch
 */

#include "CutFeature.h"
#include "WireBuilder.h"
#include "sketch/Sketch.h"
#include "sketch/SketchPlane.h"

#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
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
    
    // Build wire from sketch
    TopoDS_Wire wire = WireBuilder::buildWire(sketch);
    if (wire.IsNull()) {
        m_error = "Failed to build wire from sketch";
        return base;
    }

    // Build face using sketch plane
    const gp_Pln& plane = sketch.plane().plane();
    TopoDS_Face face;
    try {
        BRepBuilderAPI_MakeFace makeFace(plane, wire, true);
        if (makeFace.IsDone()) {
            face = makeFace.Face();
        }
    } catch (...) {
        m_error = "Exception during face creation";
        return base;
    }

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
        // Use a safe large value relative to model size
        double diag = std::sqrt(std::pow(xMax - xMin, 2) + std::pow(yMax - yMin, 2) + std::pow(zMax - zMin, 2));
        cutDepth = diag * 2.0;
        if (cutDepth < 1.0) cutDepth = 1000.0;
    }
    
    // Extrusion direction: Opposite to sketch plane normal
    gp_Dir normal = sketch.plane().normal();
    gp_Vec cutVec(normal);
    cutVec.Scale(-cutDepth);
    
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
