#pragma once
/**
 * @file Primitives.h
 * @brief Primitive shape creation (Box, Cylinder, Sphere, Cone, Torus)
 * 
 * OpenCAD - Modular CAD/CAE Platform
 * Core Geometry Module
 */

#include "Shape.h"
#include <gp_Pnt.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax2.hxx>

namespace opencad {
namespace core {

/**
 * @class Primitives
 * @brief Factory class for creating primitive 3D shapes
 * 
 * All primitive creation methods return Shape objects that wrap
 * the underlying OpenCASCADE geometry.
 */
class Primitives {
public:
    /**
     * @brief Create a box centered at origin
     * @param sizeX Width in X direction
     * @param sizeY Depth in Y direction  
     * @param sizeZ Height in Z direction
     * @return Shape containing the box solid
     */
    static Shape makeBox(double sizeX, double sizeY, double sizeZ);

    /**
     * @brief Create a box at specified position
     * @param x X coordinate of corner
     * @param y Y coordinate of corner
     * @param z Z coordinate of corner
     * @param sizeX Width
     * @param sizeY Depth
     * @param sizeZ Height
     */
    static Shape makeBox(double x, double y, double z,
                         double sizeX, double sizeY, double sizeZ);

    /**
     * @brief Create a cylinder along Z axis
     * @param radius Cylinder radius
     * @param height Cylinder height
     * @return Shape containing the cylinder solid
     */
    static Shape makeCylinder(double radius, double height);

    /**
     * @brief Create a cylinder with specified axis
     * @param baseX Base center X
     * @param baseY Base center Y
     * @param baseZ Base center Z
     * @param dirX Axis direction X
     * @param dirY Axis direction Y
     * @param dirZ Axis direction Z
     * @param radius Cylinder radius
     * @param height Cylinder height
     */
    static Shape makeCylinder(double baseX, double baseY, double baseZ,
                              double dirX, double dirY, double dirZ,
                              double radius, double height);

    /**
     * @brief Create a sphere at origin
     * @param radius Sphere radius
     * @return Shape containing the sphere solid
     */
    static Shape makeSphere(double radius);

    /**
     * @brief Create a sphere at specified center
     * @param centerX Center X
     * @param centerY Center Y
     * @param centerZ Center Z
     * @param radius Sphere radius
     */
    static Shape makeSphere(double centerX, double centerY, double centerZ,
                            double radius);

    /**
     * @brief Create a cone along Z axis
     * @param baseRadius Bottom radius
     * @param topRadius Top radius (0 for pointed cone)
     * @param height Cone height
     * @return Shape containing the cone solid
     */
    static Shape makeCone(double baseRadius, double topRadius, double height);

    /**
     * @brief Create a torus
     * @param majorRadius Distance from center to tube center
     * @param minorRadius Tube radius
     * @return Shape containing the torus solid
     */
    static Shape makeTorus(double majorRadius, double minorRadius);

    /**
     * @brief Create a wedge (tapered box)
     * @param dx X dimension at base
     * @param dy Y dimension
     * @param dz Z dimension
     * @param ltx X dimension at top
     */
    static Shape makeWedge(double dx, double dy, double dz, double ltx);

    /**
     * @brief Create a simple screw/bolt representation (Head + Shaft)
     * @param radius Shaft radius
     * @param height Shaft height
     * @param headRadius Head radius (default: 1.8 * radius)
     * @param headHeight Head height (default: 0.2 * height)
     * @return Shape containing the fused screw solid
     */
    static Shape makeScrew(double radius, double height, 
                           double headRadius = 0.0, double headHeight = 0.0);

private:
    Primitives() = delete; // Static factory only
};

} // namespace core
} // namespace opencad
