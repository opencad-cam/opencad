#pragma once
/**
 * @file BooleanOps.h
 * @brief Boolean operations on shapes (Fuse, Cut, Common)
 * 
 * OpenCAD - Modular CAD/CAE Platform
 * Core Geometry Module
 */

#include "Shape.h"

namespace opencad {
namespace core {

/**
 * @class BooleanOps
 * @brief Boolean operations between shapes
 * 
 * Implements CSG (Constructive Solid Geometry) operations:
 * - Fuse (Union): Combine two shapes
 * - Cut (Difference): Subtract one shape from another
 * - Common (Intersection): Keep only overlapping region
 */
class BooleanOps {
public:
    /**
     * @brief Fuse (union) two shapes
     * @param shape1 First shape
     * @param shape2 Second shape
     * @return Combined shape
     */
    static Shape fuse(const Shape& shape1, const Shape& shape2);

    /**
     * @brief Cut shape2 from shape1
     * @param shape1 Base shape
     * @param shape2 Tool shape to subtract
     * @return Resulting shape after cut
     */
    static Shape cut(const Shape& shape1, const Shape& shape2);

    /**
     * @brief Find common (intersection) of two shapes
     * @param shape1 First shape
     * @param shape2 Second shape
     * @return Overlapping region
     */
    static Shape common(const Shape& shape1, const Shape& shape2);

    /**
     * @brief Section: Get intersection curve/face between shapes
     * @param shape1 First shape
     * @param shape2 Second shape
     * @return Section shape (typically edges/faces)
     */
    static Shape section(const Shape& shape1, const Shape& shape2);

private:
    BooleanOps() = delete;
};

} // namespace core
} // namespace opencad
