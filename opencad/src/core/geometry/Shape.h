#pragma once
/**
 * @file Shape.h
 * @brief Base shape class wrapping OpenCASCADE TopoDS_Shape
 * 
 * OpenCAD - Modular CAD/CAE Platform
 * Core Geometry Module
 */

#include <TopoDS_Shape.hxx>
#include <gp_Trsf.hxx>
#include <string>
#include <memory>

namespace opencad {
namespace core {

/**
 * @class Shape
 * @brief Wrapper class for OpenCASCADE TopoDS_Shape
 * 
 * Provides a clean C++ interface over OpenCASCADE's shape representation.
 * All geometric entities (solids, faces, edges, vertices) inherit from this.
 */
class Shape {
public:
    Shape();
    explicit Shape(const TopoDS_Shape& occShape);
    Shape(const Shape& other);
    Shape& operator=(const Shape& other);
    Shape(Shape&& other) noexcept;
    Shape& operator=(Shape&& other) noexcept;
    virtual ~Shape();

    /// Check if shape is valid (not null)
    bool isValid() const;

    /// Check if shape is null
    bool isNull() const;

    /// Get the underlying OpenCASCADE shape
    const TopoDS_Shape& occShape() const;
    TopoDS_Shape& occShape();

    /// Get shape type as string
    std::string shapeTypeString() const;

    /// Compute bounding box dimensions
    void boundingBox(double& xMin, double& yMin, double& zMin,
                     double& xMax, double& yMax, double& zMax) const;

    /// Check for shape validity/self-intersection
    bool checkValidity() const;

    /// Get volume (for solids)
    double volume() const;

    /// Get surface area
    double surfaceArea() const;

    /// Get center of mass
    void centerOfMass(double& x, double& y, double& z) const;

protected:
    TopoDS_Shape m_shape;
};

} // namespace core
} // namespace opencad
