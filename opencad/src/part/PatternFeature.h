/**
 * @file PatternFeature.h
 * @brief Pattern features - linear and circular
 */

#pragma once

#include <TopoDS_Shape.hxx>
#include <gp_Dir.hxx>
#include <gp_Ax1.hxx>
#include <string>

namespace opencad {
namespace part {

/**
 * @class PatternFeature
 * @brief Creates linear and circular patterns of features
 */
class PatternFeature {
public:
    PatternFeature() = default;
    
    /**
     * @brief Create linear pattern in one direction
     * @param shape Shape to pattern
     * @param direction Pattern direction
     * @param count Number of instances
     * @param spacing Distance between instances
     * @return Combined pattern shape
     */
    TopoDS_Shape linearPattern(const TopoDS_Shape& shape,
                               const gp_Dir& direction,
                               int count,
                               double spacing);
    
    /**
     * @brief Create linear pattern in two directions
     * @param shape Shape to pattern
     * @param dir1 First direction
     * @param count1 Count in first direction
     * @param spacing1 Spacing in first direction
     * @param dir2 Second direction
     * @param count2 Count in second direction
     * @param spacing2 Spacing in second direction
     * @return Combined pattern shape
     */
    TopoDS_Shape linearPattern2D(const TopoDS_Shape& shape,
                                  const gp_Dir& dir1, int count1, double spacing1,
                                  const gp_Dir& dir2, int count2, double spacing2);
    
    /**
     * @brief Create circular pattern
     * @param shape Shape to pattern
     * @param axis Rotation axis
     * @param count Number of instances
     * @param angle Total angle (360 for full circle)
     * @param equalSpacing If true, distribute evenly
     * @return Combined pattern shape
     */
    TopoDS_Shape circularPattern(const TopoDS_Shape& shape,
                                  const gp_Ax1& axis,
                                  int count,
                                  double angle,
                                  bool equalSpacing = true);
    
    std::string errorMessage() const { return m_error; }
    
private:
    std::string m_error;
};

} // namespace part
} // namespace opencad
