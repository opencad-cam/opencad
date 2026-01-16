/**
 * @file SketchPolygon.h
 * @brief Regular polygon entity defined by center and a vertex
 */

#pragma once

#include "SketchEntity.h"
#include "SketchLine.h"
#include <gp_Pnt2d.hxx>
#include <vector>

namespace opencad {
namespace sketch {

class SketchPolygon : public SketchEntity {
public:
  using Ptr = std::shared_ptr<SketchPolygon>;

  SketchPolygon();
  SketchPolygon(const gp_Pnt2d &center, const gp_Pnt2d &vertex, int sides);

  // Type
  EntityType type() const override { return EntityType::Polygon; }
  std::string typeName() const override { return "Polygon"; }

  // Properties
  gp_Pnt2d center() const { return m_center; }
  gp_Pnt2d vertex() const { return m_vertex; }
  int sides() const { return m_sides; }

  void setCenter(const gp_Pnt2d &pt) { m_center = pt; }
  void setVertex(const gp_Pnt2d &pt) { m_vertex = pt; }
  void setSides(int sides) { m_sides = sides; }

  // Geometry
  Handle(Geom2d_Curve) curve() const override { return Handle(Geom2d_Curve)(); }
  gp_Pnt2d startPoint() const override { return m_vertex; }
  gp_Pnt2d endPoint() const override { return m_vertex; } // Closed
  gp_Pnt2d midPoint() const override { return m_center; }
  double length() const override; // Perimeter

  // Helper methods
  std::vector<gp_Pnt2d> getVertices() const;
  std::vector<SketchLine::Ptr> getEdges() const;

  // Solver Interface
  // 4 Parameters: CenterX, CenterY, VertexX, VertexY
  int baseDOF() const override { return 4; }
  int parameterCount() const override { return 4; }
  double getParameter(int index) const override;
  void setParameter(int index, double value) override;

  // Clone
  SketchEntity::Ptr clone() const override;

  // Validation
  bool isValid() const override;

private:
  gp_Pnt2d m_center;
  gp_Pnt2d m_vertex;
  int m_sides;
};

} // namespace sketch
} // namespace opencad
