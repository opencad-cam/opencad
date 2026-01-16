#include "SketchPolygon.h"
#include <cmath>

namespace opencad {
namespace sketch {

SketchPolygon::SketchPolygon() : m_center(0, 0), m_vertex(1, 0), m_sides(6) {}

SketchPolygon::SketchPolygon(const gp_Pnt2d &center, const gp_Pnt2d &vertex,
                             int sides)
    : m_center(center), m_vertex(vertex), m_sides(sides) {
  if (m_sides < 3)
    m_sides = 3;
}

double SketchPolygon::length() const {
  auto verts = getVertices();
  if (verts.empty())
    return 0.0;

  double len = 0.0;
  for (size_t i = 0; i < verts.size(); ++i) {
    const auto &p1 = verts[i];
    const auto &p2 = verts[(i + 1) % verts.size()];
    len += p1.Distance(p2);
  }
  return len;
}

std::vector<gp_Pnt2d> SketchPolygon::getVertices() const {
  std::vector<gp_Pnt2d> verts;
  if (m_sides < 3)
    return verts;

  double radius = m_center.Distance(m_vertex);
  if (radius < 1e-6)
    return verts;

  // Calculate angle of the first vertex (m_vertex) relative to center
  double startAngle =
      std::atan2(m_vertex.Y() - m_center.Y(), m_vertex.X() - m_center.X());
  double stepAngle = 2.0 * M_PI / m_sides;

  verts.reserve(m_sides);
  for (int i = 0; i < m_sides; ++i) {
    double angle = startAngle + i * stepAngle;
    double x = m_center.X() + radius * std::cos(angle);
    double y = m_center.Y() + radius * std::sin(angle);
    verts.emplace_back(x, y);
  }
  return verts;
}

std::vector<SketchLine::Ptr> SketchPolygon::getEdges() const {
  std::vector<SketchLine::Ptr> edges;
  auto verts = getVertices();
  if (verts.size() < 3)
    return edges;

  edges.reserve(verts.size());
  for (size_t i = 0; i < verts.size(); ++i) {
    const auto &p1 = verts[i];
    const auto &p2 = verts[(i + 1) % verts.size()];
    auto line = std::make_shared<SketchLine>(p1, p2);
    line->setConstruction(isConstruction());
    edges.push_back(line);
  }
  return edges;
}

double SketchPolygon::getParameter(int index) const {
  switch (index) {
  case 0:
    return m_center.X();
  case 1:
    return m_center.Y();
  case 2:
    return m_vertex.X();
  case 3:
    return m_vertex.Y();
  default:
    return 0.0;
  }
}

void SketchPolygon::setParameter(int index, double value) {
  switch (index) {
  case 0:
    m_center.SetX(value);
    break;
  case 1:
    m_center.SetY(value);
    break;
  case 2:
    m_vertex.SetX(value);
    break;
  case 3:
    m_vertex.SetY(value);
    break;
  }
}

SketchEntity::Ptr SketchPolygon::clone() const {
  auto copy = std::make_shared<SketchPolygon>(m_center, m_vertex, m_sides);
  copy->setId(m_id);
  copy->setConstruction(m_isConstruction);
  copy->setSelected(m_isSelected);
  return copy;
}

bool SketchPolygon::isValid() const {
  return m_sides >= 3 && m_center.Distance(m_vertex) > 1e-6;
}

} // namespace sketch
} // namespace opencad
