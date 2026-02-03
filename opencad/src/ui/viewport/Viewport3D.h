#pragma once
/**
 * @file Viewport3D.h
 * @brief 3D viewport widget using OpenCASCADE visualization
 *
 * OpenCAD - Modular CAD/CAE Platform
 * UI Module
 */

#include "core/geometry/Shape.h"

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <QOpenGLWidget>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>
#include <gp_Pln.hxx>
#include <vector>

namespace opencad {
namespace ui {

/**
 * @brief Selection mode for viewport
 */
enum class SelectionMode {
  None,
  Shape,  // Select whole shapes
  Face,   // Select faces
  Edge,   // Select edges
  Vertex, // Select vertices
  Mate    // Select faces, edges, or vertices for mating
};

/**
 * @class Viewport3D
 * @brief OpenGL widget for 3D CAD visualization
 */
class Viewport3D : public QOpenGLWidget {
  Q_OBJECT

public:
  explicit Viewport3D(QWidget *parent = nullptr);
  ~Viewport3D() override;

  /**
   * @brief Display a shape in the viewport
   * @return The created AIS handle for tracking
   */
  Handle(AIS_Shape) displayShape(const core::Shape &shape);

  /**
   * @brief Display a TopoDS_Shape directly
   * @return The created AIS handle for tracking
   */
  Handle(AIS_Shape) displayShape(const TopoDS_Shape &shape);

  /**
   * @brief Display a sketch wire with holographic appearance (cyan, thick
   * lines)
   */
  void displaySketchWire(const TopoDS_Shape &wire);

  /**
   * @brief Display sketch plane with grid and axes (SolidWorks style)
   * @param plane The sketch plane orientation
   * @param center Center point of the visible plane (where to expand from)
   * @param size Size of the visible plane rectangle
   */
  void displaySketchPlane(const gp_Pln &plane, const gp_Pnt &center,
                          double size = 100.0);

  /**
   * @brief Clear all displayed shapes
   */
  void clearAll();

  /**
   * @brief Clear display (alias for clearAll)
   */
  void clearDisplay() { clearAll(); }

  /**
   * @brief Fit all objects in view
   */
  void fitAll();

  // View orientations
  void setViewFront();
  void setViewBack();
  void setViewTop();
  void setViewBottom();
  void setViewLeft();
  void setViewRight();
  void setViewIsometric();

  /**
   * @brief Get the interactive context
   */
  Handle(AIS_InteractiveContext) context() const { return m_context; }

  /**
   * @brief Set selection mode
   */
  void setSelectionMode(SelectionMode mode);
  SelectionMode selectionMode() const { return m_selectionMode; }

  /**
   * @brief Enable/disable face selection for sketch placement
   */
  void enableFaceSelection(bool enable);

  /**
   * @brief Enable/disable edge selection for fillet/chamfer
   */
  void enableEdgeSelection(bool enable);

  /**
   * @brief Get selected face (if any)
   */
  TopoDS_Face selectedFace() const { return m_selectedFace; }

  /**
   * @brief Get selected edges
   */
  std::vector<TopoDS_Edge> getSelectedEdges() const { return m_selectedEdges; }

  /**
   * @brief Clear selected edges
   */
  void clearSelectedEdges() { m_selectedEdges.clear(); }

  /**
   * @brief Get plane of selected face
   */
  bool getSelectedFacePlane(gp_Pln &plane) const;

  /**
   * @brief Get selected shape (any, used for Assembly)
   */
  TopoDS_Shape getSelectedShape() const { return m_selectedShape; }

  /**
   * @brief Enable/disable shape selection
   */
  void enableShapeSelection(bool enable);

  /**
   * @brief Enable/disable mate selection (Face/Edge/Vertex)
   */
  void enableMateSelection(bool enable);

signals:
  void faceSelected(); // Emitted when a face is selected
  void edgeSelected(); // Emitted when an edge is selected
  void selectionCleared();
  void geometrySelected(
      const QString &type); // Emitted when any geometry is selected
  void shapeSelected(const TopoDS_Shape &shape,
                     Handle(AIS_InteractiveObject) object);
  // Component drag signals
  void componentDragStarted(Handle(AIS_InteractiveObject) object);
  void componentDragged(Handle(AIS_InteractiveObject) object, gp_Vec delta);
  void componentDragEnded(Handle(AIS_InteractiveObject) object,
                          gp_Pnt dropPoint);

public:
  /**
   * @brief Enable component drag mode for assembly operations
   */
  void enableComponentDragMode(bool enable);
  bool isComponentDragMode() const { return m_componentDragMode; }

protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int w, int h) override;

  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;

private:
  void initViewer();
  void handlePick(int x, int y);

  Handle(AIS_InteractiveContext) m_context;
  Handle(V3d_View) m_view;
  Handle(V3d_Viewer) m_viewer;
  Handle(OpenGl_GraphicDriver) m_driver;
  Handle(Aspect_DisplayConnection) m_displayConnection;

  QPoint m_lastMousePos;
  bool m_rotating = false;
  bool m_panning = false;

  // Selection
  SelectionMode m_selectionMode = SelectionMode::Shape;
  TopoDS_Face m_selectedFace;
  TopoDS_Shape m_selectedShape;
  std::vector<TopoDS_Edge> m_selectedEdges;
  bool m_faceSelectionEnabled = false;
  bool m_edgeSelectionEnabled = false;

  // Store displayed shapes for selection
  std::vector<Handle(AIS_Shape)> m_aisShapes;

  // Component drag mode
  bool m_componentDragMode = false;
  bool m_isDragging = false;
  Handle(AIS_InteractiveObject) m_draggedObject;
  gp_Pnt m_dragStartPoint;
  QPoint m_dragStartMousePos;
};

} // namespace ui
} // namespace opencad
