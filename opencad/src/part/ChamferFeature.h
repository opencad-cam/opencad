/**
 * @file ChamferFeature.h
 * @brief Chamfer (edge beveling) feature
 */

#pragma once

#include <TopoDS_Shape.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <vector>
#include <string>

namespace opencad {
namespace part {

/**
 * @class ChamferFeature
 * @brief Creates beveled edges on solid bodies
 */
class ChamferFeature {
public:
    ChamferFeature() = default;
    
    /**
     * @brief Apply symmetric chamfer (equal distances)
     * @param shape Base solid shape
     * @param edges Edges to chamfer
     * @param distance Chamfer distance
     * @return Chamfered shape
     */
    TopoDS_Shape execute(const TopoDS_Shape& shape,
                         const std::vector<TopoDS_Edge>& edges,
                         double distance);
    
    /**
     * @brief Apply asymmetric chamfer (two distances)
     * @param shape Base solid shape
     * @param edge Edge to chamfer
     * @param face Reference face for distance1
     * @param distance1 Distance on reference face
     * @param distance2 Distance on other face
     * @return Chamfered shape
     */
    TopoDS_Shape executeAsymmetric(const TopoDS_Shape& shape,
                                    const TopoDS_Edge& edge,
                                    const TopoDS_Face& face,
                                    double distance1,
                                    double distance2);
    
    /**
     * @brief Apply angle chamfer
     * @param shape Base solid shape
     * @param edge Edge to chamfer
     * @param face Reference face
     * @param distance Distance from edge
     * @param angle Chamfer angle in degrees
     * @return Chamfered shape
     */
    TopoDS_Shape executeAngle(const TopoDS_Shape& shape,
                               const TopoDS_Edge& edge,
                               const TopoDS_Face& face,
                               double distance,
                               double angle);
    
    std::string errorMessage() const { return m_error; }
    
private:
    std::string m_error;
};

} // namespace part
} // namespace opencad
