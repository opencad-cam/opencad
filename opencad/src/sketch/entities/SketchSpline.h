/**
 * @file SketchSpline.h
 * @brief 2D B-Spline curve entity for sketches with SolidWorks-style tangent
 * handles
 */

#pragma once

#include "SketchEntity.h"
#include <Geom2d_BSplineCurve.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Vec2d.hxx>
#include <vector>


namespace opencad {
namespace sketch {

/**
 * @brief Spline type - how points define the curve
 */
enum class SplineType {
  FitPoints,    // Curve passes through points (default)
  ControlPoints // Points are B-spline control points
};

/**
 * @brief 2D B-Spline curve entity with tangent handle support
 *
 * A spline defined by fit points with optional start/end tangent control.
 * SolidWorks-style tangent handles allow adjusting curve direction at
 * endpoints.
 */
class SketchSpline : public SketchEntity {
public:
  using Ptr = std::shared_ptr<SketchSpline>;

  SketchSpline();
  explicit SketchSpline(const std::vector<gp_Pnt2d> &points);

  // Type
  EntityType type() const override { return EntityType::Spline; }
  std::string typeName() const override { return "Spline"; }

  // Control points
  const std::vector<gp_Pnt2d> &controlPoints() const { return m_controlPoints; }
  void setControlPoints(const std::vector<gp_Pnt2d> &points);
  void addControlPoint(const gp_Pnt2d &point);
  void insertControlPoint(size_t index, const gp_Pnt2d &point);
  void removeControlPoint(size_t index);
  void setControlPoint(size_t index, const gp_Pnt2d &point);
  size_t controlPointCount() const { return m_controlPoints.size(); }

  // Spline type
  SplineType splineType() const { return m_splineType; }
  void setSplineType(SplineType type) {
    m_splineType = type;
    rebuildCurve();
  }

  // Degree
  int degree() const { return m_degree; }
  void setDegree(int degree);

  // Closed spline
  bool isClosed() const { return m_isClosed; }
  void setClosed(bool closed) {
    m_isClosed = closed;
    rebuildCurve();
  }

  // ========== TANGENT HANDLES (SolidWorks-style) ==========

  // Start tangent
  bool hasStartTangent() const { return m_useStartTangent; }
  void setStartTangent(const gp_Vec2d &tangent, double magnitude = 1.0);
  void clearStartTangent();
  gp_Vec2d startTangent() const { return m_startTangent; }
  double startTangentMagnitude() const { return m_startTangentMagnitude; }
  gp_Pnt2d startTangentHandle() const; // Returns handle position for UI

  // End tangent
  bool hasEndTangent() const { return m_useEndTangent; }
  void setEndTangent(const gp_Vec2d &tangent, double magnitude = 1.0);
  void clearEndTangent();
  gp_Vec2d endTangent() const { return m_endTangent; }
  double endTangentMagnitude() const { return m_endTangentMagnitude; }
  gp_Pnt2d endTangentHandle() const; // Returns handle position for UI

  // Set tangent from handle position (for dragging)
  void setStartTangentFromHandle(const gp_Pnt2d &handlePos);
  void setEndTangentFromHandle(const gp_Pnt2d &handlePos);

  // ========================================================

  // Geometry
  Handle(Geom2d_Curve) curve() const override;
  gp_Pnt2d startPoint() const override;
  gp_Pnt2d endPoint() const override;
  gp_Pnt2d midPoint() const override;
  double length() const override;

  // Point on curve
  gp_Pnt2d pointAtParameter(double u) const;
  gp_Vec2d tangentAtParameter(double u) const;
  double curvatureAtParameter(double u) const;

  // DOF: 2 per control point + 4 for tangents if used
  int baseDOF() const override;

  // Parameters
  int parameterCount() const override {
    return static_cast<int>(m_controlPoints.size() * 2);
  }
  double getParameter(int index) const override;
  void setParameter(int index, double value) override;

  // Clone
  SketchEntity::Ptr clone() const override;

  // Validation
  bool isValid() const override;

private:
  std::vector<gp_Pnt2d> m_controlPoints;
  SplineType m_splineType;
  int m_degree;
  bool m_isClosed;

  // Tangent handles
  gp_Vec2d m_startTangent;
  gp_Vec2d m_endTangent;
  double m_startTangentMagnitude;
  double m_endTangentMagnitude;
  bool m_useStartTangent;
  bool m_useEndTangent;

  mutable Handle(Geom2d_BSplineCurve) m_curve;
  mutable bool m_curveValid;

  void rebuildCurve() const;
};

} // namespace sketch
} // namespace opencad
