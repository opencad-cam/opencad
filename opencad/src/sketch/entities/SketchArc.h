/**
 * @file SketchArc.h
 * @brief 2D Arc entity for sketches
 */

#pragma once

#include "SketchEntity.h"
#include <Geom2d_TrimmedCurve.hxx>
#include <gp_Circ2d.hxx>
#include <gp_Pnt2d.hxx>


namespace opencad {
namespace sketch {

/**
 * @brief 2D Arc entity
 *
 * An arc can be defined by:
 * - Center, radius, start angle, end angle (4 DOF: cx, cy, r, sweep)
 * - Three points: start, end, through
 *
 * Arc has 5 degrees of freedom: center(x,y), radius, startAngle, endAngle
 */
class SketchArc : public SketchEntity {
public:
  using Ptr = std::shared_ptr<SketchArc>;

  SketchArc();
  SketchArc(const gp_Pnt2d &center, double radius, double startAngle,
            double endAngle);
  // Three-point arc constructor: start, end, through (mid point)
  SketchArc(const gp_Pnt2d &start, const gp_Pnt2d &end,
            const gp_Pnt2d &through);

  // Type
  EntityType type() const override { return EntityType::Arc; }
  std::string typeName() const override { return "Arc"; }

  // Center and radius (parametric representation)
  gp_Pnt2d center() const { return m_center; }
  double radius() const { return m_radius; }
  void setCenter(const gp_Pnt2d &center) { m_center = center; }
  void setCenter(double x, double y) { m_center.SetCoord(x, y); }
  void setRadius(double radius) { m_radius = radius; }

  // Angles (in radians)
  double startAngle() const { return m_startAngle; }
  double endAngle() const { return m_endAngle; }
  void setStartAngle(double angle) { m_startAngle = angle; }
  void setEndAngle(double angle) { m_endAngle = angle; }
  double sweepAngle() const; // Can be negative for CW arcs

  // Three-point arc data
  bool hasThreePointData() const { return m_hasThreePointData; }
  gp_Pnt2d arcStart() const { return m_arcStart; }
  gp_Pnt2d arcEnd() const { return m_arcEnd; }
  gp_Pnt2d arcThrough() const { return m_arcThrough; }

  // Geometry
  Handle(Geom2d_Curve) curve() const override;
  gp_Pnt2d startPoint() const override;
  gp_Pnt2d endPoint() const override;
  gp_Pnt2d midPoint() const override;
  double length() const override;

  gp_Circ2d circle2d() const;

  // DOF: center(x,y) + radius + 2 angles = 5
  int baseDOF() const override { return 5; }

  // Parameters
  int parameterCount() const override { return 5; }
  double getParameter(int index) const override;
  void setParameter(int index, double value) override;

  // Clone
  SketchEntity::Ptr clone() const override;

  // Validation
  bool isValid() const override;

  // Utilities
  gp_Pnt2d pointAtAngle(double angle) const;
  gp_Pnt2d pointAtParameter(double t) const; // t in [0, 1]
  double distanceToPoint(const gp_Pnt2d &point) const;

private:
  // Parametric representation
  gp_Pnt2d m_center;
  double m_radius;
  double m_startAngle;
  double m_endAngle;

  // Original three-point data (for wire/edge building)
  bool m_hasThreePointData;
  gp_Pnt2d m_arcStart;
  gp_Pnt2d m_arcEnd;
  gp_Pnt2d m_arcThrough;

  // Compute center/radius from three points
  void computeFromThreePoints(const gp_Pnt2d &p1, const gp_Pnt2d &p2,
                              const gp_Pnt2d &p3);
};

} // namespace sketch
} // namespace opencad
