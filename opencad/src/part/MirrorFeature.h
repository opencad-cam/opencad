/**
 * @file MirrorFeature.h
 * @brief Mirror feature - reflect across a plane
 */

#pragma once

#include <TopoDS_Shape.hxx>
#include <gp_Pln.hxx>
#include <gp_Ax2.hxx>
#include <string>

namespace opencad {
namespace part {

/**
 * @class MirrorFeature
 * @brief Creates a mirrored copy of a shape
 */
class MirrorFeature {
public:
    MirrorFeature() = default;
    
    /**
     * @brief Mirror shape across a plane
     * @param shape Shape to mirror
     * @param plane Mirror plane
     * @param keepOriginal If true, fuse with original
     * @return Mirrored (and optionally fused) shape
     */
    TopoDS_Shape execute(const TopoDS_Shape& shape,
                         const gp_Pln& plane,
                         bool keepOriginal = true);
    
    /**
     * @brief Mirror shape across a plane defined by axis
     * @param shape Shape to mirror
     * @param axis Axis defining the mirror plane (plane is perpendicular to axis direction)
     * @param keepOriginal If true, fuse with original
     * @return Mirrored shape
     */
    TopoDS_Shape executeAxis(const TopoDS_Shape& shape,
                              const gp_Ax2& axis,
                              bool keepOriginal = true);
    
    std::string errorMessage() const { return m_error; }
    
private:
    std::string m_error;
};

} // namespace part
} // namespace opencad
