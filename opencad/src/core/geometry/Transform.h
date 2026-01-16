#pragma once
/**
 * @file Transform.h
 * @brief Geometric transformations (translate, rotate, scale, mirror)
 * 
 * OpenCAD - Modular CAD/CAE Platform
 * Core Geometry Module
 */

#include "Shape.h"
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <gp_Ax1.hxx>

namespace opencad {
namespace core {

/**
 * @class Transform
 * @brief Geometric transformation operations
 */
class Transform {
public:
    /**
     * @brief Translate shape by vector
     * @param shape Shape to translate
     * @param dx X displacement
     * @param dy Y displacement
     * @param dz Z displacement
     * @return Translated shape
     */
    static Shape translate(const Shape& shape, double dx, double dy, double dz);

    /**
     * @brief Rotate shape around axis
     * @param shape Shape to rotate
     * @param axisX Axis point X
     * @param axisY Axis point Y
     * @param axisZ Axis point Z
     * @param dirX Axis direction X
     * @param dirY Axis direction Y
     * @param dirZ Axis direction Z
     * @param angleRadians Rotation angle in radians
     * @return Rotated shape
     */
    static Shape rotate(const Shape& shape,
                        double axisX, double axisY, double axisZ,
                        double dirX, double dirY, double dirZ,
                        double angleRadians);

    /**
     * @brief Rotate around X axis at origin
     */
    static Shape rotateX(const Shape& shape, double angleRadians);

    /**
     * @brief Rotate around Y axis at origin
     */
    static Shape rotateY(const Shape& shape, double angleRadians);

    /**
     * @brief Rotate around Z axis at origin
     */
    static Shape rotateZ(const Shape& shape, double angleRadians);

    /**
     * @brief Scale shape uniformly
     * @param shape Shape to scale
     * @param factor Scale factor
     * @return Scaled shape
     */
    static Shape scale(const Shape& shape, double factor);

    /**
     * @brief Scale shape non-uniformly
     * @param shape Shape to scale
     * @param factorX X scale factor
     * @param factorY Y scale factor
     * @param factorZ Z scale factor
     * @return Scaled shape
     */
    static Shape scale(const Shape& shape, 
                       double factorX, double factorY, double factorZ);

    /**
     * @brief Mirror shape across a plane
     * @param shape Shape to mirror
     * @param planePointX Point on mirror plane X
     * @param planePointY Point on mirror plane Y
     * @param planePointZ Point on mirror plane Z
     * @param normalX Plane normal X
     * @param normalY Plane normal Y
     * @param normalZ Plane normal Z
     * @return Mirrored shape
     */
    static Shape mirror(const Shape& shape,
                        double planePointX, double planePointY, double planePointZ,
                        double normalX, double normalY, double normalZ);

    /**
     * @brief Apply transformation matrix
     * @param shape Shape to transform
     * @param trsf OpenCASCADE transformation
     * @return Transformed shape
     */
    static Shape apply(const Shape& shape, const gp_Trsf& trsf);

    /**
     * @brief Create a copy of the shape
     */
    static Shape copy(const Shape& shape);

private:
    Transform() = delete;
};

} // namespace core
} // namespace opencad
