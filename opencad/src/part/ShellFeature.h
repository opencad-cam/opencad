/**
 * @file ShellFeature.h
 * @brief Shell (hollowing) feature
 */

#pragma once

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <vector>
#include <string>

namespace opencad {
namespace part {

/**
 * @class ShellFeature
 * @brief Creates a hollow shell from a solid by removing faces
 */
class ShellFeature {
public:
    ShellFeature() = default;
    
    /**
     * @brief Create shell by removing faces and offsetting
     * @param shape Base solid shape
     * @param facesToRemove Faces to open (remove)
     * @param thickness Wall thickness (positive = inward, negative = outward)
     * @return Shelled shape
     */
    TopoDS_Shape execute(const TopoDS_Shape& shape,
                         const std::vector<TopoDS_Face>& facesToRemove,
                         double thickness);
    
    std::string errorMessage() const { return m_error; }
    
private:
    std::string m_error;
};

} // namespace part
} // namespace opencad
