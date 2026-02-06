#pragma once

#include <TopoDS_Shape.hxx>
#include <string>

namespace opencad {
namespace part {

struct GearParams {
  double module = 1.0;
  int numTeeth = 20;
  double pressureAngle = 20.0; // Degrees
  double thickness = 5.0;
  bool centerHole = false;
  double holeDiameter = 0.0;
};

class GearFeature {
public:
  GearFeature() = default;

  /**
   * @brief Create a spur gear solid
   * @param params Gear parameters
   * @return TopoDS_Shape Solid gear or null on failure
   */
  TopoDS_Shape execute(const GearParams &params);

  const std::string &errorMessage() const { return m_error; }

private:
  std::string m_error;
};

} // namespace part
} // namespace opencad
