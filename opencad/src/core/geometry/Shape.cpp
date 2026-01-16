/**
 * @file Shape.cpp
 * @brief Implementation of Shape class
 */

#include "Shape.h"

#include <BRepBndLib.hxx>
#include <Bnd_Box.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <TopAbs_ShapeEnum.hxx>

namespace opencad {
namespace core {

Shape::Shape() : m_shape() {}

Shape::Shape(const TopoDS_Shape& occShape) : m_shape(occShape) {}

Shape::Shape(const Shape& other) : m_shape(other.m_shape) {}

Shape& Shape::operator=(const Shape& other) {
    if (this != &other) {
        m_shape = other.m_shape;
    }
    return *this;
}

Shape::Shape(Shape&& other) noexcept : m_shape(std::move(other.m_shape)) {}

Shape& Shape::operator=(Shape&& other) noexcept {
    if (this != &other) {
        m_shape = std::move(other.m_shape);
    }
    return *this;
}

Shape::~Shape() = default;

bool Shape::isValid() const {
    return !m_shape.IsNull();
}

bool Shape::isNull() const {
    return m_shape.IsNull();
}

const TopoDS_Shape& Shape::occShape() const {
    return m_shape;
}

TopoDS_Shape& Shape::occShape() {
    return m_shape;
}

std::string Shape::shapeTypeString() const {
    if (isNull()) return "Null";
    
    switch (m_shape.ShapeType()) {
        case TopAbs_COMPOUND:  return "Compound";
        case TopAbs_COMPSOLID: return "CompSolid";
        case TopAbs_SOLID:     return "Solid";
        case TopAbs_SHELL:     return "Shell";
        case TopAbs_FACE:      return "Face";
        case TopAbs_WIRE:      return "Wire";
        case TopAbs_EDGE:      return "Edge";
        case TopAbs_VERTEX:    return "Vertex";
        case TopAbs_SHAPE:     return "Shape";
        default:               return "Unknown";
    }
}

void Shape::boundingBox(double& xMin, double& yMin, double& zMin,
                        double& xMax, double& yMax, double& zMax) const {
    if (isNull()) {
        xMin = yMin = zMin = xMax = yMax = zMax = 0.0;
        return;
    }
    
    Bnd_Box box;
    BRepBndLib::Add(m_shape, box);
    
    if (box.IsVoid()) {
        xMin = yMin = zMin = xMax = yMax = zMax = 0.0;
        return;
    }
    
    box.Get(xMin, yMin, zMin, xMax, yMax, zMax);
}

bool Shape::checkValidity() const {
    if (isNull()) return false;
    
    BRepCheck_Analyzer analyzer(m_shape);
    return analyzer.IsValid();
}

double Shape::volume() const {
    if (isNull()) return 0.0;
    
    GProp_GProps props;
    BRepGProp::VolumeProperties(m_shape, props);
    return props.Mass();
}

double Shape::surfaceArea() const {
    if (isNull()) return 0.0;
    
    GProp_GProps props;
    BRepGProp::SurfaceProperties(m_shape, props);
    return props.Mass();
}

void Shape::centerOfMass(double& x, double& y, double& z) const {
    if (isNull()) {
        x = y = z = 0.0;
        return;
    }
    
    GProp_GProps props;
    BRepGProp::VolumeProperties(m_shape, props);
    gp_Pnt com = props.CentreOfMass();
    x = com.X();
    y = com.Y();
    z = com.Z();
}

} // namespace core
} // namespace opencad
