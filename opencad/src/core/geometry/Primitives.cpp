/**
 * @file Primitives.cpp
 * @brief Implementation of primitive shape creation
 */

#include "Primitives.h"

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <BRepPrimAPI_MakeWedge.hxx>
#include <gp_Ax2.hxx>

namespace opencad {
namespace core {

Shape Primitives::makeBox(double sizeX, double sizeY, double sizeZ) {
    // Create box centered at origin using two corner points
    gp_Pnt corner1(-sizeX/2, -sizeY/2, -sizeZ/2);
    gp_Pnt corner2(sizeX/2, sizeY/2, sizeZ/2);
    BRepPrimAPI_MakeBox maker(corner1, corner2);
    maker.Build();
    
    if (!maker.IsDone()) {
        return Shape(); // Return null shape on failure
    }
    
    return Shape(maker.Shape());
}

Shape Primitives::makeBox(double x, double y, double z,
                          double sizeX, double sizeY, double sizeZ) {
    BRepPrimAPI_MakeBox maker(gp_Pnt(x, y, z),
                               sizeX, sizeY, sizeZ);
    maker.Build();
    
    if (!maker.IsDone()) {
        return Shape();
    }
    
    return Shape(maker.Shape());
}

Shape Primitives::makeCylinder(double radius, double height) {
    BRepPrimAPI_MakeCylinder maker(radius, height);
    maker.Build();
    
    if (!maker.IsDone()) {
        return Shape();
    }
    
    return Shape(maker.Shape());
}

Shape Primitives::makeCylinder(double baseX, double baseY, double baseZ,
                               double dirX, double dirY, double dirZ,
                               double radius, double height) {
    gp_Pnt base(baseX, baseY, baseZ);
    gp_Dir dir(dirX, dirY, dirZ);
    gp_Ax2 axis(base, dir);
    
    BRepPrimAPI_MakeCylinder maker(axis, radius, height);
    maker.Build();
    
    if (!maker.IsDone()) {
        return Shape();
    }
    
    return Shape(maker.Shape());
}

Shape Primitives::makeSphere(double radius) {
    BRepPrimAPI_MakeSphere maker(radius);
    maker.Build();
    
    if (!maker.IsDone()) {
        return Shape();
    }
    
    return Shape(maker.Shape());
}

Shape Primitives::makeSphere(double centerX, double centerY, double centerZ,
                             double radius) {
    gp_Pnt center(centerX, centerY, centerZ);
    BRepPrimAPI_MakeSphere maker(center, radius);
    maker.Build();
    
    if (!maker.IsDone()) {
        return Shape();
    }
    
    return Shape(maker.Shape());
}

Shape Primitives::makeCone(double baseRadius, double topRadius, double height) {
    BRepPrimAPI_MakeCone maker(baseRadius, topRadius, height);
    maker.Build();
    
    if (!maker.IsDone()) {
        return Shape();
    }
    
    return Shape(maker.Shape());
}

Shape Primitives::makeTorus(double majorRadius, double minorRadius) {
    BRepPrimAPI_MakeTorus maker(majorRadius, minorRadius);
    maker.Build();
    
    if (!maker.IsDone()) {
        return Shape();
    }
    
    return Shape(maker.Shape());
}

Shape Primitives::makeWedge(double dx, double dy, double dz, double ltx) {
    BRepPrimAPI_MakeWedge maker(dx, dy, dz, ltx);
    maker.Build();
    
    if (!maker.IsDone()) {
        return Shape();
    }
    
    return Shape(maker.Shape());
}

} // namespace core
} // namespace opencad
