/**
 * @file SketchView2D.cpp
 * @brief 2D Sketch view implementation
 */

#include "SketchView2D.h"
#include "sketch/Sketch.h"
#include "sketch/entities/SketchArc.h"
#include "sketch/entities/SketchCircle.h"
#include "sketch/entities/SketchEllipse.h"
#include "sketch/entities/SketchEntity.h"
#include "sketch/entities/SketchLine.h"
#include "sketch/entities/SketchPoint.h"

#include "sketch/entities/SketchPolygon.h"
#include "sketch/entities/SketchRectangle.h"
#include "sketch/entities/SketchSlot.h"
#include "sketch/entities/SketchSpline.h"

// Constraints for auto-constraint feature
#include "sketch/constraints/CoincidentConstraint.h"
#include "sketch/constraints/HorizontalConstraint.h"
#include "sketch/constraints/VerticalConstraint.h"

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>
#include <limits>

// OpenCASCADE includes for profile selection
#include <BRepAdaptor_Curve.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepClass_FaceClassifier.hxx>
#include <BRepGProp.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <BRep_Tool.hxx>
#include <GProp_GProps.hxx>
#include <Geom_Curve.hxx>
#include <TopAbs_State.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <gp_Pnt.hxx>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace opencad {
namespace ui {

SketchView2D::SketchView2D(QWidget *parent) : QWidget(parent) {
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);
  setMinimumSize(400, 300);
  setAutoFillBackground(true);

  QPalette pal = palette();
  pal.setColor(QPalette::Window, Qt::white);
  setPalette(pal);

  // Center origin in view
  m_offset = QPointF(200, 150);
}

SketchView2D::~SketchView2D() = default;

void SketchView2D::setSketch(std::shared_ptr<sketch::Sketch> sketch) {
  m_sketch = sketch;

  // Reset undo history and save initial state
  m_sketchHistory.clear();
  m_sketchHistoryIndex = -1;
  if (m_sketch) {
    saveSketchCheckpoint("Initial");
  }

  update();
}

void SketchView2D::setTool(SketchToolType tool) {
  if (m_currentTool != tool) {
    cancelCurrentEntity();
    m_currentTool = tool;
    m_escPressedOnce = false; // Reset ESC flag when switching tools
    emit toolChanged(tool);
    update();
  }
}

void SketchView2D::fitAll() {
  // Reset to default view
  m_scale = 5.0;
  m_offset = QPointF(width() / 2.0, height() / 2.0);
  update();
}

void SketchView2D::zoomIn() {
  m_scale *= 1.2;
  update();
}

void SketchView2D::zoomOut() {
  m_scale /= 1.2;
  if (m_scale < 0.1)
    m_scale = 0.1;
  update();
}

void SketchView2D::resetView() {
  m_scale = 5.0;
  m_offset = QPointF(width() / 2.0, height() / 2.0);
  update();
}

// Coordinate conversion
QPointF SketchView2D::worldToScreen(const gp_Pnt2d &world) const {
  return QPointF(world.X() * m_scale + m_offset.x(),
                 -world.Y() * m_scale + m_offset.y() // Y is flipped
  );
}

gp_Pnt2d SketchView2D::screenToWorld(const QPointF &screen) const {
  return gp_Pnt2d((screen.x() - m_offset.x()) / m_scale,
                  -(screen.y() - m_offset.y()) / m_scale // Y is flipped
  );
}

// ============ ADVANCED SNAP SYSTEM ============

SketchView2D::SnapResult
SketchView2D::findBestSnap(const gp_Pnt2d &point) const {
  SnapResult result;
  result.point = point;
  result.type = SnapType::None;

  if (!m_sketch) {
    // Grid snap only
    if (m_snapToGrid) {
      result.point = gp_Pnt2d(std::round(point.X() / m_gridSize) * m_gridSize,
                              std::round(point.Y() / m_gridSize) * m_gridSize);
      result.type = SnapType::Grid;
    }
    return result;
  }

  double snapWorldRadius = m_snapRadius / m_scale;
  double bestDist = snapWorldRadius;

  // Priority 1: Endpoints (highest priority)
  for (const gp_Pnt2d &ep : findEndpoints()) {
    double dist = point.Distance(ep);
    if (dist < bestDist) {
      bestDist = dist;
      result.point = ep;
      result.type = SnapType::Endpoint;
    }
  }

  // Priority 2: Midpoints
  for (const gp_Pnt2d &mp : findMidpoints()) {
    double dist = point.Distance(mp);
    if (dist < bestDist) {
      bestDist = dist;
      result.point = mp;
      result.type = SnapType::Midpoint;
    }
  }

  // Priority 3: Centers (circles, arcs)
  for (const gp_Pnt2d &cp : findCenters()) {
    double dist = point.Distance(cp);
    if (dist < bestDist) {
      bestDist = dist;
      result.point = cp;
      result.type = SnapType::Center;
    }
  }

  // Priority 4: Intersections
  for (const gp_Pnt2d &ip : findIntersections()) {
    double dist = point.Distance(ip);
    if (dist < bestDist) {
      bestDist = dist;
      result.point = ip;
      result.type = SnapType::Intersection;
    }
  }

  // Priority 5: On-curve snap (only if no other snap found)
  if (result.type == SnapType::None) {
    for (auto &entity : m_sketch->entities()) {
      gp_Pnt2d onCurve = findNearestOnCurve(point, entity.get());
      double dist = point.Distance(onCurve);
      if (dist < bestDist && dist < snapWorldRadius * 0.7) {
        bestDist = dist;
        result.point = onCurve;
        result.type = SnapType::OnCurve;
        result.entity = entity.get();
      }
    }
  }

  // Fallback: Grid snap
  if (result.type == SnapType::None && m_snapToGrid) {
    result.point = gp_Pnt2d(std::round(point.X() / m_gridSize) * m_gridSize,
                            std::round(point.Y() / m_gridSize) * m_gridSize);
    result.type = SnapType::Grid;
  }

  return result;
}

std::vector<gp_Pnt2d> SketchView2D::findEndpoints() const {
  std::vector<gp_Pnt2d> endpoints;
  if (!m_sketch)
    return endpoints;

  for (const auto &entity : m_sketch->entities()) {
    if (entity->type() == sketch::EntityType::Line ||
        entity->type() == sketch::EntityType::Arc ||
        entity->type() == sketch::EntityType::Spline) {
      endpoints.push_back(entity->startPoint());
      endpoints.push_back(entity->endPoint());
    } else if (entity->type() == sketch::EntityType::Rectangle) {
      auto *rect = dynamic_cast<sketch::SketchRectangle *>(entity.get());
      if (rect) {
        auto corners = rect->corners();
        for (const auto &c : corners) {
          endpoints.push_back(c);
        }
      }
    }
  }
  return endpoints;
}

std::vector<gp_Pnt2d> SketchView2D::findMidpoints() const {
  std::vector<gp_Pnt2d> midpoints;
  if (!m_sketch)
    return midpoints;

  for (const auto &entity : m_sketch->entities()) {
    if (entity->type() == sketch::EntityType::Line) {
      midpoints.push_back(entity->midPoint());
    } else if (entity->type() == sketch::EntityType::Rectangle) {
      auto *rect = dynamic_cast<sketch::SketchRectangle *>(entity.get());
      if (rect) {
        // Edge midpoints using corner1 and corner2
        gp_Pnt2d c1 = rect->corner1();
        gp_Pnt2d c2 = rect->corner2();
        double cx = (c1.X() + c2.X()) / 2;
        double cy = (c1.Y() + c2.Y()) / 2;
        // Midpoints of 4 edges
        midpoints.push_back(gp_Pnt2d(cx, c1.Y())); // bottom edge
        midpoints.push_back(gp_Pnt2d(cx, c2.Y())); // top edge
        midpoints.push_back(gp_Pnt2d(c1.X(), cy)); // left edge
        midpoints.push_back(gp_Pnt2d(c2.X(), cy)); // right edge
      }
    }
  }
  return midpoints;
}

std::vector<gp_Pnt2d> SketchView2D::findCenters() const {
  std::vector<gp_Pnt2d> centers;
  if (!m_sketch)
    return centers;

  for (const auto &entity : m_sketch->entities()) {
    if (entity->type() == sketch::EntityType::Circle) {
      auto *circle = dynamic_cast<sketch::SketchCircle *>(entity.get());
      if (circle)
        centers.push_back(circle->center());
    } else if (entity->type() == sketch::EntityType::Arc) {
      auto *arc = dynamic_cast<sketch::SketchArc *>(entity.get());
      if (arc)
        centers.push_back(arc->center());
    } else if (entity->type() == sketch::EntityType::Ellipse) {
      auto *ellipse = dynamic_cast<sketch::SketchEllipse *>(entity.get());
      if (ellipse)
        centers.push_back(ellipse->center());
    }
  }
  return centers;
}

std::vector<gp_Pnt2d> SketchView2D::findIntersections() const {
  std::vector<gp_Pnt2d> intersections;
  if (!m_sketch)
    return intersections;

  const auto &entities = m_sketch->entities();

  // Check all pairs for line-line intersections
  for (size_t i = 0; i < entities.size(); ++i) {
    for (size_t j = i + 1; j < entities.size(); ++j) {
      if (entities[i]->type() == sketch::EntityType::Line &&
          entities[j]->type() == sketch::EntityType::Line) {
        auto *line1 = dynamic_cast<sketch::SketchLine *>(entities[i].get());
        auto *line2 = dynamic_cast<sketch::SketchLine *>(entities[j].get());

        if (line1 && line2) {
          // Line intersection formula
          double x1 = line1->startPoint().X(), y1 = line1->startPoint().Y();
          double x2 = line1->endPoint().X(), y2 = line1->endPoint().Y();
          double x3 = line2->startPoint().X(), y3 = line2->startPoint().Y();
          double x4 = line2->endPoint().X(), y4 = line2->endPoint().Y();

          double denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
          if (std::abs(denom) > 1e-10) {
            double t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denom;
            double u = -((x1 - x2) * (y1 - y3) - (y1 - y2) * (x1 - x3)) / denom;

            // Check if intersection is within both line segments (extended
            // slightly)
            if (t >= -0.1 && t <= 1.1 && u >= -0.1 && u <= 1.1) {
              double ix = x1 + t * (x2 - x1);
              double iy = y1 + t * (y2 - y1);
              intersections.push_back(gp_Pnt2d(ix, iy));
            }
          }
        }
      }
    }
  }
  return intersections;
}

gp_Pnt2d SketchView2D::findNearestOnCurve(const gp_Pnt2d &point,
                                          sketch::SketchEntity *entity) const {
  if (!entity)
    return point;

  if (entity->type() == sketch::EntityType::Line) {
    auto *line = dynamic_cast<sketch::SketchLine *>(entity);
    if (line) {
      gp_Pnt2d p1 = line->startPoint();
      gp_Pnt2d p2 = line->endPoint();

      // Project point onto line
      double dx = p2.X() - p1.X();
      double dy = p2.Y() - p1.Y();
      double len2 = dx * dx + dy * dy;
      if (len2 < 1e-10)
        return p1;

      double t = ((point.X() - p1.X()) * dx + (point.Y() - p1.Y()) * dy) / len2;
      t = std::max(0.0, std::min(1.0, t));
      return gp_Pnt2d(p1.X() + t * dx, p1.Y() + t * dy);
    }
  } else if (entity->type() == sketch::EntityType::Circle) {
    auto *circle = dynamic_cast<sketch::SketchCircle *>(entity);
    if (circle) {
      gp_Pnt2d c = circle->center();
      double r = circle->radius();
      double dx = point.X() - c.X();
      double dy = point.Y() - c.Y();
      double dist = std::sqrt(dx * dx + dy * dy);
      if (dist < 1e-10)
        return gp_Pnt2d(c.X() + r, c.Y());
      return gp_Pnt2d(c.X() + r * dx / dist, c.Y() + r * dy / dist);
    }
  }

  return point;
}

void SketchView2D::drawSnapIndicator(QPainter &painter,
                                     const SnapResult &snap) {
  if (snap.type == SnapType::None || snap.type == SnapType::Grid)
    return;

  QPointF screenPos = worldToScreen(snap.point);
  painter.save();

  switch (snap.type) {
  case SnapType::Endpoint:
    painter.setPen(QPen(m_snapEndpointColor, 2));
    painter.setBrush(Qt::transparent);
    painter.drawRect(QRectF(screenPos.x() - 6, screenPos.y() - 6, 12, 12));
    break;
  case SnapType::Midpoint:
    painter.setPen(QPen(m_snapMidpointColor, 2));
    painter.setBrush(Qt::transparent);
    {
      QPolygonF triangle;
      triangle << QPointF(screenPos.x(), screenPos.y() - 8)
               << QPointF(screenPos.x() - 7, screenPos.y() + 5)
               << QPointF(screenPos.x() + 7, screenPos.y() + 5);
      painter.drawPolygon(triangle);
    }
    break;
  case SnapType::Center:
    painter.setPen(QPen(m_snapCenterColor, 2));
    painter.setBrush(Qt::transparent);
    painter.drawEllipse(screenPos, 6, 6);
    painter.drawLine(screenPos.x() - 8, screenPos.y(), screenPos.x() + 8,
                     screenPos.y());
    painter.drawLine(screenPos.x(), screenPos.y() - 8, screenPos.x(),
                     screenPos.y() + 8);
    break;
  case SnapType::Intersection:
    painter.setPen(QPen(m_snapIntersectionColor, 2));
    painter.drawLine(screenPos.x() - 6, screenPos.y() - 6, screenPos.x() + 6,
                     screenPos.y() + 6);
    painter.drawLine(screenPos.x() - 6, screenPos.y() + 6, screenPos.x() + 6,
                     screenPos.y() - 6);
    break;
  case SnapType::OnCurve:
    painter.setPen(QPen(m_snapOnCurveColor, 2));
    painter.setBrush(m_snapOnCurveColor);
    painter.drawEllipse(screenPos, 4, 4);
    break;
  default:
    break;
  }

  painter.restore();
}

// Paint event
void SketchView2D::paintEvent(QPaintEvent * /*event*/) {
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // Draw solid background - CAD style dark blue-gray
  QLinearGradient bgGradient(0, 0, 0, height());
  bgGradient.setColorAt(0, QColor(40, 45, 55)); // Darker at top
  bgGradient.setColorAt(1, QColor(55, 65, 80)); // Slightly lighter at bottom
  painter.fillRect(rect(), bgGradient);

  // Draw grid
  if (m_showGrid) {
    drawGrid(painter);
  }

  // Draw closed profile faces (filled areas)
  drawClosedProfileFaces(painter);

  // Draw entities
  drawEntities(painter);

  // Draw profile overlays in ProfileSelect mode
  if (m_currentTool == SketchToolType::ProfileSelect && m_sketch) {
    drawProfileOverlays(painter);
  }

  // Draw dimensions
  if (m_showDimensions) {
    drawDimensions(painter);
  }

  // Draw preview (current drawing)
  if (m_isDrawing) {
    drawPreview(painter);
  }

  // Draw selection visual feedback
  if (m_currentTool == SketchToolType::Select) {
    drawSelectionHandles(painter);
    drawBoundingBox(painter);
    drawBoxSelection(painter);
  }

  // Draw snap indicator
  drawSnapIndicator(painter, m_currentSnap);

  // Draw cursor position
  drawCursor(painter);
}

void SketchView2D::drawGrid(QPainter &painter) {
  painter.save();

  // Calculate visible range
  gp_Pnt2d topLeft = screenToWorld(QPointF(0, 0));
  gp_Pnt2d bottomRight = screenToWorld(QPointF(width(), height()));

  double startX = std::floor(topLeft.X() / m_gridSize) * m_gridSize;
  double endX = std::ceil(bottomRight.X() / m_gridSize) * m_gridSize;
  double startY = std::floor(bottomRight.Y() / m_gridSize) * m_gridSize;
  double endY = std::ceil(topLeft.Y() / m_gridSize) * m_gridSize;

  // Draw minor grid
  painter.setPen(QPen(m_gridColor, 1, Qt::DotLine));
  for (double x = startX; x <= endX; x += m_gridSize) {
    QPointF p1 = worldToScreen(gp_Pnt2d(x, startY));
    QPointF p2 = worldToScreen(gp_Pnt2d(x, endY));
    painter.drawLine(p1, p2);
  }
  for (double y = startY; y <= endY; y += m_gridSize) {
    QPointF p1 = worldToScreen(gp_Pnt2d(startX, y));
    QPointF p2 = worldToScreen(gp_Pnt2d(endX, y));
    painter.drawLine(p1, p2);
  }

  // Draw origin axes
  painter.setPen(QPen(m_axisXColor, 2));
  QPointF ox1 = worldToScreen(gp_Pnt2d(startX, 0));
  QPointF ox2 = worldToScreen(gp_Pnt2d(endX, 0));
  painter.drawLine(ox1, ox2);

  painter.setPen(QPen(m_axisYColor, 2));
  QPointF oy1 = worldToScreen(gp_Pnt2d(0, startY));
  QPointF oy2 = worldToScreen(gp_Pnt2d(0, endY));
  painter.drawLine(oy1, oy2);

  // Draw origin marker
  QPointF origin = worldToScreen(gp_Pnt2d(0, 0));
  painter.setPen(QPen(Qt::black, 2));
  painter.drawEllipse(origin, 5, 5);

  painter.restore();
}

void SketchView2D::drawEntities(QPainter &painter) {
  if (!m_sketch)
    return;

  for (const auto &entity : m_sketch->entities()) {
    drawEntity(painter, entity.get());
  }
}

// Draw filled faces for closed profiles
void SketchView2D::drawClosedProfileFaces(QPainter &painter) {
  if (!m_sketch)
    return;

  // Detect all closed profiles
  auto closedProfiles = m_sketch->detectClosedProfiles();
  if (closedProfiles.empty())
    return;

  // Set semi-transparent fill color (CAD-style light blue)
  QColor fillColor(100, 150, 200, 60); // RGBA: light blue with alpha
  painter.setBrush(QBrush(fillColor));
  painter.setPen(Qt::NoPen); // No outline for fill

  // Draw each closed profile as a filled polygon
  for (const auto &wire : closedProfiles) {
    try {
      // Extract edges from wire
      TopExp_Explorer edgeExp(wire, TopAbs_EDGE);
      QPolygonF polygon;

      while (edgeExp.More()) {
        TopoDS_Edge edge = static_cast<const TopoDS_Edge &>(edgeExp.Current());

        // Get edge curve and sample points
        Standard_Real first, last;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);

        if (!curve.IsNull()) {
          // Sample points along the edge
          int numSamples = 20;
          for (int i = 0; i <= numSamples; ++i) {
            double param = first + (last - first) * i / numSamples;
            gp_Pnt point3D;
            curve->D0(param, point3D);

            // Convert 3D point to 2D sketch coordinates
            // Assuming sketch is on XY plane
            gp_Pnt2d point2D(point3D.X(), point3D.Y());
            QPointF screenPt = worldToScreen(point2D);
            polygon << screenPt;
          }
        }

        edgeExp.Next();
      }

      // Draw filled polygon
      if (!polygon.isEmpty()) {
        painter.drawPolygon(polygon);
      }
    } catch (...) {
      // Skip invalid profiles
      continue;
    }
  }
}

void SketchView2D::drawEntity(QPainter &painter,
                              const sketch::SketchEntity *entity) {
  if (!entity)
    return;

  painter.save();

  // Set color based on selection/hover
  QColor color = m_entityColor;
  int lineWidth = 2;

  if (entity == m_selectedEntity) {
    color = m_selectedColor;
    lineWidth = 3;
  } else if (entity == m_hoveredEntity) {
    color = QColor(100, 150, 255);
    lineWidth = 2;
  }

  if (entity->isConstruction()) {
    painter.setPen(QPen(color, lineWidth, Qt::DashLine));
  } else {
    painter.setPen(QPen(color, lineWidth));
  }

  // Draw based on entity type
  switch (entity->type()) {
  case sketch::EntityType::Point: {
    auto *pt = static_cast<const sketch::SketchPoint *>(entity);
    QPointF screenPt = worldToScreen(pt->position());
    painter.drawEllipse(screenPt, 4, 4);
    break;
  }
  case sketch::EntityType::Line: {
    auto *line = static_cast<const sketch::SketchLine *>(entity);
    QPointF p1 = worldToScreen(line->startPoint());
    QPointF p2 = worldToScreen(line->endPoint());
    painter.drawLine(p1, p2);

    // Draw endpoints
    painter.setBrush(color);
    painter.drawEllipse(p1, 3, 3);
    painter.drawEllipse(p2, 3, 3);
    break;
  }
  case sketch::EntityType::Circle: {
    auto *circle = static_cast<const sketch::SketchCircle *>(entity);
    QPointF center = worldToScreen(circle->center());
    double radiusPixels = circle->radius() * m_scale;
    painter.drawEllipse(center, radiusPixels, radiusPixels);

    // Draw center
    painter.setBrush(color);
    painter.drawEllipse(center, 3, 3);
    break;
  }
  case sketch::EntityType::Rectangle: {
    auto *rect = static_cast<const sketch::SketchRectangle *>(entity);
    QPointF c1 = worldToScreen(rect->corner1());
    QPointF c2 = worldToScreen(rect->corner2());
    painter.drawRect(QRectF(c1, c2).normalized());
    break;
  }
  case sketch::EntityType::Arc: {
    auto *arc = static_cast<const sketch::SketchArc *>(entity);

    // Draw arc using line segments (for debugging - this guarantees we draw the
    // calculated path)
    QPainterPath path;
    QPointF firstPt = worldToScreen(arc->pointAtParameter(0.0));
    path.moveTo(firstPt);
    for (double t = 0.02; t <= 1.0; t += 0.02) {
      QPointF pt = worldToScreen(arc->pointAtParameter(t));
      path.lineTo(pt);
    }
    painter.drawPath(path);

    // Draw the 3 arc points for visual verification
    if (arc->hasThreePointData()) {
      QPointF startPt = worldToScreen(arc->arcStart());
      QPointF throughPt = worldToScreen(arc->arcThrough());
      QPointF endPt = worldToScreen(arc->arcEnd());

      painter.setBrush(Qt::green);
      painter.setPen(Qt::NoPen);
      painter.drawEllipse(startPt, 5, 5); // Green = Start

      painter.setBrush(Qt::yellow);
      painter.drawEllipse(throughPt, 5, 5); // Yellow = Through

      painter.setBrush(Qt::red);
      painter.drawEllipse(endPt, 5, 5); // Red = End
    }
    break;
  }
  case sketch::EntityType::Ellipse: {
    auto *ellipse = static_cast<const sketch::SketchEllipse *>(entity);
    QPointF center = worldToScreen(ellipse->center());
    double majorPixels = ellipse->majorRadius() * m_scale;
    double minorPixels = ellipse->minorRadius() * m_scale;
    painter.drawEllipse(center, majorPixels, minorPixels);

    // Draw center
    painter.setBrush(color);
    painter.drawEllipse(center, 3, 3);
    break;
  }
  case sketch::EntityType::Spline: {
    auto *spline = static_cast<const sketch::SketchSpline *>(entity);
    const auto &points = spline->controlPoints();
    if (points.size() >= 2) {
      // Draw curve using multiple segments
      QPainterPath path;
      QPointF first = worldToScreen(spline->pointAtParameter(0.0));
      path.moveTo(first);
      for (double t = 0.02; t <= 1.0; t += 0.02) {
        QPointF pt = worldToScreen(spline->pointAtParameter(t));
        path.lineTo(pt);
      }
      painter.drawPath(path);

      // Draw fit points (green circles)
      painter.setBrush(QColor(0, 180, 0));
      painter.setPen(QPen(QColor(0, 150, 0), 1));
      for (const auto &pt : points) {
        QPointF screenPt = worldToScreen(pt);
        painter.drawEllipse(screenPt, 4, 4);
      }

      // Draw tangent handles (SolidWorks-style)
      if (spline->hasStartTangent() || spline->hasEndTangent()) {
        QPen handlePen(QColor(0, 100, 255), 1, Qt::DashLine);
        painter.setPen(handlePen);
        painter.setBrush(QColor(0, 100, 255));

        // Start tangent handle
        if (spline->hasStartTangent()) {
          QPointF startPt = worldToScreen(spline->startPoint());
          QPointF handlePt = worldToScreen(spline->startTangentHandle());
          painter.drawLine(startPt, handlePt);
          painter.drawRect(QRectF(handlePt.x() - 4, handlePt.y() - 4, 8, 8));
        }

        // End tangent handle
        if (spline->hasEndTangent()) {
          QPointF endPt = worldToScreen(spline->endPoint());
          QPointF handlePt = worldToScreen(spline->endTangentHandle());
          painter.drawLine(endPt, handlePt);
          painter.drawRect(QRectF(handlePt.x() - 4, handlePt.y() - 4, 8, 8));
        }
      }
    }
    break;
  }
  case sketch::EntityType::Polygon: {
    auto *polygon = static_cast<const sketch::SketchPolygon *>(entity);
    const auto vertices = polygon->getVertices();
    if (vertices.size() >= 3) {
      QPolygonF poly;
      for (const auto &v : vertices) {
        poly << worldToScreen(v);
      }
      painter.drawPolygon(poly);

      // Draw center
      painter.setBrush(color);
      QPointF centerPt = worldToScreen(polygon->center());
      painter.drawEllipse(centerPt, 3, 3);
    }
    break;
  }
  case sketch::EntityType::Slot: {
    auto *slot = static_cast<const sketch::SketchSlot *>(entity);
    gp_Pnt2d c1 = slot->center1();
    gp_Pnt2d c2 = slot->center2();
    double width = slot->width();
    double halfWidth = width / 2.0;
    double screenHalfWidth = halfWidth * m_scale;

    // Convert centers to screen first
    QPointF sc1 = worldToScreen(c1);
    QPointF sc2 = worldToScreen(c2);

    // Calculate direction in SCREEN coordinates
    double sdx = sc2.x() - sc1.x();
    double sdy = sc2.y() - sc1.y();
    double slen = std::sqrt(sdx * sdx + sdy * sdy);

    if (slen > 1) {
      // Screen angle from sc1 to sc2
      double screenAngle = std::atan2(sdy, sdx);
      double screenAngleDeg = screenAngle * 180.0 / M_PI;

      // Perpendicular direction in screen coords
      double perpAngle = screenAngle + M_PI / 2;
      double spx = std::cos(perpAngle) * screenHalfWidth;
      double spy = std::sin(perpAngle) * screenHalfWidth;

      // Calculate 4 corner points in SCREEN coordinates
      QPointF sp1(sc1.x() + spx, sc1.y() + spy); // c1 side A
      QPointF sp2(sc1.x() - spx, sc1.y() - spy); // c1 side B
      QPointF sp3(sc2.x() - spx, sc2.y() - spy); // c2 side B
      QPointF sp4(sc2.x() + spx, sc2.y() + spy); // c2 side A

      // Draw the two parallel lines
      painter.drawLine(sp1, sp4); // side A line
      painter.drawLine(sp2, sp3); // side B line

      // Arc at c1: semicircle facing AWAY from c2 (outward)
      // Direction from c1 to c2 is screenAngle, so arc should be centered at
      // screenAngle + 180 Qt: positive angles = counter-clockwise, start angle
      // at 3 o'clock = 0
      QRectF arc1Rect(sc1.x() - screenHalfWidth, sc1.y() - screenHalfWidth,
                      screenHalfWidth * 2, screenHalfWidth * 2);
      // Arc starts at perpendicular to direction (90° offset) and goes 180°
      // To face away from c2, the arc center direction is screenAngle + 180
      int arc1Start = static_cast<int>((-screenAngleDeg + 90) * 16);
      painter.drawArc(arc1Rect, arc1Start, 180 * 16);

      // Arc at c2: semicircle facing AWAY from c1 (outward)
      // Direction from c2 to c1 is screenAngle + 180, so arc faces that way
      QRectF arc2Rect(sc2.x() - screenHalfWidth, sc2.y() - screenHalfWidth,
                      screenHalfWidth * 2, screenHalfWidth * 2);
      int arc2Start = static_cast<int>((-screenAngleDeg - 90) * 16);
      painter.drawArc(arc2Rect, arc2Start, 180 * 16);

      // Draw centers
      painter.setBrush(color);
      painter.drawEllipse(sc1, 3, 3);
      painter.drawEllipse(sc2, 3, 3);
    }
    break;
  }
  default:
    break;
  }

  // Draw handles if selected
  if (entity == m_selectedEntity) {
    painter.setPen(Qt::black);
    painter.setBrush(QColor(0, 120, 255)); // Blue handles

    // Determine highlighted point
    int highlightPointIdx = -1;
    if (m_isDraggingPoint) {
      highlightPointIdx = m_selectedControlPointIndex;
    }

    auto drawHandle = [&](const gp_Pnt2d &pt, int idx) {
      QPointF sPt = worldToScreen(pt);
      if (idx == highlightPointIdx)
        painter.setBrush(Qt::yellow);
      else
        painter.setBrush(QColor(0, 120, 255)); // Blue default
      painter.drawRect(QRectF(sPt.x() - 4, sPt.y() - 4, 8, 8));
    };

    switch (entity->type()) {
    case sketch::EntityType::Line: {
      auto *l = static_cast<const sketch::SketchLine *>(entity);
      drawHandle(l->startPoint(), 0);
      drawHandle(l->endPoint(), 1);
      break;
    }
    case sketch::EntityType::Circle: {
      auto *c = static_cast<const sketch::SketchCircle *>(entity);
      drawHandle(c->center(), 0);
      break;
    }
    case sketch::EntityType::Arc: {
      auto *a = static_cast<const sketch::SketchArc *>(entity);
      drawHandle(a->center(), 0);
      drawHandle(a->startPoint(), 1);
      drawHandle(a->endPoint(), 2);
      break;
    }
    }
  }

  painter.restore();
}

void SketchView2D::drawDimensions(QPainter &painter) {
  if (!m_sketch)
    return;

  painter.save();
  painter.setPen(QPen(m_dimensionColor, 1));
  painter.setFont(QFont("Arial", 9));

  for (const auto &entity : m_sketch->entities()) {
    switch (entity->type()) {
    case sketch::EntityType::Line: {
      auto *line = static_cast<const sketch::SketchLine *>(entity.get());
      QPointF p1 = worldToScreen(line->startPoint());
      QPointF p2 = worldToScreen(line->endPoint());
      QPointF mid = (p1 + p2) / 2.0;

      double length = line->length();
      QString dimText = QString::number(length, 'f', 1);

      // Offset text from line
      QPointF offset(10, -10);
      painter.drawText(mid + offset, dimText);
      break;
    }
    case sketch::EntityType::Circle: {
      auto *circle = static_cast<const sketch::SketchCircle *>(entity.get());
      QPointF center = worldToScreen(circle->center());

      QString dimText = "R" + QString::number(circle->radius(), 'f', 1);
      painter.drawText(center + QPointF(10, -10), dimText);
      break;
    }
    case sketch::EntityType::Arc: {
      auto *arc = static_cast<const sketch::SketchArc *>(entity.get());
      QPointF center = worldToScreen(arc->center());

      QString dimText = "R" + QString::number(arc->radius(), 'f', 1);
      painter.drawText(center + QPointF(10, -10), dimText);
      break;
    }
    default:
      break;
    }
  }

  painter.restore();
}

void SketchView2D::drawPreview(QPainter &painter) {
  painter.save();
  painter.setPen(QPen(m_previewColor, 2, Qt::DashLine));

  QPointF start = worldToScreen(m_startPoint);
  QPointF current = worldToScreen(m_currentPoint);

  switch (m_currentTool) {
  case SketchToolType::Line:
    painter.drawLine(start, current);
    // Show length preview
    {
      double dx = m_currentPoint.X() - m_startPoint.X();
      double dy = m_currentPoint.Y() - m_startPoint.Y();
      double length = std::sqrt(dx * dx + dy * dy);
      QPointF mid = (start + current) / 2.0;
      painter.drawText(mid + QPointF(5, -5), QString::number(length, 'f', 1));
    }
    break;

  case SketchToolType::Rectangle:
    painter.drawRect(QRectF(start, current).normalized());
    // Show dimensions
    {
      double w = std::abs(m_currentPoint.X() - m_startPoint.X());
      double h = std::abs(m_currentPoint.Y() - m_startPoint.Y());
      painter.drawText(current + QPointF(5, 15),
                       QString::number(w, 'f', 1) + " x " +
                           QString::number(h, 'f', 1));
    }
    break;

  case SketchToolType::Circle: {
    double dx = m_currentPoint.X() - m_startPoint.X();
    double dy = m_currentPoint.Y() - m_startPoint.Y();
    double radius = std::sqrt(dx * dx + dy * dy);
    double radiusPixels = radius * m_scale;
    painter.drawEllipse(start, radiusPixels, radiusPixels);
    // Show radius
    painter.drawText(start + QPointF(radiusPixels + 5, 0),
                     "R" + QString::number(radius, 'f', 1));
    break;
  }
  case SketchToolType::Arc: {
    // 3-point arc preview
    if (!m_arcHasMidPoint) {
      // Step 1: Drawing line from start to end point
      painter.drawLine(start, current);

      // Draw start point (green) and end point preview (blue)
      painter.setPen(Qt::NoPen);
      painter.setBrush(Qt::green);
      painter.drawEllipse(start, 4, 4);
      painter.setBrush(m_previewColor);
      painter.drawEllipse(current, 4, 4);

      // Info text
      double dist = m_startPoint.Distance(m_currentPoint);
      painter.setPen(m_previewColor);
      painter.drawText(
          current + QPointF(10, -10),
          QString("Length: %1 (click for through point)").arg(dist, 0, 'f', 1));
    } else {
      // Step 2: Have start and through, now picking end point
      QPointF throughPt = worldToScreen(m_arcMidPoint);

      // Draw start-through line (dashed)
      painter.setPen(QPen(QColor(100, 100, 100), 1, Qt::DashLine));
      painter.drawLine(start, throughPt);

      // Create a temporary SketchArc to calculate the preview exactly like the
      // final arc This ensures preview matches the final result perfectly
      sketch::SketchArc tempArc(m_startPoint, m_arcMidPoint, m_currentPoint);

      if (tempArc.isValid()) {
        // Draw reference circle (subtle)
        QPointF centerScreen = worldToScreen(tempArc.center());
        double radiusPixels = tempArc.radius() * m_scale;
        painter.setPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
        painter.drawEllipse(centerScreen, radiusPixels, radiusPixels);

        // Draw arc using line segments - exactly like drawEntity does
        painter.setPen(QPen(m_previewColor, 2));
        QPainterPath path;
        QPointF firstPt = worldToScreen(tempArc.pointAtParameter(0.0));
        path.moveTo(firstPt);
        for (double t = 0.02; t <= 1.0; t += 0.02) {
          QPointF pt = worldToScreen(tempArc.pointAtParameter(t));
          path.lineTo(pt);
        }
        painter.drawPath(path);

        // Draw center
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(150, 150, 150));
        painter.drawEllipse(centerScreen, 3, 3);

        // Info
        painter.setPen(m_previewColor);
        painter.drawText(centerScreen + QPointF(10, -10),
                         QString("R: %1").arg(tempArc.radius(), 0, 'f', 1));
      }

      // Draw the 3 points
      painter.setPen(Qt::NoPen);
      painter.setBrush(Qt::green);
      painter.drawEllipse(start, 4, 4); // Start (green)
      painter.setBrush(Qt::yellow);
      painter.drawEllipse(throughPt, 4, 4);    // Through point (yellow)
      painter.setBrush(QColor(200, 100, 255)); // Purple for end point
      painter.drawEllipse(current, 4, 4);      // End point (moving)

      painter.setPen(m_previewColor);
      painter.drawText(current + QPointF(10, 5), "(click to set end point)");
    }
    break;
  }
  case SketchToolType::Ellipse: {
    double rx = std::abs(m_currentPoint.X() - m_startPoint.X());
    double ry = std::abs(m_currentPoint.Y() - m_startPoint.Y());
    double rxPixels = rx * m_scale;
    double ryPixels = ry * m_scale;
    painter.drawEllipse(start, rxPixels, ryPixels);
    painter.drawText(
        current + QPointF(5, 15),
        QString("R: %1 x %2").arg(rx, 0, 'f', 1).arg(ry, 0, 'f', 1));
    break;
  }
  case SketchToolType::Spline:
    // Draw existing spline points and smooth curve preview
    if (!m_splinePoints.empty()) {
      // Draw control points
      for (size_t i = 0; i < m_splinePoints.size(); ++i) {
        QPointF pt = worldToScreen(m_splinePoints[i]);
        painter.setBrush(i == 0 ? Qt::green : m_previewColor);
        painter.drawEllipse(pt, 5, 5);
        painter.drawText(pt + QPointF(8, -5), QString::number(i + 1));
      }

      // Check if close to first point (for closed spline)
      bool canClose = false;
      if (m_splinePoints.size() >= 3) {
        double distToFirst = m_currentPoint.Distance(m_splinePoints.front());
        canClose = (distToFirst < m_gridSize * 1.5);
      }

      // Draw smooth B-spline curve preview
      if (m_splinePoints.size() >= 2) {
        QPainterPath path;
        QPointF firstPt = worldToScreen(m_splinePoints[0]);
        path.moveTo(firstPt);

        // Use cubic bezier approximation for smooth preview
        std::vector<gp_Pnt2d> previewPts = m_splinePoints;
        previewPts.push_back(m_currentPoint);

        for (size_t i = 0; i < previewPts.size() - 1; ++i) {
          QPointF p0 = worldToScreen(previewPts[i]);
          QPointF p1 = worldToScreen(previewPts[i + 1]);

          // Simple Catmull-Rom to Bezier conversion for smooth curves
          if (i > 0 && i < previewPts.size() - 2) {
            QPointF pm1 = worldToScreen(previewPts[i - 1]);
            QPointF p2 = worldToScreen(previewPts[i + 2]);
            QPointF ctrl1 = p0 + (p1 - pm1) / 6.0;
            QPointF ctrl2 = p1 - (p2 - p0) / 6.0;
            path.cubicTo(ctrl1, ctrl2, p1);
          } else {
            path.lineTo(p1);
          }
        }

        painter.drawPath(path);

        // Draw close indicator if near first point
        if (canClose) {
          painter.save();
          painter.setPen(QPen(Qt::green, 3));
          QPointF fp = worldToScreen(m_splinePoints.front());
          painter.drawEllipse(fp, 10, 10);
          painter.drawText(fp + QPointF(15, 0), "Click to close");
          painter.restore();
        }
      } else {
        QPointF lastPt = worldToScreen(m_splinePoints.back());
        painter.drawLine(lastPt, current);
      }

      // Show point count and instructions
      painter.drawText(current + QPointF(10, 20),
                       QString("%1 pts | Enter: Finish | Esc: Cancel")
                           .arg(m_splinePoints.size()));
    }
    break;
  case SketchToolType::Polygon:
    if (m_isDrawing) {
      // Calculate world radius
      double worldRadius = m_startPoint.Distance(m_currentPoint);

      if (worldRadius > 0.1) {
        // Calculate angle of first vertex (direction from center to first
        // vertex)
        double dx = m_currentPoint.X() - m_startPoint.X();
        double dy = m_currentPoint.Y() - m_startPoint.Y();
        double startAngle = std::atan2(dy, dx);
        double stepAngle = 2.0 * M_PI / m_polygonSides;

        // Draw polygon in world coordinates, then convert to screen
        QPolygonF poly;
        for (int i = 0; i < m_polygonSides; ++i) {
          double angle = startAngle + i * stepAngle;
          double x = m_startPoint.X() + worldRadius * std::cos(angle);
          double y = m_startPoint.Y() + worldRadius * std::sin(angle);
          poly << worldToScreen(gp_Pnt2d(x, y));
        }

        // Draw polygon outline
        painter.setPen(QPen(m_previewColor, 2));
        painter.drawPolygon(poly);

        // Draw center point
        painter.setBrush(m_previewColor);
        painter.drawEllipse(start, 4, 4);

        // Draw construction circle (inscribed circle)
        double screenRadius = worldRadius * m_scale;
        painter.setPen(QPen(m_previewColor.darker(130), 1, Qt::DashLine));
        painter.drawEllipse(start, screenRadius, screenRadius);

        // Draw radius line from center to current vertex
        painter.setPen(QPen(m_previewColor, 1));
        painter.drawLine(start, current);

        // Show info text (SolidWorks style)
        painter.setPen(m_previewColor);
        QString info = QString("Sides: %1  R: %2")
                           .arg(m_polygonSides)
                           .arg(worldRadius, 0, 'f', 2);
        painter.drawText(current + QPointF(15, -10), info);
      }
    }
    break;
  case SketchToolType::Slot:
    if (m_isDrawing) {
      // Calculate slot using screen coordinates for correct direction
      double screenDx = current.x() - start.x();
      double screenDy = current.y() - start.y();
      double screenDistance =
          std::sqrt(screenDx * screenDx + screenDy * screenDy);

      // World distance for display
      double worldDistance = m_startPoint.Distance(m_currentPoint);

      if (screenDistance > 5) { // Minimum distance in pixels
        // Screen angle (Qt coordinate system: Y increases downward)
        double screenAngle = std::atan2(screenDy, screenDx);
        double perpAngle = screenAngle + M_PI / 2;

        // Half width in screen pixels
        double screenHalfWidth = (m_slotWidth / 2.0) * m_scale;

        // Calculate perpendicular offset in screen coordinates
        double perpDx = screenHalfWidth * std::cos(perpAngle);
        double perpDy = screenHalfWidth * std::sin(perpAngle);

        // Calculate the 4 corner points in screen coordinates
        QPointF p1(start.x() + perpDx, start.y() + perpDy);
        QPointF p2(start.x() - perpDx, start.y() - perpDy);
        QPointF p3(current.x() - perpDx, current.y() - perpDy);
        QPointF p4(current.x() + perpDx, current.y() + perpDy);

        // Draw the slot outline (parallel lines)
        painter.setPen(QPen(m_previewColor, 2));
        painter.drawLine(p1, p4); // Top line (connects start to end)
        painter.drawLine(p2, p3); // Bottom line

        // Draw semicircles at ends - use screenAngle for consistent direction
        double screenAngleDeg = screenAngle * 180.0 / M_PI;

        QRectF startArcRect(start.x() - screenHalfWidth,
                            start.y() - screenHalfWidth, screenHalfWidth * 2,
                            screenHalfWidth * 2);
        QRectF endArcRect(current.x() - screenHalfWidth,
                          current.y() - screenHalfWidth, screenHalfWidth * 2,
                          screenHalfWidth * 2);

        // Start semicircle: facing away from current (outward)
        int arc1Start = static_cast<int>((-screenAngleDeg + 90) * 16);
        painter.drawArc(startArcRect, arc1Start, 180 * 16);

        // End semicircle: facing away from start (outward)
        int arc2Start = static_cast<int>((-screenAngleDeg - 90) * 16);
        painter.drawArc(endArcRect, arc2Start, 180 * 16);

        // Draw center line (dashed)
        painter.setPen(QPen(m_previewColor.darker(130), 1, Qt::DashLine));
        painter.drawLine(start, current);

        // Draw center points
        painter.setBrush(m_previewColor);
        painter.setPen(QPen(m_previewColor, 1));
        painter.drawEllipse(start, 4, 4);
        painter.drawEllipse(current, 4, 4);

        // Show info text
        painter.setPen(m_previewColor);
        QString info = QString("Length: %1  Width: %2")
                           .arg(worldDistance, 0, 'f', 2)
                           .arg(m_slotWidth, 0, 'f', 2);
        painter.drawText(current + QPointF(15, -10), info);
      }
    }
    break;
  default:
    break;
  }

  // Draw endpoints
  painter.setBrush(m_previewColor);
  painter.drawEllipse(start, 4, 4);
  painter.drawEllipse(current, 4, 4);

  painter.restore();
}

void SketchView2D::drawCursor(QPainter &painter) {
  // Show coordinates at bottom left
  painter.save();
  painter.setPen(Qt::black);
  painter.setFont(QFont("Consolas", 10));

  gp_Pnt2d worldPos = screenToWorld(m_lastMousePos);
  QString coordText = QString("X: %1  Y: %2")
                          .arg(worldPos.X(), 0, 'f', 2)
                          .arg(worldPos.Y(), 0, 'f', 2);

  painter.drawText(10, height() - 10, coordText);

  // Show current tool
  QString toolText;
  switch (m_currentTool) {
  case SketchToolType::Select:
    toolText = "Select";
    break;
  case SketchToolType::Line:
    toolText = "Line";
    break;
  case SketchToolType::Rectangle:
    toolText = "Rectangle";
    break;
  case SketchToolType::Circle:
    toolText = "Circle";
    break;
  case SketchToolType::Arc:
    toolText = "Arc";
    break;
  case SketchToolType::Point:
    toolText = "Point";
    break;
  case SketchToolType::Spline:
    toolText = "Spline";
    break;
  case SketchToolType::Ellipse:
    toolText = "Ellipse";
    break;
  case SketchToolType::Polygon:
    toolText = QString("Polygon (%1 sides)").arg(m_polygonSides);
    break;
  default:
    toolText = "";
    break;
  }
  painter.drawText(10, 20, "Tool: " + toolText);

  painter.restore();
}

// Mouse events
void SketchView2D::mousePressEvent(QMouseEvent *event) {
  setFocus(); // Grab keyboard focus when clicking
  m_lastMousePos = event->pos();

  if (event->button() == Qt::MiddleButton) {
    m_isPanning = true;
    setCursor(Qt::ClosedHandCursor);
    return;
  }

  if (event->button() == Qt::LeftButton && m_sketch) {
    gp_Pnt2d worldPos = screenToWorld(event->pos());
    SnapResult snap = findBestSnap(worldPos);
    gp_Pnt2d snappedPos = snap.point;
    m_currentSnap = snap;

    switch (m_currentTool) {
    case SketchToolType::Point:
      m_sketch->addPoint(snappedPos.X(), snappedPos.Y());
      update();
      break;

    case SketchToolType::Line:
    case SketchToolType::Rectangle:
    case SketchToolType::Circle:
      m_isDrawing = true;
      m_startPoint = snappedPos;
      m_currentPoint = snappedPos;
      m_startSnap = snap;
      break;

    case SketchToolType::Arc:
      // 3-point arc: click 1 = start, click 2 = end, click 3 = point on arc
      if (!m_isDrawing) {
        // First click: start point
        m_isDrawing = true;
        m_startPoint = snappedPos;
        m_currentPoint = snappedPos;
        m_arcHasMidPoint = false;
        m_startSnap = snap;
      } else if (!m_arcHasMidPoint) {
        // Second click: end point
        m_arcMidPoint = m_startPoint; // Temporarily store, will be replaced
        m_arcHasMidPoint = true;
        // m_currentPoint already has the end position from mouseMove
      }
      // Third click handled in mouseReleaseEvent
      break;
    case SketchToolType::Ellipse:
    case SketchToolType::Polygon:
    case SketchToolType::Slot:
      m_isDrawing = true;
      m_startPoint = snappedPos;
      m_currentPoint = snappedPos;
      m_startSnap =
          snap; // Save snap for start point (for coincident constraints)
      break;

    case SketchToolType::Spline:
      // Spline uses click-to-add-point mode
      if (!m_isDrawing) {
        m_isDrawing = true;
        m_splinePoints.clear();
        m_splinePoints.push_back(snappedPos);
      } else {
        // Check if clicking near first point to close the spline
        if (m_splinePoints.size() >= 3) {
          double distToFirst = snappedPos.Distance(m_splinePoints.front());
          if (distToFirst < m_gridSize * 1.5) {
            // Close and finish the spline
            m_sketch->addSpline(m_splinePoints, true); // closed = true
            m_isDrawing = false;
            m_splinePoints.clear();
            emit entityCreated(nullptr);
            update();
            break;
          }
        }
        m_splinePoints.push_back(snappedPos);
      }
      m_currentPoint = snappedPos;
      update();
      break;

    case SketchToolType::Select: {
      double tolerance = m_snapRadius / m_scale;
      bool ctrlHeld = event->modifiers() & Qt::ControlModifier;

      // 1. Try to find a control point first
      sketch::SketchEntity *cpEntity = nullptr;
      int cpIdx = findControlPointAtPoint(snappedPos, tolerance, cpEntity);

      if (cpIdx != -1 && cpEntity) {
        selectEntity(cpEntity, ctrlHeld);
        m_selectedControlPointIndex = cpIdx;
        m_isDraggingPoint = true;
        m_isDragging = false;
        update();
        break;
      }

      m_selectedControlPointIndex = -1;
      m_isDraggingPoint = false;

      // 2. Find entity under cursor (edge selection)
      sketch::SketchEntity *entity = findEntityAtPoint(snappedPos, tolerance);

      if (entity) {
        // Ctrl+Click toggles selection
        if (ctrlHeld) {
          toggleEntitySelection(entity);
        } else {
          selectEntity(entity, false);
        }

        // Start dragging if any entities are selected
        if (!m_selectedEntities.empty()) {
          m_isDragging = true;
          m_dragStartWorld = snappedPos;
          m_entityOriginalPos = entity->startPoint();
        }
      } else {
        // Clicked on empty space
        if (!ctrlHeld) {
          // Start box selection
          m_isBoxSelecting = true;
          m_boxSelectStart = event->pos();
          m_boxSelectEnd = event->pos();
          clearSelection();
        }
        m_isDragging = false;
      }
      update();
      break;
    }

    case SketchToolType::ProfileSelect: {
      // Multi-selection mode: clicking adds to selection list
      // Prioritize hovered profile if available
      int profileIdx = (m_hoveredProfileIndex >= 0)
                           ? m_hoveredProfileIndex
                           : findProfileAtPoint(event->pos());

      if (profileIdx == -2) {
        // Ring selection - add to list
        std::pair<int, int> ring = {m_pendingRingOuter, m_pendingRingInner};
        // Check if already selected, toggle off if so
        auto it = std::find(m_selectedProfiles.begin(),
                            m_selectedProfiles.end(), ring);
        if (it != m_selectedProfiles.end()) {
          m_selectedProfiles.erase(it);
        } else {
          m_selectedProfiles.push_back(ring);
        }
        update();
      } else if (profileIdx >= 0) {
        // Solid profile selection - add to list
        std::pair<int, int> solid = {profileIdx, -1};
        auto it = std::find(m_selectedProfiles.begin(),
                            m_selectedProfiles.end(), solid);
        if (it != m_selectedProfiles.end()) {
          m_selectedProfiles.erase(it);
        } else {
          m_selectedProfiles.push_back(solid);
        }
        update();
      }
      break;
    }

    default:
      break;
    }
  }
}

void SketchView2D::mouseMoveEvent(QMouseEvent *event) {
  QPointF delta = event->pos() - m_lastMousePos;

  if (m_isPanning) {
    m_offset += delta;
    m_lastMousePos = event->pos();
    update();
    return;
  }

  m_lastMousePos = event->pos();
  gp_Pnt2d worldPos = screenToWorld(event->pos());
  emit cursorPositionChanged(worldPos.X(), worldPos.Y());

  // Update snap indicator
  m_currentSnap = findBestSnap(worldPos);

  if (m_isDrawing) {
    m_currentPoint = m_currentSnap.point;

    // Track arc winding direction
    if (m_currentTool == SketchToolType::Arc) {
      QPointF startScreen = worldToScreen(m_startPoint);
      QPointF currentScreen = worldToScreen(m_currentPoint);
      double screenDx = currentScreen.x() - startScreen.x();
      double screenDy = currentScreen.y() - startScreen.y();
      double currentAngle = std::atan2(screenDy, screenDx);

      // Only accumulate delta if we have a valid previous angle
      // Use cumulative angle as the "initialized" flag
      if (m_arcCumulativeAngle != 0.0 || m_arcPrevAngle != 0.0) {
        double delta = currentAngle - m_arcPrevAngle;
        // Handle wrap-around at ±π
        if (delta > M_PI)
          delta -= 2 * M_PI;
        if (delta < -M_PI)
          delta += 2 * M_PI;
        m_arcCumulativeAngle += delta;
      }
      // Always update previous angle for next iteration
      m_arcPrevAngle = currentAngle;
    }
  }

  // Handle entity dragging or point dragging in Select mode
  if (m_currentTool == SketchToolType::Select) {
    // Box selection update
    if (m_isBoxSelecting) {
      m_boxSelectEnd = event->pos();
      update();
      return;
    }

    // Transform mode
    if (m_transformMode != TransformMode::None) {
      applyTransform(worldPos);
      return;
    }

    if (m_isDraggingPoint && m_selectedEntity &&
        m_selectedControlPointIndex != -1) {
      // Drag control point
      modifyEntityPoint(m_selectedEntity, m_selectedControlPointIndex,
                        m_currentSnap.point);
      emit entityCreated(nullptr);
    } else if (m_isDragging && !m_selectedEntities.empty()) {
      // Drag all selected entities
      gp_Pnt2d snappedPos = m_currentSnap.point;
      double dx = snappedPos.X() - m_dragStartWorld.X();
      double dy = snappedPos.Y() - m_dragStartWorld.Y();

      moveEntities(dx, dy);
      m_dragStartWorld = snappedPos;

      emit entityCreated(nullptr);
    }
  }

  // Update hover state for Select mode
  if (m_currentTool == SketchToolType::Select && !m_isDragging &&
      !m_isBoxSelecting) {
    double tolerance = m_snapRadius / m_scale;
    sketch::SketchEntity *hovered = findEntityAtPoint(worldPos, tolerance);
    if (hovered != m_hoveredEntity) {
      m_hoveredEntity = hovered;
    }
  }

  // Update hover state for ProfileSelect mode
  if (m_currentTool == SketchToolType::ProfileSelect) {
    int newHover = findProfileAtPoint(event->pos());
    if (newHover != m_hoveredProfileIndex) {
      m_hoveredProfileIndex = newHover;
      emit profileHovered(newHover);
    }
  }

  update(); // For cursor and snap indicator
}

void SketchView2D::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::MiddleButton) {
    m_isPanning = false;
    setCursor(Qt::ArrowCursor);
    return;
  }

  if (event->button() == Qt::LeftButton) {
    // Finish box selection
    if (m_isBoxSelecting) {
      m_isBoxSelecting = false;
      QRectF selectionBox = QRectF(m_boxSelectStart, m_boxSelectEnd);
      selectEntitiesInBox(selectionBox);
      update();
      return;
    }

    // Finish transform
    if (m_transformMode != TransformMode::None) {
      finishTransform();
      return;
    }

    // Finish dragging
    if (m_isDragging || m_isDraggingPoint) {
      m_isDragging = false;
      m_isDraggingPoint = false;
      if (!m_selectedEntities.empty() || m_selectedEntity) {
        saveSketchCheckpoint("Modify Entity");
      }
      update();
      return;
    }
  }

  if (event->button() == Qt::LeftButton && m_isDrawing && m_sketch) {
    finishCurrentEntity();
  }
}

void SketchView2D::finishCurrentEntity() {
  if (!m_sketch)
    return;

  sketch::SketchEntity *createdEntity = nullptr;

  switch (m_currentTool) {
  case SketchToolType::Line:
    if (m_startPoint.Distance(m_currentPoint) > 0.1) {
      auto line = m_sketch->addLine(m_startPoint.X(), m_startPoint.Y(),
                                    m_currentPoint.X(), m_currentPoint.Y());
      createdEntity = line.get();

      // Apply auto-constraints for the line
      if (m_autoConstraint && line) {
        // Check for horizontal/vertical alignment
        double dx = m_currentPoint.X() - m_startPoint.X();
        double dy = m_currentPoint.Y() - m_startPoint.Y();
        double angle = std::atan2(std::abs(dy), std::abs(dx)) * 180.0 / M_PI;

        // Near horizontal (angle < tolerance)
        if (angle < m_autoConstraintAngleTolerance) {
          m_sketch->addHorizontal(line);
          qDebug() << "Auto-constraint: Added Horizontal constraint";
        }
        // Near vertical (angle close to 90)
        else if (angle > (90.0 - m_autoConstraintAngleTolerance)) {
          m_sketch->addVertical(line);
          qDebug() << "Auto-constraint: Added Vertical constraint";
        }

        // Check for coincident constraint at START point (using m_startSnap)
        if (m_startSnap.type == SnapType::Endpoint && m_startSnap.entity) {
          auto *snapLine =
              dynamic_cast<sketch::SketchLine *>(m_startSnap.entity);
          if (snapLine && snapLine != line.get()) {
            double distToStart = m_startPoint.Distance(snapLine->startPoint());
            double distToEnd = m_startPoint.Distance(snapLine->endPoint());
            int snapEndpoint = (distToStart < distToEnd) ? 0 : 1;

            auto snapLinePtr = std::dynamic_pointer_cast<sketch::SketchLine>(
                m_sketch->getEntity(snapLine->id()));
            if (snapLinePtr) {
              m_sketch->addCoincident(line, 0, snapLinePtr, snapEndpoint);
              qDebug() << "Auto-constraint: Added Coincident (start)";
            }
          }
        }

        // Check for coincident constraint at END point (using m_currentSnap)
        if (m_currentSnap.type == SnapType::Endpoint && m_currentSnap.entity) {
          auto *snapLine =
              dynamic_cast<sketch::SketchLine *>(m_currentSnap.entity);
          if (snapLine && snapLine != line.get()) {
            double distToStart =
                m_currentPoint.Distance(snapLine->startPoint());
            double distToEnd = m_currentPoint.Distance(snapLine->endPoint());
            int snapEndpoint = (distToStart < distToEnd) ? 0 : 1;

            auto snapLinePtr = std::dynamic_pointer_cast<sketch::SketchLine>(
                m_sketch->getEntity(snapLine->id()));
            if (snapLinePtr) {
              m_sketch->addCoincident(line, 1, snapLinePtr, snapEndpoint);
              qDebug() << "Auto-constraint: Added Coincident (end)";
            }
          }
        }
      }
    }
    break;

  case SketchToolType::Rectangle:
    if (std::abs(m_currentPoint.X() - m_startPoint.X()) > 0.1 &&
        std::abs(m_currentPoint.Y() - m_startPoint.Y()) > 0.1) {
      double x = std::min(m_startPoint.X(), m_currentPoint.X());
      double y = std::min(m_startPoint.Y(), m_currentPoint.Y());
      double w = std::abs(m_currentPoint.X() - m_startPoint.X());
      double h = std::abs(m_currentPoint.Y() - m_startPoint.Y());
      m_sketch->addRectangle(x, y, w, h);
    }
    break;

  case SketchToolType::Circle: {
    double radius = m_startPoint.Distance(m_currentPoint);
    if (radius > 0.1) {
      m_sketch->addCircle(m_startPoint.X(), m_startPoint.Y(), radius);
    }
    break;
  }
  case SketchToolType::Arc: {
    // 3-point arc: click 3 = point on arc (defines curvature)
    if (m_arcHasMidPoint) {
      // Third click: create the arc using start, end, and point on arc
      double dist = m_startPoint.Distance(m_arcMidPoint);
      if (dist > 0.1) {
        // m_startPoint = start (1st click)
        // m_arcMidPoint = through point (2nd click, point ON the arc)
        // m_currentPoint = end point (3rd click, where arc ends)
        m_sketch->addThreePointArc(m_startPoint, m_currentPoint, m_arcMidPoint);
      }
      m_arcHasMidPoint = false;
      m_isDrawing = false;
    } else {
      // Second click: save end point and wait for third click
      if (m_startPoint.Distance(m_currentPoint) > 0.1) {
        m_arcMidPoint = m_currentPoint; // Store end point
        m_arcHasMidPoint = true;
      }
      return; // Don't finish drawing yet, wait for 3rd click
    }
    break;
  }
  case SketchToolType::Ellipse: {
    double rx = std::abs(m_currentPoint.X() - m_startPoint.X());
    double ry = std::abs(m_currentPoint.Y() - m_startPoint.Y());
    if (rx > 0.1 && ry > 0.1) {
      m_sketch->addEllipse(m_startPoint, rx, ry);
    }
    break;
  }
  case SketchToolType::Spline:
    // Spline is not finished on mouse release - use Enter/Escape
    return;
  case SketchToolType::Polygon: {
    double radius = m_startPoint.Distance(m_currentPoint);
    if (radius > 0.1) {
      m_sketch->addPolygon(m_startPoint, m_currentPoint, m_polygonSides);
    }
    break;
  }
  case SketchToolType::Slot: {
    double distance = m_startPoint.Distance(m_currentPoint);
    if (distance > 0.1 && m_slotWidth > 0.1) {
      // Create slot using SketchSlot constructor (center1, center2, width)
      auto slot = std::make_shared<sketch::SketchSlot>(
          m_startPoint, m_currentPoint, m_slotWidth);
      m_sketch->addEntity(slot);
      createdEntity = slot.get();
    }
    break;
  }
  default:
    break;
  }

  m_isDrawing = false;
  m_splinePoints.clear();

  // Solve constraints to apply them to geometry
  if (m_sketch && !m_sketch->constraints().empty()) {
    auto status = m_sketch->solve();
    qDebug() << "Auto-constraint: Solver status =" << static_cast<int>(status);
  }

  // Save checkpoint for undo
  saveSketchCheckpoint("Add Entity");

  // Notify MainWindow to update 3D viewport with new sketch wire
  emit entityCreated(createdEntity);

  update();
}

void SketchView2D::cancelCurrentEntity() {
  m_isDrawing = false;
  update();
}

void SketchView2D::wheelEvent(QWheelEvent *event) {
  double factor = event->angleDelta().y() > 0 ? 1.1 : 0.9;

  // Zoom centered on mouse position
  QPointF mousePos = event->position();
  gp_Pnt2d worldBefore = screenToWorld(mousePos);

  m_scale *= factor;
  if (m_scale < 0.1)
    m_scale = 0.1;
  if (m_scale > 100)
    m_scale = 100;

  // Adjust offset to keep mouse position fixed
  QPointF newScreenPos = worldToScreen(worldBefore);
  m_offset += mousePos - newScreenPos;

  update();
}

void SketchView2D::resizeEvent(QResizeEvent * /*event*/) { update(); }

void SketchView2D::handleEscPress() {
  qDebug() << "handleEscPress called, currentTool:"
           << static_cast<int>(m_currentTool) << "isDrawing:" << m_isDrawing
           << "escPressedOnce:" << m_escPressedOnce;

  if (m_currentTool == SketchToolType::ProfileSelect) {
    exitProfileSelectMode();
    m_escPressedOnce = false;
  }
  // If in a drawing tool (not Select), switch to Select mode
  else if (m_currentTool != SketchToolType::Select) {
    cancelCurrentEntity();
    m_splinePoints.clear();
    setTool(SketchToolType::Select);
    m_escPressedOnce = false; // Reset! Need TWO more ESC presses to exit
    qDebug() << "Switched to Select mode, press ESC twice to exit";
  }
  // If actively drawing in Select mode, cancel it
  else if (m_isDrawing || m_isDragging) {
    m_isDrawing = false;
    m_isDragging = false;
    m_escPressedOnce = false;
  }
  // Clear selection if any
  else if (!m_selectedEntities.empty()) {
    clearSelection();
    m_escPressedOnce = false;
  }
  // Double-ESC to exit: second press
  else if (m_escPressedOnce) {
    qDebug() << "Second ESC - exiting sketch";
    emit sketchExitRequested();
    m_escPressedOnce = false;
  }
  // Double-ESC to exit: first press
  else {
    m_escPressedOnce = true;
    qDebug() << "First ESC in Select mode - press ESC again to exit sketch";
  }
}

void SketchView2D::keyPressEvent(QKeyEvent *event) {
  qDebug() << "=== SketchView2D::keyPressEvent called! key:" << event->key();

  // Handle Ctrl+ shortcuts
  if (event->modifiers() & Qt::ControlModifier) {
    switch (event->key()) {
    case Qt::Key_Z:
      undo();
      return;
    case Qt::Key_Y:
      redo();
      return;
    case Qt::Key_A:
      // Select all
      selectAll();
      return;
    case Qt::Key_C:
      // Copy
      copySelection();
      return;
    case Qt::Key_V:
      // Paste
      pasteClipboard();
      return;
    case Qt::Key_X:
      // Cut
      cutSelection();
      return;
    }
  }

  switch (event->key()) {

  case Qt::Key_Return:
  case Qt::Key_Enter:
    // Confirm multi-profile selection
    if (m_currentTool == SketchToolType::ProfileSelect) {
      if (!m_selectedProfiles.empty()) {
        emit multiProfilesConfirmed(m_selectedProfiles);
      }
      exitProfileSelectMode();
    }
    // Finish spline with Enter key
    else if (m_currentTool == SketchToolType::Spline &&
             m_splinePoints.size() >= 2 && m_sketch) {
      m_sketch->addSpline(m_splinePoints);
      saveSketchCheckpoint("Add Spline");
      m_isDrawing = false;
      m_splinePoints.clear();
      update();
    }
    // Confirm transform
    else if (m_transformMode != TransformMode::None) {
      finishTransform();
    }
    break;

  case Qt::Key_Escape:
    // Delegate to handleEscPress for consistency
    if (m_transformMode != TransformMode::None) {
      cancelTransform();
      m_escPressedOnce = false;
    } else if (m_isBoxSelecting) {
      m_isBoxSelecting = false;
      m_escPressedOnce = false;
      update();
    } else {
      handleEscPress();
    }
    break;

  // ========== DRAWING TOOLS ==========
  case Qt::Key_S:
    // S = Select (default mode)
    setTool(SketchToolType::Select);
    m_escPressedOnce = false;
    break;
  case Qt::Key_L:
    // L = Line
    setTool(SketchToolType::Line);
    break;
  case Qt::Key_R:
    // R = Rectangle
    setTool(SketchToolType::Rectangle);
    break;
  case Qt::Key_C:
    // C = Circle (only without Ctrl modifier)
    if (!(event->modifiers() & Qt::ControlModifier)) {
      setTool(SketchToolType::Circle);
    }
    break;
  case Qt::Key_E:
    // E = Ellipse
    setTool(SketchToolType::Ellipse);
    break;
  case Qt::Key_A:
    // A = Arc (only without Ctrl modifier)
    if (!(event->modifiers() & Qt::ControlModifier)) {
      setTool(SketchToolType::Arc);
    }
    break;
  case Qt::Key_P:
    // P = Point
    setTool(SketchToolType::Point);
    break;
  case Qt::Key_N:
    // N = Spline (curvature tool)
    setTool(SketchToolType::Spline);
    break;
  case Qt::Key_O:
    // O = Offset
    setTool(SketchToolType::Offset);
    break;
  case Qt::Key_Y:
    // Y = polYgon
    setTool(SketchToolType::Polygon);
    break;

  // ========== TRANSFORM TOOLS (when entity selected) ==========
  case Qt::Key_G:
    // G = Grab/Move (Blender-style)
    if (!m_selectedEntities.empty()) {
      startTransform(TransformMode::Move);
    }
    break;
  case Qt::Key_T:
    // T = Rotate (Transform rotate)
    if (!m_selectedEntities.empty()) {
      startTransform(TransformMode::Rotate);
    }
    break;
  case Qt::Key_K:
    // K = Scale (sKale)
    if (!m_selectedEntities.empty()) {
      startTransform(TransformMode::Scale);
    }
    break;

  // ========== VIEW CONTROLS ==========
  case Qt::Key_F:
    // F = Fit all
    fitAll();
    break;
  case Qt::Key_H:
    // H = Toggle grid (Hide/show grid)
    m_showGrid = !m_showGrid;
    update();
    break;
  case Qt::Key_D:
    // D = Toggle dimensions
    m_showDimensions = !m_showDimensions;
    update();
    break;

  // ========== EDIT OPERATIONS ==========
  case Qt::Key_Delete:
  case Qt::Key_Backspace:
    // Delete/Backspace = Delete selected
    if (!m_selectedEntities.empty()) {
      deleteSelectedEntities();
    } else if (m_selectedEntity) {
      deleteSelectedEntity();
    }
    break;
  case Qt::Key_X:
    // X = Delete selected (only without Ctrl modifier, as Ctrl+X is Cut)
    if (!(event->modifiers() & Qt::ControlModifier)) {
      if (!m_selectedEntities.empty()) {
        deleteSelectedEntities();
      } else if (m_selectedEntity) {
        deleteSelectedEntity();
      }
    }
    break;

  // Polygon side adjustment
  case Qt::Key_Plus:
  case Qt::Key_Equal: // often + is Shift+=
    if (m_currentTool == SketchToolType::Polygon) {
      m_polygonSides++;
      update();
    }
    break;
  case Qt::Key_Minus:
  case Qt::Key_Underscore:
    if (m_currentTool == SketchToolType::Polygon) {
      if (m_polygonSides > 3)
        m_polygonSides--;
      update();
    }
    break;

  default:
    QWidget::keyPressEvent(event);
  }
}

// ============ PROFILE SELECTION FUNCTIONS ============

void SketchView2D::drawProfileOverlays(QPainter &painter) {
  if (!m_sketch || m_profiles.empty())
    return;

  painter.save();

  // Draw all profiles with different colors
  for (size_t i = 0; i < m_profiles.size(); ++i) {
    const TopoDS_Wire &wire = m_profiles[i];

    // Check if this profile is selected
    bool isSelected = false;
    for (const auto &sel : m_selectedProfiles) {
      if (sel.first == static_cast<int>(i)) {
        isSelected = true;
        break;
      }
    }

    // Check if this profile is hovered
    bool isHovered = (m_hoveredProfileIndex == static_cast<int>(i));

    // Choose color based on state
    QColor outlineColor;
    int lineWidth;
    if (isSelected) {
      outlineColor = QColor(0, 255, 0, 255); // Bright green for selected
      lineWidth = 4;
    } else if (isHovered) {
      outlineColor = QColor(255, 255, 0, 255); // Yellow for hovered
      lineWidth = 3;
    } else {
      // Use profile color from palette
      outlineColor = m_profileColors[i % m_profileColors.size()];
      lineWidth = 2;
    }

    painter.setPen(QPen(outlineColor, lineWidth));
    painter.setBrush(Qt::NoBrush);

    // Draw wire outline
    try {
      TopExp_Explorer edgeExp(wire, TopAbs_EDGE);

      while (edgeExp.More()) {
        TopoDS_Edge edge = static_cast<const TopoDS_Edge &>(edgeExp.Current());

        // Use BRepAdaptor_Curve to handle both 3D curves and curves on surfaces
        // (like Splines)
        BRepAdaptor_Curve curveAdaptor(edge);
        double first = curveAdaptor.FirstParameter();
        double last = curveAdaptor.LastParameter();

        // Sample points along the edge
        int numSamples = 30; // Increase resolution for splines
        QPointF prevPt;
        for (int j = 0; j <= numSamples; ++j) {
          double param = first + (last - first) * j / numSamples;
          gp_Pnt point3D = curveAdaptor.Value(param);

          // Convert 3D point to 2D sketch coordinates
          gp_Pnt2d point2D(point3D.X(), point3D.Y());
          QPointF screenPt = worldToScreen(point2D);

          if (j > 0) {
            painter.drawLine(prevPt, screenPt);
          }
          prevPt = screenPt;
        }

        edgeExp.Next();
      }
    } catch (...) {
      // Skip invalid profiles
      continue;
    }
  }

  // Draw selection summary and instructions at bottom
  painter.setPen(Qt::white);
  painter.setFont(QFont("Arial", 11));

  QString selectionInfo =
      m_selectedProfiles.empty()
          ? "No selection"
          : QString("Selected: %1 item(s)").arg(m_selectedProfiles.size());

  QString hint =
      QString("%1 | Click to add/remove, ENTER to confirm, ESC to cancel")
          .arg(selectionInfo);
  painter.drawText(10, height() - 10, hint);

  painter.restore();
}

int SketchView2D::findProfileAtPoint(const QPointF &screenPos) {
  if (m_profiles.empty())
    return -1;

  gp_Pnt2d worldPt = screenToWorld(screenPos);

  // Convert 2D sketch point to 3D using sketch plane
  gp_Pnt testPoint3D = m_sketch->plane().to3D(worldPt.X(), worldPt.Y());

  // Collect all profiles that contain this point with their areas
  struct ProfileInfo {
    int index;
    double area;
  };
  std::vector<ProfileInfo> containingProfiles;

  for (size_t i = 0; i < m_profiles.size(); ++i) {
    const auto &wire = m_profiles[i];
    if (wire.IsNull())
      continue;

    try {
      BRepBuilderAPI_MakeFace faceBuilder(wire, true);
      if (faceBuilder.IsDone()) {
        TopoDS_Face face = faceBuilder.Face();
        BRepClass_FaceClassifier classifier(face, testPoint3D, 1e-6);
        TopAbs_State state = classifier.State();

        if (state == TopAbs_IN || state == TopAbs_ON) {
          GProp_GProps props;
          BRepGProp::SurfaceProperties(face, props);
          containingProfiles.push_back({static_cast<int>(i), props.Mass()});
        }
      }
    } catch (...) {
    }
  }

  if (containingProfiles.empty()) {
    m_pendingRingOuter = -1;
    m_pendingRingInner = -1;
    return -1;
  }

  // Sort by area (smallest first)
  std::sort(containingProfiles.begin(), containingProfiles.end(),
            [](const ProfileInfo &a, const ProfileInfo &b) {
              return a.area < b.area;
            });

  if (containingProfiles.size() == 1) {
    // Only one profile contains point - check if there's a smaller profile
    // inside it
    int outerIdx = containingProfiles[0].index;
    double outerArea = containingProfiles[0].area;

    // Find any profile that is INSIDE the outer but doesn't contain the point
    int innerIdx = -1;
    double innerArea = 0;
    for (size_t i = 0; i < m_profiles.size(); ++i) {
      if (static_cast<int>(i) == outerIdx)
        continue;

      const auto &wire = m_profiles[i];
      if (wire.IsNull())
        continue;

      try {
        BRepBuilderAPI_MakeFace faceBuilder(wire, true);
        if (faceBuilder.IsDone()) {
          TopoDS_Face face = faceBuilder.Face();
          GProp_GProps props;
          BRepGProp::SurfaceProperties(face, props);
          double area = props.Mass();

          // Check if this profile is smaller than outer (i.e., inside it)
          if (area < outerArea && area > innerArea) {
            innerArea = area;
            innerIdx = static_cast<int>(i);
          }
        }
      } catch (...) {
      }
    }

    if (innerIdx >= 0) {
      // Ring selection: point is between outer and inner profiles
      m_pendingRingOuter = outerIdx;
      m_pendingRingInner = innerIdx;
      return -2; // Special code for ring selection
    } else {
      // Single solid profile
      m_pendingRingOuter = -1;
      m_pendingRingInner = -1;
      return outerIdx;
    }
  } else {
    // Multiple profiles contain point - return smallest (innermost)
    m_pendingRingOuter = -1;
    m_pendingRingInner = -1;
    return containingProfiles[0].index;
  }
}

void SketchView2D::enterProfileSelectMode() {
  // Cache profiles from sketch and clear selection
  if (m_sketch) {
    m_profiles = m_sketch->detectClosedProfiles();
    qDebug() << "enterProfileSelectMode: Detected" << m_profiles.size()
             << "closed profiles";
  } else {
    qDebug() << "enterProfileSelectMode: No sketch!";
  }
  m_selectedProfiles.clear(); // Clear previous selections
  m_hoveredProfileIndex = -1;
  setTool(SketchToolType::ProfileSelect);
  update();
}

void SketchView2D::exitProfileSelectMode() {
  m_profiles.clear();
  m_hoveredProfileIndex = -1;
  setTool(SketchToolType::Select);
  emit profileSelectionCancelled();
  update();
}

// ============ SKETCH UNDO/REDO SYSTEM ============

void SketchView2D::saveSketchCheckpoint(const std::string &description) {
  if (!m_sketch)
    return;

  // Remove future history if we're not at the end
  while (m_sketchHistoryIndex < static_cast<int>(m_sketchHistory.size()) - 1) {
    m_sketchHistory.pop_back();
  }

  // Create snapshot
  SketchSnapshot snapshot;
  snapshot.description = description;

  // Clone all entities from sketch
  const auto &entities = m_sketch->entities();
  for (const auto &entity : entities) {
    snapshot.entities.push_back(entity);
  }

  // Add to history
  m_sketchHistory.push_back(std::move(snapshot));
  m_sketchHistoryIndex = static_cast<int>(m_sketchHistory.size()) - 1;

  // Limit history size
  while (m_sketchHistory.size() > m_maxSketchHistory) {
    m_sketchHistory.pop_front();
    m_sketchHistoryIndex--;
  }

  qDebug() << "Sketch checkpoint:" << QString::fromStdString(description)
           << "| Entities:" << entities.size()
           << "| History:" << m_sketchHistory.size()
           << "| Index:" << m_sketchHistoryIndex;
}

void SketchView2D::undo() {
  if (!canUndo() || !m_sketch) {
    qDebug() << "Sketch: Nothing to undo";
    return;
  }

  // Move back one step
  m_sketchHistoryIndex--;

  // Restore previous state
  const auto &snapshot = m_sketchHistory[m_sketchHistoryIndex];

  // Clear current sketch entities
  m_sketch->clearEntities();

  // Restore entities from snapshot
  for (const auto &entity : snapshot.entities) {
    m_sketch->addEntity(entity);
  }

  m_selectedEntity = nullptr;
  m_hoveredEntity = nullptr;
  update();

  qDebug() << "Sketch Undo:"
           << QString::fromStdString(
                  m_sketchHistory[m_sketchHistoryIndex + 1].description);
}

void SketchView2D::redo() {
  if (!canRedo() || !m_sketch) {
    qDebug() << "Sketch: Nothing to redo";
    return;
  }

  // Move forward one step
  m_sketchHistoryIndex++;

  // Restore next state
  const auto &snapshot = m_sketchHistory[m_sketchHistoryIndex];

  // Clear current sketch entities
  m_sketch->clearEntities();

  // Restore entities from snapshot
  for (const auto &entity : snapshot.entities) {
    m_sketch->addEntity(entity);
  }

  m_selectedEntity = nullptr;
  m_hoveredEntity = nullptr;
  update();

  qDebug() << "Sketch Redo:" << QString::fromStdString(snapshot.description);
}

// ============ ENTITY SELECTION AND EDITING ============

sketch::SketchEntity *SketchView2D::findEntityAtPoint(const gp_Pnt2d &worldPos,
                                                      double tolerance) {
  if (!m_sketch)
    return nullptr;

  sketch::SketchEntity *closest = nullptr;
  double minDist = tolerance;

  for (auto &entity : m_sketch->entities()) {
    double dist = distanceToEntity(worldPos, entity.get());
    if (dist < minDist) {
      minDist = dist;
      closest = entity.get();
    }
  }

  return closest;
}

double
SketchView2D::distanceToEntity(const gp_Pnt2d &point,
                               const sketch::SketchEntity *entity) const {
  if (!entity)
    return std::numeric_limits<double>::max();

  switch (entity->type()) {
  case sketch::EntityType::Point: {
    auto *pt = static_cast<const sketch::SketchPoint *>(entity);
    return point.Distance(pt->position());
  }
  case sketch::EntityType::Line: {
    auto *line = static_cast<const sketch::SketchLine *>(entity);
    gp_Pnt2d p1 = line->startPoint();
    gp_Pnt2d p2 = line->endPoint();

    // Project point onto line segment
    double dx = p2.X() - p1.X();
    double dy = p2.Y() - p1.Y();
    double len2 = dx * dx + dy * dy;
    if (len2 < 1e-10)
      return point.Distance(p1);

    double t = std::max(0.0, std::min(1.0, ((point.X() - p1.X()) * dx +
                                            (point.Y() - p1.Y()) * dy) /
                                               len2));
    gp_Pnt2d proj(p1.X() + t * dx, p1.Y() + t * dy);
    return point.Distance(proj);
  }
  case sketch::EntityType::Circle: {
    auto *circle = static_cast<const sketch::SketchCircle *>(entity);
    double distToCenter = point.Distance(circle->center());
    return std::abs(distToCenter - circle->radius());
  }
  case sketch::EntityType::Rectangle: {
    auto *rect = static_cast<const sketch::SketchRectangle *>(entity);
    gp_Pnt2d c1 = rect->corner1();
    gp_Pnt2d c2 = rect->corner2();

    // Check distance to all 4 edges
    double minDist = std::numeric_limits<double>::max();
    gp_Pnt2d corners[4] = {c1, gp_Pnt2d(c2.X(), c1.Y()), c2,
                           gp_Pnt2d(c1.X(), c2.Y())};
    for (int i = 0; i < 4; ++i) {
      gp_Pnt2d p1 = corners[i];
      gp_Pnt2d p2 = corners[(i + 1) % 4];

      double dx = p2.X() - p1.X();
      double dy = p2.Y() - p1.Y();
      double len2 = dx * dx + dy * dy;
      if (len2 < 1e-10)
        continue;

      double t = std::max(0.0, std::min(1.0, ((point.X() - p1.X()) * dx +
                                              (point.Y() - p1.Y()) * dy) /
                                                 len2));
      gp_Pnt2d proj(p1.X() + t * dx, p1.Y() + t * dy);
      minDist = std::min(minDist, point.Distance(proj));
    }
    return minDist;
  }
  case sketch::EntityType::Arc: {
    auto *arc = static_cast<const sketch::SketchArc *>(entity);
    double distToCenter = point.Distance(arc->center());
    return std::abs(distToCenter - arc->radius());
  }
  default:
    // For other types, check distance to start/end points
    double d1 = point.Distance(entity->startPoint());
    double d2 = point.Distance(entity->endPoint());
    return std::min(d1, d2);
  }
}

void SketchView2D::moveEntity(sketch::SketchEntity *entity, double dx,
                              double dy) {
  if (!entity)
    return;

  switch (entity->type()) {
  case sketch::EntityType::Point: {
    auto *pt = dynamic_cast<sketch::SketchPoint *>(entity);
    if (pt) {
      gp_Pnt2d pos = pt->position();
      pt->setPosition(gp_Pnt2d(pos.X() + dx, pos.Y() + dy));
    }
    break;
  }
  case sketch::EntityType::Line: {
    auto *line = dynamic_cast<sketch::SketchLine *>(entity);
    if (line) {
      gp_Pnt2d p1 = line->startPoint();
      gp_Pnt2d p2 = line->endPoint();
      line->setStartPoint(gp_Pnt2d(p1.X() + dx, p1.Y() + dy));
      line->setEndPoint(gp_Pnt2d(p2.X() + dx, p2.Y() + dy));
    }
    break;
  }
  case sketch::EntityType::Circle: {
    auto *circle = dynamic_cast<sketch::SketchCircle *>(entity);
    if (circle) {
      gp_Pnt2d c = circle->center();
      circle->setCenter(gp_Pnt2d(c.X() + dx, c.Y() + dy));
    }
    break;
  }
  case sketch::EntityType::Rectangle: {
    auto *rect = dynamic_cast<sketch::SketchRectangle *>(entity);
    if (rect) {
      gp_Pnt2d c1 = rect->corner1();
      gp_Pnt2d c2 = rect->corner2();
      rect->setCorner1(gp_Pnt2d(c1.X() + dx, c1.Y() + dy));
      rect->setCorner2(gp_Pnt2d(c2.X() + dx, c2.Y() + dy));
    }
    break;
  }
  case sketch::EntityType::Arc: {
    auto *arc = dynamic_cast<sketch::SketchArc *>(entity);
    if (arc) {
      gp_Pnt2d c = arc->center();
      arc->setCenter(gp_Pnt2d(c.X() + dx, c.Y() + dy));
    }
    break;
  }
  default:
    break;
  }
}

void SketchView2D::deleteSelectedEntity() {
  if (!m_selectedEntity || !m_sketch)
    return;

  // Get entity ID before removing
  uint64_t entityId = m_selectedEntity->id();

  // Remove the entity from sketch
  m_sketch->removeEntity(entityId);

  // Clear selection
  m_selectedEntity = nullptr;
  m_hoveredEntity = nullptr;

  // Save checkpoint
  saveSketchCheckpoint("Delete Entity");

  // Notify and update
  emit entityCreated(nullptr); // Trigger 3D view update
  update();
}

// Helper to find specific control point index
int SketchView2D::findControlPointAtPoint(
    const gp_Pnt2d &worldPos, double tolerance,
    opencad::sketch::SketchEntity *&outEntity) {
  if (!m_sketch)
    return -1;

  for (auto &entity : m_sketch->entities()) {
    // Check based on type
    if (entity->type() == sketch::EntityType::Line) {
      auto line = std::dynamic_pointer_cast<sketch::SketchLine>(entity);
      if (worldPos.Distance(line->startPoint()) < tolerance) {
        outEntity = entity.get();
        return 0; // Start
      }
      if (worldPos.Distance(line->endPoint()) < tolerance) {
        outEntity = entity.get();
        return 1; // End
      }
    } else if (entity->type() == sketch::EntityType::Circle) {
      auto circle = std::dynamic_pointer_cast<sketch::SketchCircle>(entity);
      if (worldPos.Distance(circle->center()) < tolerance) {
        outEntity = entity.get();
        return 0; // Center
      }
    }
  }
  return -1;
}

void SketchView2D::modifyEntityPoint(opencad::sketch::SketchEntity *entity,
                                     int pointIndex, const gp_Pnt2d &newPos) {
  if (!entity)
    return;

  if (entity->type() == sketch::EntityType::Line) {
    auto line = dynamic_cast<sketch::SketchLine *>(entity);
    if (line) {
      if (pointIndex == 0)
        line->setStartPoint(newPos);
      else if (pointIndex == 1)
        line->setEndPoint(newPos);
    }
  } else if (entity->type() == sketch::EntityType::Circle) {
    auto circle = dynamic_cast<sketch::SketchCircle *>(entity);
    if (circle && pointIndex == 0) {
      circle->setCenter(newPos);
    }
  }
}

// ============ MULTI-SELECTION SYSTEM ============

void SketchView2D::selectEntity(sketch::SketchEntity *entity,
                                bool addToSelection) {
  if (!entity)
    return;

  if (!addToSelection) {
    clearSelection();
  }

  if (!isEntitySelected(entity)) {
    m_selectedEntities.push_back(entity);
    entity->setSelected(true);
  }

  // Update legacy single selection for compatibility
  m_selectedEntity =
      m_selectedEntities.empty() ? nullptr : m_selectedEntities.back();

  emit entitySelected(m_selectedEntity);
  update();
}

void SketchView2D::toggleEntitySelection(sketch::SketchEntity *entity) {
  if (!entity)
    return;

  auto it =
      std::find(m_selectedEntities.begin(), m_selectedEntities.end(), entity);
  if (it != m_selectedEntities.end()) {
    (*it)->setSelected(false);
    m_selectedEntities.erase(it);
  } else {
    m_selectedEntities.push_back(entity);
    entity->setSelected(true);
  }

  m_selectedEntity =
      m_selectedEntities.empty() ? nullptr : m_selectedEntities.back();
  emit entitySelected(m_selectedEntity);
  update();
}

void SketchView2D::selectEntitiesInBox(const QRectF &boxScreen) {
  if (!m_sketch)
    return;

  QRectF normalizedBox = boxScreen.normalized();

  for (auto &entity : m_sketch->entities()) {
    // Check if entity intersects with box
    gp_Pnt2d start = entity->startPoint();
    gp_Pnt2d end = entity->endPoint();

    QPointF startScreen = worldToScreen(start);
    QPointF endScreen = worldToScreen(end);

    // Simple check: if any key point is inside the box
    if (normalizedBox.contains(startScreen) ||
        normalizedBox.contains(endScreen)) {
      if (!isEntitySelected(entity.get())) {
        m_selectedEntities.push_back(entity.get());
        entity->setSelected(true);
      }
    }
  }

  m_selectedEntity =
      m_selectedEntities.empty() ? nullptr : m_selectedEntities.back();
  update();
}

void SketchView2D::clearSelection() {
  for (auto *entity : m_selectedEntities) {
    if (entity)
      entity->setSelected(false);
  }
  m_selectedEntities.clear();
  m_selectedEntity = nullptr;
  m_selectedControlPointIndex = -1;
  update();
}

void SketchView2D::selectAll() {
  if (!m_sketch)
    return;

  clearSelection();
  for (auto &entity : m_sketch->entities()) {
    m_selectedEntities.push_back(entity.get());
    entity->setSelected(true);
  }

  m_selectedEntity =
      m_selectedEntities.empty() ? nullptr : m_selectedEntities.back();
  update();
}

bool SketchView2D::isEntitySelected(sketch::SketchEntity *entity) const {
  return std::find(m_selectedEntities.begin(), m_selectedEntities.end(),
                   entity) != m_selectedEntities.end();
}

QRectF SketchView2D::getSelectionBoundingBox() const {
  if (m_selectedEntities.empty())
    return QRectF();

  double minX = std::numeric_limits<double>::max();
  double minY = std::numeric_limits<double>::max();
  double maxX = std::numeric_limits<double>::lowest();
  double maxY = std::numeric_limits<double>::lowest();

  for (auto *entity : m_selectedEntities) {
    if (!entity)
      continue;

    gp_Pnt2d start = entity->startPoint();
    gp_Pnt2d end = entity->endPoint();

    minX = std::min({minX, start.X(), end.X()});
    minY = std::min({minY, start.Y(), end.Y()});
    maxX = std::max({maxX, start.X(), end.X()});
    maxY = std::max({maxY, start.Y(), end.Y()});
  }

  QPointF topLeft = worldToScreen(gp_Pnt2d(minX, maxY));
  QPointF bottomRight = worldToScreen(gp_Pnt2d(maxX, minY));

  return QRectF(topLeft, bottomRight).normalized();
}

gp_Pnt2d SketchView2D::getSelectionCenter() const {
  if (m_selectedEntities.empty())
    return gp_Pnt2d(0, 0);

  double sumX = 0, sumY = 0;
  int count = 0;

  for (auto *entity : m_selectedEntities) {
    if (!entity)
      continue;
    gp_Pnt2d mid = entity->midPoint();
    sumX += mid.X();
    sumY += mid.Y();
    count++;
  }

  if (count == 0)
    return gp_Pnt2d(0, 0);
  return gp_Pnt2d(sumX / count, sumY / count);
}

// ============ TRANSFORM SYSTEM ============

void SketchView2D::startTransform(TransformMode mode) {
  if (m_selectedEntities.empty())
    return;

  m_transformMode = mode;
  m_transformPivot = getSelectionCenter();
  m_originalPositions.clear();

  for (auto *entity : m_selectedEntities) {
    if (entity) {
      m_originalPositions.emplace_back(entity, entity->midPoint());
    }
  }

  update();
}

void SketchView2D::applyTransform(const gp_Pnt2d &currentPos) {
  if (m_transformMode == TransformMode::None)
    return;

  switch (m_transformMode) {
  case TransformMode::Move: {
    double dx = currentPos.X() - m_dragStartWorld.X();
    double dy = currentPos.Y() - m_dragStartWorld.Y();

    // Restore original positions first
    for (auto &pair : m_originalPositions) {
      // Move from original to new position
    }

    // Apply delta to all selected
    for (auto *entity : m_selectedEntities) {
      moveEntity(entity, dx, dy);
    }
    break;
  }
  case TransformMode::Rotate: {
    double dx = currentPos.X() - m_transformPivot.X();
    double dy = currentPos.Y() - m_transformPivot.Y();
    double angle = std::atan2(dy, dx);
    rotateEntities(angle - m_transformInitialAngle);
    break;
  }
  case TransformMode::Scale: {
    double dist = currentPos.Distance(m_transformPivot);
    double initialDist = m_dragStartWorld.Distance(m_transformPivot);
    if (initialDist > 0.001) {
      double scale = dist / initialDist;
      scaleEntities(scale);
    }
    break;
  }
  default:
    break;
  }

  update();
}

void SketchView2D::finishTransform() {
  if (m_transformMode != TransformMode::None) {
    saveSketchCheckpoint("Transform");
    m_transformMode = TransformMode::None;
    m_originalPositions.clear();
  }
}

void SketchView2D::cancelTransform() {
  // Restore original positions
  for (auto &pair : m_originalPositions) {
    // Restore entity to original position (simplified)
  }
  m_transformMode = TransformMode::None;
  m_originalPositions.clear();
  update();
}

void SketchView2D::rotateEntities(double angleDelta) {
  double cosA = std::cos(angleDelta);
  double sinA = std::sin(angleDelta);

  for (auto *entity : m_selectedEntities) {
    if (!entity)
      continue;

    // Get entity center relative to pivot
    gp_Pnt2d mid = entity->midPoint();
    double rx = mid.X() - m_transformPivot.X();
    double ry = mid.Y() - m_transformPivot.Y();

    // Rotate
    double newX = rx * cosA - ry * sinA + m_transformPivot.X();
    double newY = rx * sinA + ry * cosA + m_transformPivot.Y();

    // Move entity
    double dx = newX - mid.X();
    double dy = newY - mid.Y();
    moveEntity(entity, dx, dy);
  }
}

void SketchView2D::scaleEntities(double scaleFactor) {
  for (auto *entity : m_selectedEntities) {
    if (!entity)
      continue;

    gp_Pnt2d mid = entity->midPoint();
    double rx = mid.X() - m_transformPivot.X();
    double ry = mid.Y() - m_transformPivot.Y();

    double newX = rx * scaleFactor + m_transformPivot.X();
    double newY = ry * scaleFactor + m_transformPivot.Y();

    double dx = newX - mid.X();
    double dy = newY - mid.Y();
    moveEntity(entity, dx, dy);
  }
}

void SketchView2D::moveEntities(double dx, double dy) {
  for (auto *entity : m_selectedEntities) {
    moveEntity(entity, dx, dy);
  }
}

// ============ CLIPBOARD SYSTEM ============

void SketchView2D::copySelection() {
  m_clipboard.clear();
  if (m_selectedEntities.empty())
    return;

  m_clipboardCenter = getSelectionCenter();

  for (auto *entity : m_selectedEntities) {
    if (entity) {
      m_clipboard.push_back(entity->clone());
    }
  }

  qDebug() << "Copied" << m_clipboard.size() << "entities to clipboard";
}

void SketchView2D::cutSelection() {
  copySelection();
  deleteSelectedEntities();
}

void SketchView2D::pasteClipboard() {
  if (m_clipboard.empty() || !m_sketch)
    return;

  clearSelection();

  // Offset paste position slightly from original
  double offsetX = 20.0 / m_scale;
  double offsetY = -20.0 / m_scale;

  for (auto &entityPtr : m_clipboard) {
    auto clone = entityPtr->clone();

    // Move clone to offset position
    gp_Pnt2d mid = clone->midPoint();
    double dx = offsetX;
    double dy = offsetY;

    // Add to sketch
    if (clone->type() == sketch::EntityType::Line) {
      auto line = std::dynamic_pointer_cast<sketch::SketchLine>(clone);
      if (line) {
        line->setStartPoint(
            gp_Pnt2d(line->startPoint().X() + dx, line->startPoint().Y() + dy));
        line->setEndPoint(
            gp_Pnt2d(line->endPoint().X() + dx, line->endPoint().Y() + dy));
      }
    } else if (clone->type() == sketch::EntityType::Circle) {
      auto circle = std::dynamic_pointer_cast<sketch::SketchCircle>(clone);
      if (circle) {
        circle->setCenter(
            gp_Pnt2d(circle->center().X() + dx, circle->center().Y() + dy));
      }
    }

    m_sketch->addEntity(clone);
    m_selectedEntities.push_back(clone.get());
    clone->setSelected(true);
  }

  m_selectedEntity =
      m_selectedEntities.empty() ? nullptr : m_selectedEntities.back();
  saveSketchCheckpoint("Paste");
  update();

  qDebug() << "Pasted" << m_clipboard.size() << "entities";
}

void SketchView2D::deleteSelectedEntities() {
  if (m_selectedEntities.empty() || !m_sketch)
    return;

  for (auto *entity : m_selectedEntities) {
    if (entity) {
      m_sketch->removeEntity(entity->id());
    }
  }

  clearSelection();
  saveSketchCheckpoint("Delete Entities");
  emit entityCreated(nullptr);
  update();
}

// ============ VISUAL FEEDBACK ============

void SketchView2D::drawSelectionHandles(QPainter &painter) {
  if (m_selectedEntities.empty())
    return;

  painter.save();

  const double handleSize = 8.0;
  QPen handlePen(QColor(0, 100, 255), 2);
  QBrush handleBrush(QColor(255, 255, 255));

  painter.setPen(handlePen);
  painter.setBrush(handleBrush);

  for (auto *entity : m_selectedEntities) {
    if (!entity)
      continue;

    // Draw handles at key points
    QPointF start = worldToScreen(entity->startPoint());
    QPointF end = worldToScreen(entity->endPoint());
    QPointF mid = worldToScreen(entity->midPoint());

    // Start point handle
    painter.drawRect(QRectF(start.x() - handleSize / 2,
                            start.y() - handleSize / 2, handleSize,
                            handleSize));

    // End point handle
    painter.drawRect(QRectF(end.x() - handleSize / 2, end.y() - handleSize / 2,
                            handleSize, handleSize));

    // Center/mid handle (different style)
    painter.setBrush(QColor(0, 100, 255));
    painter.drawEllipse(mid, handleSize / 2, handleSize / 2);
    painter.setBrush(handleBrush);
  }

  painter.restore();
}

void SketchView2D::drawBoundingBox(QPainter &painter) {
  if (m_selectedEntities.size() <= 1)
    return;

  QRectF bbox = getSelectionBoundingBox();
  if (bbox.isEmpty())
    return;

  painter.save();

  QPen dashPen(QColor(0, 100, 255, 150), 1, Qt::DashLine);
  painter.setPen(dashPen);
  painter.setBrush(Qt::NoBrush);

  // Expand bbox slightly
  bbox.adjust(-5, -5, 5, 5);
  painter.drawRect(bbox);

  // Draw corner handles
  const double handleSize = 8.0;
  painter.setPen(QPen(QColor(0, 100, 255), 2));
  painter.setBrush(QColor(255, 255, 255));

  QPointF corners[] = {bbox.topLeft(), bbox.topRight(), bbox.bottomLeft(),
                       bbox.bottomRight()};
  for (const auto &corner : corners) {
    painter.drawRect(QRectF(corner.x() - handleSize / 2,
                            corner.y() - handleSize / 2, handleSize,
                            handleSize));
  }

  painter.restore();
}

void SketchView2D::drawBoxSelection(QPainter &painter) {
  if (!m_isBoxSelecting)
    return;

  painter.save();

  QRectF selectionBox = QRectF(m_boxSelectStart, m_boxSelectEnd).normalized();

  // Fill
  painter.setBrush(QColor(0, 100, 255, 30));
  painter.setPen(QPen(QColor(0, 100, 255), 1, Qt::DashLine));
  painter.drawRect(selectionBox);

  painter.restore();
}

} // namespace ui
} // namespace opencad
