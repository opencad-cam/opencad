/**
 * @file Transform.cpp
 * @brief Implementation of geometric transformations
 */

#include "Transform.h"

#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <gp_Ax1.hxx>
#include <gp_Ax2.hxx>
#include <gp_Pln.hxx>
#include <gp_GTrsf.hxx>

namespace opencad {
namespace core {

Shape Transform::translate(const Shape& shape, double dx, double dy, double dz) {
    if (shape.isNull()) return Shape();

    gp_Trsf trsf;
    trsf.SetTranslation(gp_Vec(dx, dy, dz));
    
    return apply(shape, trsf);
}

Shape Transform::rotate(const Shape& shape,
                        double axisX, double axisY, double axisZ,
                        double dirX, double dirY, double dirZ,
                        double angleRadians) {
    if (shape.isNull()) return Shape();

    gp_Pnt point(axisX, axisY, axisZ);
    gp_Dir dir(dirX, dirY, dirZ);
    gp_Ax1 axis(point, dir);

    gp_Trsf trsf;
    trsf.SetRotation(axis, angleRadians);
    
    return apply(shape, trsf);
}

Shape Transform::rotateX(const Shape& shape, double angleRadians) {
    return rotate(shape, 0, 0, 0, 1, 0, 0, angleRadians);
}

Shape Transform::rotateY(const Shape& shape, double angleRadians) {
    return rotate(shape, 0, 0, 0, 0, 1, 0, angleRadians);
}

Shape Transform::rotateZ(const Shape& shape, double angleRadians) {
    return rotate(shape, 0, 0, 0, 0, 0, 1, angleRadians);
}

Shape Transform::scale(const Shape& shape, double factor) {
    if (shape.isNull()) return Shape();

    gp_Trsf trsf;
    trsf.SetScaleFactor(factor);
    
    return apply(shape, trsf);
}

Shape Transform::scale(const Shape& shape, 
                       double factorX, double factorY, double factorZ) {
    if (shape.isNull()) return Shape();

    // Non-uniform scaling requires GTrsf
    gp_GTrsf gtrsf;
    gtrsf.SetValue(1, 1, factorX);
    gtrsf.SetValue(2, 2, factorY);
    gtrsf.SetValue(3, 3, factorZ);

    BRepBuilderAPI_GTransform transformer(shape.occShape(), gtrsf, true);
    transformer.Build();

    if (!transformer.IsDone()) {
        return Shape();
    }

    return Shape(transformer.Shape());
}

Shape Transform::mirror(const Shape& shape,
                        double planePointX, double planePointY, double planePointZ,
                        double normalX, double normalY, double normalZ) {
    if (shape.isNull()) return Shape();

    gp_Pnt point(planePointX, planePointY, planePointZ);
    gp_Dir normal(normalX, normalY, normalZ);
    gp_Ax2 axis(point, normal);

    gp_Trsf trsf;
    trsf.SetMirror(axis);
    
    return apply(shape, trsf);
}

Shape Transform::apply(const Shape& shape, const gp_Trsf& trsf) {
    if (shape.isNull()) return Shape();

    BRepBuilderAPI_Transform transformer(shape.occShape(), trsf, true);
    transformer.Build();

    if (!transformer.IsDone()) {
        return Shape();
    }

    return Shape(transformer.Shape());
}

Shape Transform::copy(const Shape& shape) {
    if (shape.isNull()) return Shape();

    BRepBuilderAPI_Copy copier(shape.occShape());
    copier.Build();

    if (!copier.IsDone()) {
        return Shape();
    }

    return Shape(copier.Shape());
}

} // namespace core
} // namespace opencad
