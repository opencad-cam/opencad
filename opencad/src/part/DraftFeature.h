/**
 * @file DraftFeature.h
 * @brief Draft feature - add taper angle to faces
 *
 * OpenCAD - Modular CAD/CAE Platform
 */
#pragma once

#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <gp_Dir.hxx>
#include <gp_Pln.hxx>
#include <string>
#include <vector>


namespace opencad {
namespace part {

/**
 * @enum DraftType
 * @brief Type of draft operation
 */
enum class DraftType {
  NeutralPlane, // Draft from a neutral plane
  PartingLine,  // Draft from a parting line
  StepDraft     // Step draft with multiple angles
};

/**
 * @struct DraftParams
 * @brief Parameters for draft operation
 */
struct DraftParams {
  double angle = 3.0;    // Draft angle in degrees
  bool reversed = false; // Reverse draft direction
  DraftType type = DraftType::NeutralPlane;
};

/**
 * @class DraftFeature
 * @brief Adds draft (taper) angle to selected faces
 *
 * Draft is used for creating moldable parts where faces need
 * a slight angle for extraction from molds.
 */
class DraftFeature {
public:
  DraftFeature() = default;

  /**
   * @brief Apply draft to faces using neutral plane
   * @param shape Input shape
   * @param faces Faces to draft
   * @param neutralPlane The neutral plane (faces drafted relative to this)
   * @param direction Pull direction
   * @param angleDeg Draft angle in degrees
   * @return Shape with drafted faces
   */
  TopoDS_Shape execute(const TopoDS_Shape &shape,
                       const std::vector<TopoDS_Face> &faces,
                       const gp_Pln &neutralPlane, const gp_Dir &direction,
                       double angleDeg);

  /**
   * @brief Apply draft to all faces in a given direction
   * @param shape Input shape
   * @param direction Pull direction
   * @param neutralPlane Neutral plane
   * @param angleDeg Draft angle in degrees
   * @return Shape with drafted faces
   */
  TopoDS_Shape executeAll(const TopoDS_Shape &shape, const gp_Dir &direction,
                          const gp_Pln &neutralPlane, double angleDeg);

  /**
   * @brief Apply draft with parameters
   */
  TopoDS_Shape execute(const TopoDS_Shape &shape,
                       const std::vector<TopoDS_Face> &faces,
                       const gp_Dir &direction, const DraftParams &params);

  /**
   * @brief Get last error message
   */
  const std::string &errorMessage() const { return m_error; }

private:
  std::string m_error;

  /**
   * @brief Find faces perpendicular to direction that can be drafted
   */
  std::vector<TopoDS_Face> findDraftableFaces(const TopoDS_Shape &shape,
                                              const gp_Dir &direction) const;
};

} // namespace part
} // namespace opencad
