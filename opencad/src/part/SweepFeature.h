/**
 * @file SweepFeature.h
 * @brief Sweep feature - profile along a path
 */

#pragma once

#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <string>

namespace opencad {
namespace part {

/**
 * @class SweepFeature
 * @brief Sweeps a profile along a path to create a solid
 */
class SweepFeature {
public:
  SweepFeature() = default;

  /**
   * @brief Simple sweep (profile along path)
   * @param profile Profile wire/face to sweep
   * @param path Path wire to follow
   * @param closedPath If true, treats path as closed curve (connects end to
   * start)
   * @return Swept solid shape
   */
  TopoDS_Shape execute(const TopoDS_Shape &profile, const TopoDS_Wire &path,
                       bool closedPath = false);

  /**
   * @brief Sweep with guide curve
   * @param profile Profile wire/face
   * @param path Path wire
   * @param guide Guide curve for orientation
   * @return Swept solid
   */
  TopoDS_Shape executeWithGuide(const TopoDS_Shape &profile,
                                const TopoDS_Wire &path,
                                const TopoDS_Wire &guide);

  std::string errorMessage() const { return m_error; }

private:
  std::string m_error;
};

} // namespace part
} // namespace opencad
