/**
 * @file BooleanOps.cpp
 * @brief Implementation of boolean operations
 */

#include "BooleanOps.h"

#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>

namespace opencad {
namespace core {

Shape BooleanOps::fuse(const Shape& shape1, const Shape& shape2) {
    if (shape1.isNull() || shape2.isNull()) {
        return Shape();
    }

    BRepAlgoAPI_Fuse fuser(shape1.occShape(), shape2.occShape());
    fuser.Build();

    if (!fuser.IsDone()) {
        return Shape();
    }

    // Clean up the result - merge same-domain faces
    TopoDS_Shape result = fuser.Shape();
    ShapeUpgrade_UnifySameDomain unifier(result);
    unifier.Build();
    
    if (unifier.Shape().IsNull()) {
        return Shape(result);
    }
    
    return Shape(unifier.Shape());
}

Shape BooleanOps::cut(const Shape& shape1, const Shape& shape2) {
    if (shape1.isNull() || shape2.isNull()) {
        return Shape();
    }

    BRepAlgoAPI_Cut cutter(shape1.occShape(), shape2.occShape());
    cutter.Build();

    if (!cutter.IsDone()) {
        return Shape();
    }

    // Clean up the result
    TopoDS_Shape result = cutter.Shape();
    ShapeUpgrade_UnifySameDomain unifier(result);
    unifier.Build();
    
    if (unifier.Shape().IsNull()) {
        return Shape(result);
    }
    
    return Shape(unifier.Shape());
}

Shape BooleanOps::common(const Shape& shape1, const Shape& shape2) {
    if (shape1.isNull() || shape2.isNull()) {
        return Shape();
    }

    BRepAlgoAPI_Common inter(shape1.occShape(), shape2.occShape());
    inter.Build();

    if (!inter.IsDone()) {
        return Shape();
    }

    return Shape(inter.Shape());
}

Shape BooleanOps::section(const Shape& shape1, const Shape& shape2) {
    if (shape1.isNull() || shape2.isNull()) {
        return Shape();
    }

    BRepAlgoAPI_Section sectioner(shape1.occShape(), shape2.occShape());
    sectioner.Build();

    if (!sectioner.IsDone()) {
        return Shape();
    }

    return Shape(sectioner.Shape());
}

} // namespace core
} // namespace opencad
