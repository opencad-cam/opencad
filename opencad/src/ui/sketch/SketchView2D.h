#pragma once
/**
 * @file SketchView2D.h
 * @brief 2D Sketch view widget with mouse interaction
 *
 * OpenCAD - Modular CAD/CAE Platform
 */

#include <BRepBuilderAPI_MakeFace.hxx>
#include <QInputDialog>
#include <QMainWindow>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPainter>
#include <QPointF>
#include <QWheelEvent>
#include <QWidget>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <deque>
#include <gp_Pnt2d.hxx>
#include <memory>
#include <string>
#include <vector>

namespace opencad {
namespace sketch {
class Sketch;
class SketchEntity;
} // namespace sketch
namespace ui {

/**
 * @brief Drawing tool types
 */
enum class SketchToolType {
  None,
  Select,
  Line,
  Rectangle,
  Circle,
  Arc,
  Point,
  Spline,
  Ellipse,
  Polygon,       // Regular polygon tool
  Slot,          // Slot (elongated hole) tool
  Offset,        // Offset entities tool
  Dimension,     // Smart dimension tool for setting lengths and radii
  ProfileSelect, // For selecting closed profiles during Extrude/Cut
  PointSelect,   // For picking a specific 2D coordinate/snap on the sketch
  Trim           // For PowerTrimming sketch entities
};

/**
 * @class SketchView2D
 * @brief Interactive 2D sketch editing widget
 */
class SketchView2D : public QWidget {
  Q_OBJECT

public:
  explicit SketchView2D(QWidget *parent = nullptr);
  ~SketchView2D() override;

  /// Set the sketch to edit
  void setSketch(std::shared_ptr<sketch::Sketch> sketch);
  std::shared_ptr<sketch::Sketch> sketch() const { return m_sketch; }

  /// Set current drawing tool
  void setTool(SketchToolType tool);
  SketchToolType currentTool() const { return m_currentTool; }

  /// Grid settings
  void setGridSize(double size) {
    m_gridSize = size;
    update();
  }
  double gridSize() const { return m_gridSize; }
  void setSnapToGrid(bool snap) { m_snapToGrid = snap; }
  bool snapToGrid() const { return m_snapToGrid; }
  void setShowGrid(bool show) {
    m_showGrid = show;
    update();
  }
  bool showGrid() const { return m_showGrid; }

  /// Auto-dimension settings
  void setShowDimensions(bool show) {
    m_showDimensions = show;
    update();
  }
  bool showDimensions() const { return m_showDimensions; }

  /// Auto-constraint settings
  void setAutoConstraint(bool enabled) { m_autoConstraint = enabled; }
  bool autoConstraint() const { return m_autoConstraint; }
  void setAutoConstraintTolerance(double degrees) {
    m_autoConstraintAngleTolerance = degrees;
  }
  double autoConstraintTolerance() const {
    return m_autoConstraintAngleTolerance;
  }

  /// Polygon settings
  void setPolygonSides(int sides) {
    m_polygonSides = qBound(3, sides, 32);
    update();
  }
  int polygonSides() const { return m_polygonSides; }
  void setPolygonInscribed(bool inscribed) {
    m_polygonInscribed = inscribed;
    update();
  }
  bool polygonInscribed() const { return m_polygonInscribed; }

  /// Slot settings
  void setSlotWidth(double width) {
    m_slotWidth = qMax(0.5, width);
    update();
  }
  double slotWidth() const { return m_slotWidth; }

  /// View controls
  void fitAll();
  void zoomIn();
  void zoomOut();
  void resetView();

  /// Selection
  sketch::SketchEntity *selectedEntity() const { return m_selectedEntity; }
  void clearSelection(); // Implemented in cpp

  /// Handle ESC key press (called from MainWindow global shortcut)
  void handleEscPress();

signals:
  void entityCreated(sketch::SketchEntity *entity);
  void entitySelected(sketch::SketchEntity *entity);
  void toolChanged(SketchToolType tool);
  void cursorPositionChanged(double x, double y);
  void sketchExitRequested(); // Emitted when ESC pressed in Select mode

  // Profile selection signals
  void profileSelected(int profileIndex);
  void ringSelected(int outerProfileIndex, int innerProfileIndex);
  void multiProfilesConfirmed(
      const std::vector<int> &selections); // indices of selected regions
  void profileHovered(int profileIndex);
  void profileSelectionConfirmed(); // Emitted when Enter pressed in
                                    // ProfileSelect mode
  void profileSelectionCancelled();

  // Emitted when selecting a point in PointSelect tool
  void pointSelected(double x, double y);

protected:
  void paintEvent(QPaintEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  void resizeEvent(QResizeEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

private:
  // Coordinate conversion
  QPointF worldToScreen(const gp_Pnt2d &world) const;
  gp_Pnt2d screenToWorld(const QPointF &screen) const;

  // Advanced snap system
  enum class SnapType {
    None,
    Grid,
    Endpoint,
    Midpoint,
    Center,
    Intersection,
    OnCurve,
    Quadrant
  };

  struct SnapResult {
    gp_Pnt2d point;
    SnapType type = SnapType::None;
    sketch::SketchEntity *entity = nullptr;
  };

  SnapResult findBestSnap(const gp_Pnt2d &point) const;
  std::vector<gp_Pnt2d> findEndpoints() const;
  std::vector<gp_Pnt2d> findMidpoints() const;
  std::vector<gp_Pnt2d> findCenters() const;
  std::vector<gp_Pnt2d> findIntersections() const;
  gp_Pnt2d findNearestOnCurve(const gp_Pnt2d &point,
                              sketch::SketchEntity *entity) const;
  void drawSnapIndicator(QPainter &painter, const SnapResult &snap);

  // Drawing
  void drawGrid(QPainter &painter);
  void drawEntities(QPainter &painter);
  void drawClosedProfileFaces(
      QPainter &painter); // Draw filled faces for closed profiles
  void drawEntity(QPainter &painter, const sketch::SketchEntity *entity);
  void drawDimensions(QPainter &painter);
  void drawPreview(QPainter &painter);
  void drawCursor(QPainter &painter);

  // Profile selection drawing and detection
  void drawProfileOverlays(QPainter &painter);
  int findProfileAtPoint(const QPointF &screenPos);

  // Tool operations
  void finishCurrentEntity();
  void cancelCurrentEntity();

public:
  // Profile selection mode
  void enterProfileSelectMode(bool allowOpenProfiles = false);
  void exitProfileSelectMode();
  const std::vector<TopoDS_Shape> &getProfiles() const { return m_profiles; }

  // Data
  std::shared_ptr<sketch::Sketch> m_sketch;
  SketchToolType m_currentTool = SketchToolType::Select;

  // View transform
  double m_scale = 5.0; // pixels per unit
  QPointF m_offset;     // view offset in pixels

  // Grid
  double m_gridSize = 10.0;
  bool m_snapToGrid = true;
  bool m_showGrid = true;
  bool m_showDimensions = true;
  bool m_autoConstraint = true; // Auto-apply constraints when drawing
  double m_autoConstraintAngleTolerance =
      5.0; // Degrees tolerance for H/V detection

  // Mouse state
  bool m_isDrawing = false;
  gp_Pnt2d m_startPoint;
  gp_Pnt2d m_currentPoint;
  QPointF m_lastMousePos;
  bool m_isPanning = false;
  std::vector<gp_Pnt2d> m_splinePoints; // For spline construction

  // Snap state
  SnapResult m_currentSnap;
  SnapResult
      m_startSnap; // Snap at line start point (for coincident constraints)
  double m_snapRadius = 15.0; // pixels

  // Polygon tool settings
  int m_polygonSides = 6;
  bool m_polygonInscribed = true; // true = inscribed, false = circumscribed

  // Arc tool settings
  double m_arcPrevAngle = 0.0; // Previous angle for direction detection
  double m_arcCumulativeAngle =
      0.0;                       // Accumulated angle change (+ = CCW, - = CW)
  gp_Pnt2d m_arcMidPoint;        // For 3-point arc mode (third click)
  bool m_arcHasMidPoint = false; // Whether mid point is set

  // Slot tool settings
  double m_slotWidth = 10.0; // Default slot width

  // Preview points (e.g. for Hole Wizard point selection visualization)
  std::vector<gp_Pnt2d> m_previewPoints;
  void setPreviewPoints(const std::vector<gp_Pnt2d> &points) {
    m_previewPoints = points;
    update();
  }
  void clearPreviewPoints() {
    m_previewPoints.clear();
    update();
  }

  // ============================================
  // Dimension Tool State
  // ============================================
  // First entity selected in dimension tool (for two-entity distance dimension)
  sketch::SketchEntity *m_dimFirstEntity = nullptr;
  // Position clicked on first entity (used to determine reference point)
  gp_Pnt2d m_dimFirstClickPos;

  // ============================================
  // Multi-Selection System
  // ============================================
  std::vector<sketch::SketchEntity *> m_selectedEntities; // Multi-selection
  sketch::SketchEntity *m_selectedEntity =
      nullptr; // Legacy single selection (for compatibility)
  sketch::SketchEntity *m_hoveredEntity = nullptr;
  bool m_isDragging = false;
  gp_Pnt2d m_dragStartWorld;
  gp_Pnt2d m_entityOriginalPos;         // Original position before drag
  bool m_escPressedOnce = false;        // For double-ESC to exit sketch
  int m_selectedControlPointIndex = -1; // -1: none, 0: start, 1: end/other
  bool m_isDraggingPoint = false;

  // Box selection
  bool m_isBoxSelecting = false;
  QPointF m_boxSelectStart;
  QPointF m_boxSelectEnd;

  // Selection helpers
  void selectEntity(sketch::SketchEntity *entity, bool addToSelection = false);
  void toggleEntitySelection(sketch::SketchEntity *entity);
  void selectEntitiesInBox(const QRectF &boxScreen);
  void selectAll();
  bool isEntitySelected(sketch::SketchEntity *entity) const;
  QRectF getSelectionBoundingBox() const;
  gp_Pnt2d getSelectionCenter() const;

  // ============================================
  // Transform System
  // ============================================
  enum class TransformMode { None, Move, Rotate, Scale };
  TransformMode m_transformMode = TransformMode::None;
  gp_Pnt2d m_transformPivot;            // Transform center point
  double m_transformInitialAngle = 0.0; // For rotation
  double m_transformInitialScale = 1.0; // For scaling
  std::vector<std::pair<sketch::SketchEntity *, gp_Pnt2d>>
      m_originalPositions; // Entity -> original center

  void startTransform(TransformMode mode);
  void applyTransform(const gp_Pnt2d &currentPos);
  void finishTransform();
  void cancelTransform();
  void rotateEntities(double angleDelta);
  void scaleEntities(double scaleFactor);
  void moveEntities(double dx, double dy);

  // ============================================
  // Clipboard System
  // ============================================
  std::vector<std::shared_ptr<sketch::SketchEntity>> m_clipboard;
  gp_Pnt2d m_clipboardCenter; // Center of copied entities

  void copySelection();
  void cutSelection();
  void pasteClipboard();
  void deleteSelectedEntities();

  // Entity hit-testing and selection
  sketch::SketchEntity *findEntityAtPoint(const gp_Pnt2d &worldPos,
                                          double tolerance);
  // Returns point index (0, 1, etc.) or -1 if none. Updates outEntity.
  int findControlPointAtPoint(const gp_Pnt2d &worldPos, double tolerance,
                              sketch::SketchEntity *&outEntity);

  double distanceToEntity(const gp_Pnt2d &point,
                          const sketch::SketchEntity *entity) const;
  void moveEntity(sketch::SketchEntity *entity, double dx, double dy);
  void modifyEntityPoint(sketch::SketchEntity *entity, int pointIndex,
                         const gp_Pnt2d &newPos);
  void deleteSelectedEntity(); // Legacy single delete

  // Visual feedback
  void drawSelectionHandles(QPainter &painter);
  void drawBoundingBox(QPainter &painter);
  void drawBoxSelection(QPainter &painter);

  // Profile selection mode
  // Profile selection mode
  int m_hoveredProfileIndex = -1; // Currently hovered profile
  std::vector<TopoDS_Shape>
      m_profiles; // Cached profiles (Faces or Open Wires/Edges)

  // Multi-selection support (indices)
  std::vector<int> m_selectedProfiles;
  bool m_multiSelectMode = true; // Always multi-select mode

  // Colors
  const QColor m_gridColor{200, 200, 200};
  const QColor m_axisXColor{255, 0, 0};
  const QColor m_axisYColor{0, 255, 0};
  const QColor m_entityColor{0, 0, 0};
  const QColor m_selectedColor{0, 100, 255};
  const QColor m_previewColor{100, 100, 255};
  const QColor m_dimensionColor{50, 150, 50};
  const QColor m_snapEndpointColor{255, 165, 0};     // Orange
  const QColor m_snapMidpointColor{0, 191, 255};     // Deep Sky Blue
  const QColor m_snapCenterColor{255, 0, 255};       // Magenta
  const QColor m_snapIntersectionColor{255, 255, 0}; // Yellow
  const QColor m_snapOnCurveColor{0, 255, 128};      // Spring Green

  // Profile selection colors
  const std::vector<QColor> m_profileColors = {
      QColor(255, 100, 100, 150), // Red
      QColor(100, 255, 100, 150), // Green
      QColor(100, 100, 255, 150), // Blue
      QColor(255, 255, 100, 150), // Yellow
      QColor(255, 100, 255, 150), // Magenta
      QColor(100, 255, 255, 150)  // Cyan
  };

  // ============================================
  // Sketch Undo/Redo System
  // ============================================
public:
  /// Undo last sketch operation
  void undo();

  /// Redo last undone operation
  void redo();

  /// Check if undo is available
  bool canUndo() const { return m_sketchHistoryIndex > 0; }

  /// Check if redo is available
  bool canRedo() const {
    return m_sketchHistoryIndex < static_cast<int>(m_sketchHistory.size()) - 1;
  }

private:
  /// Save current sketch state as checkpoint
  void saveSketchCheckpoint(const std::string &description);

  /// Sketch state snapshot
  struct SketchSnapshot {
    std::vector<std::shared_ptr<sketch::SketchEntity>> entities;
    std::string description;
  };

  std::deque<SketchSnapshot> m_sketchHistory;
  int m_sketchHistoryIndex = -1;
  static constexpr size_t m_maxSketchHistory = 50;
};

} // namespace ui
} // namespace opencad
