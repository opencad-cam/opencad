/**
 * @file FilletFeature.h
 * @brief Fillet (edge rounding) feature
 */

#pragma once

#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <vector>
#include <string>

namespace opencad {
namespace part {

/**
 * @class FilletFeature
 * @brief Creates rounded edges on solid bodies
 */
class FilletFeature {
public:
    FilletFeature() = default;
    
    /**
     * @brief Apply fillet to selected edges
     * @param shape Base solid shape
     * @param edges Edges to fillet
     * @param radius Fillet radius
     * @return Filleted shape, or null if failed
     */
    TopoDS_Shape execute(const TopoDS_Shape& shape,
                         const std::vector<TopoDS_Edge>& edges,
                         double radius);
    
    /**
     * @brief Apply fillet with variable radius
     * @param shape Base solid shape
     * @param edge Edge to fillet
     * @param radius1 Start radius
     * @param radius2 End radius
     * @return Filleted shape
     */
    TopoDS_Shape executeVariable(const TopoDS_Shape& shape,
                                  const TopoDS_Edge& edge,
                                  double radius1,
                                  double radius2);
    
    /**
     * @brief Get last error message
     */
    std::string errorMessage() const { return m_error; }
    
private:
    std::string m_error;
};

} // namespace part
} // namespace opencad
