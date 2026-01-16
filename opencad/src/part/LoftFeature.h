/**
 * @file LoftFeature.h
 * @brief Loft feature - transition between profiles
 */

#pragma once

#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <vector>
#include <string>

namespace opencad {
namespace part {

/**
 * @class LoftFeature
 * @brief Creates a solid by blending between multiple profiles
 */
class LoftFeature {
public:
    LoftFeature() = default;
    
    /**
     * @brief Create loft from multiple profiles
     * @param profiles List of profile wires (in order)
     * @param solid If true, create solid; if false, create shell
     * @param ruled If true, use ruled surface; if false, smooth
     * @return Lofted shape
     */
    TopoDS_Shape execute(const std::vector<TopoDS_Wire>& profiles,
                         bool solid = true,
                         bool ruled = false);
    
    std::string errorMessage() const { return m_error; }
    
private:
    std::string m_error;
};

} // namespace part
} // namespace opencad
