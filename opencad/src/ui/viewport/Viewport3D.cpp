/**
 * @file Viewport3D.cpp
 * @brief 3D viewport implementation
 */

#include "Viewport3D.h"

#include <QMessageBox>
#include <QMouseEvent>
#include <QWheelEvent>

#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <Aspect_Handle.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Bnd_Box.hxx>
#include <GeomAbs_SurfaceType.hxx>
#include <Graphic3d_GraphicDriver.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <QDebug>
#include <StdSelect_BRepOwner.hxx>
#include <TopoDS.hxx>
#include <V3d_View.hxx>
#include <V3d_Viewer.hxx>

#ifdef _WIN32
#include <WNT_Window.hxx>
#else
#include <Xw_Window.hxx>
#endif

namespace opencad {
namespace ui {

Viewport3D::Viewport3D(QWidget *parent) : QOpenGLWidget(parent) {
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);

  // Enable multisampling for smoother edges
  QSurfaceFormat format;
  format.setDepthBufferSize(24);
  format.setStencilBufferSize(8);
  format.setSamples(4);
  setFormat(format);
}

Viewport3D::~Viewport3D() {
  if (!m_view.IsNull()) {
    m_view->Remove();
  }
}

void Viewport3D::initViewer() {
  // Create display connection
  m_displayConnection = new Aspect_DisplayConnection();

  // Create graphic driver
  m_driver = new OpenGl_GraphicDriver(m_displayConnection);

  // Create viewer
  m_viewer = new V3d_Viewer(m_driver);
  m_viewer->SetDefaultLights();
  m_viewer->SetLightOn();

  // Create AIS context
  m_context = new AIS_InteractiveContext(m_viewer);
  m_context->SetDisplayMode(AIS_Shaded, true);

  // Create view
  m_view = m_viewer->CreateView();

  // Create window handle
#ifdef _WIN32
  Handle(WNT_Window) window =
      new WNT_Window(reinterpret_cast<Aspect_Handle>(winId()));
#else
  Handle(Xw_Window) window = new Xw_Window(
      m_displayConnection, reinterpret_cast<Aspect_Drawable>(winId()));
#endif

  m_view->SetWindow(window);
  if (!window->IsMapped()) {
    window->Map();
  }

  // Set background gradient
  Quantity_Color topColor(0.2, 0.3, 0.5, Quantity_TOC_RGB);
  Quantity_Color bottomColor(0.8, 0.85, 0.9, Quantity_TOC_RGB);
  m_view->SetBgGradientColors(topColor, bottomColor,
                              Aspect_GradientFillMethod_Vertical);

  // Set initial viewpoint
  m_view->SetProj(V3d_XposYposZpos);
  // Enable selection for faces by default
  m_context->SetAutomaticHilight(true);
}

void Viewport3D::initializeGL() { initViewer(); }

void Viewport3D::paintGL() {
  if (!m_view.IsNull()) {
    m_view->Redraw();
  }
}

void Viewport3D::resizeGL(int w, int h) {
  if (!m_view.IsNull()) {
    m_view->MustBeResized();
  }
}

void Viewport3D::displayShape(const core::Shape &shape) {
  if (m_context.IsNull())
    return;

  Handle(AIS_Shape) aisShape = new AIS_Shape(shape.occShape());
  m_context->Display(aisShape, AIS_Shaded, 0, true);
  m_aisShapes.push_back(aisShape); // Store for selection
  fitAll();
}

void Viewport3D::displayShape(const TopoDS_Shape &shape) {
  if (m_context.IsNull())
    return;

  qDebug() << "Viewport3D::displayShape - Shape IsNull:" << shape.IsNull();

  if (shape.IsNull()) {
    qDebug() << "Viewport3D::displayShape - WARNING: Received null shape!";
    return;
  }

  // Compute mesh tessellation for proper display
  qDebug() << "Viewport3D::displayShape - Computing mesh tessellation...";
  try {
    BRepMesh_IncrementalMesh mesh(shape, 0.1);
    mesh.Perform();
    qDebug() << "Viewport3D::displayShape - Mesh tessellation completed";
  } catch (...) {
    qDebug() << "Viewport3D::displayShape - Mesh tessellation failed";
  }

  Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
  qDebug() << "Viewport3D::displayShape - Displaying shape in AIS context...";
  m_context->Display(aisShape, AIS_Shaded, 0, true);
  m_aisShapes.push_back(aisShape); // Store for selection
  qDebug() << "Viewport3D::displayShape - Shape displayed, total AIS shapes:"
           << m_aisShapes.size();
  fitAll();
  qDebug() << "Viewport3D::displayShape - fitAll() completed";
}

void Viewport3D::displaySketchWire(const TopoDS_Shape &wire) {
  if (m_context.IsNull())
    return;

  Handle(AIS_Shape) aisShape = new AIS_Shape(wire);

  // Holographic effect - wireframe mode with extra thick glowing lines
  m_context->SetDisplayMode(aisShape, AIS_WireFrame, false);

  // Bright cyan/green holographic color
  aisShape->SetColor(Quantity_NOC_CYAN1); // Brighter cyan

  // Extra thick lines for holographic glow effect
  aisShape->SetWidth(4.0);

  // Add slight transparency for holographic look
  aisShape->SetTransparency(0.1);

  // Display without selection (sketch is just for visual reference)
  m_context->Display(aisShape, false);

  // Also display a "glow" outline with thicker semi-transparent lines
  Handle(AIS_Shape) glowShape = new AIS_Shape(wire);
  m_context->SetDisplayMode(glowShape, AIS_WireFrame, false);
  glowShape->SetColor(Quantity_NOC_TURQUOISE); // Outer glow color
  glowShape->SetWidth(8.0);                    // Thicker for glow
  glowShape->SetTransparency(0.6);             // More transparent
  m_context->Display(glowShape, false);
}

void Viewport3D::displaySketchPlane(const gp_Pln &plane, const gp_Pnt &center,
                                    double size) {
  if (m_context.IsNull())
    return;

  // Use the provided center point instead of plane.Location()
  gp_Pnt origin = center;
  gp_Dir normal = plane.Axis().Direction();
  gp_Dir xDir = plane.XAxis().Direction();
  gp_Dir yDir = plane.YAxis().Direction();

  // Create rectangle corners
  double halfSize = size / 2.0;
  gp_Pnt p1 = origin.Translated(gp_Vec(xDir) * (-halfSize) +
                                gp_Vec(yDir) * (-halfSize));
  gp_Pnt p2 =
      origin.Translated(gp_Vec(xDir) * halfSize + gp_Vec(yDir) * (-halfSize));
  gp_Pnt p3 =
      origin.Translated(gp_Vec(xDir) * halfSize + gp_Vec(yDir) * halfSize);
  gp_Pnt p4 =
      origin.Translated(gp_Vec(xDir) * (-halfSize) + gp_Vec(yDir) * halfSize);

  // Build rectangle wire
  BRepBuilderAPI_MakeWire wireBuilder;
  wireBuilder.Add(BRepBuilderAPI_MakeEdge(p1, p2).Edge());
  wireBuilder.Add(BRepBuilderAPI_MakeEdge(p2, p3).Edge());
  wireBuilder.Add(BRepBuilderAPI_MakeEdge(p3, p4).Edge());
  wireBuilder.Add(BRepBuilderAPI_MakeEdge(p4, p1).Edge());

  if (!wireBuilder.IsDone())
    return;

  // Create face from wire
  BRepBuilderAPI_MakeFace faceBuilder(wireBuilder.Wire());
  if (!faceBuilder.IsDone())
    return;

  TopoDS_Face planeFace = faceBuilder.Face();

  // Display plane as semi-transparent blue
  Handle(AIS_Shape) aisPlane = new AIS_Shape(planeFace);
  aisPlane->SetColor(Quantity_NOC_LIGHTBLUE);
  aisPlane->SetTransparency(0.7); // 70% transparent
  m_context->Display(aisPlane, AIS_Shaded, 0, false);

  // Display grid lines on the plane
  double gridStep = size / 10.0;
  for (int i = -5; i <= 5; i++) {
    // Horizontal lines
    gp_Pnt lStart = origin.Translated(gp_Vec(xDir) * (-halfSize) +
                                      gp_Vec(yDir) * (i * gridStep));
    gp_Pnt lEnd = origin.Translated(gp_Vec(xDir) * halfSize +
                                    gp_Vec(yDir) * (i * gridStep));

    TopoDS_Edge hLine = BRepBuilderAPI_MakeEdge(lStart, lEnd).Edge();
    Handle(AIS_Shape) hLineAis = new AIS_Shape(hLine);
    hLineAis->SetColor(Quantity_NOC_GRAY50);
    hLineAis->SetWidth(1.0);
    m_context->Display(hLineAis, false);

    // Vertical lines
    gp_Pnt vStart = origin.Translated(gp_Vec(xDir) * (i * gridStep) +
                                      gp_Vec(yDir) * (-halfSize));
    gp_Pnt vEnd = origin.Translated(gp_Vec(xDir) * (i * gridStep) +
                                    gp_Vec(yDir) * halfSize);

    TopoDS_Edge vLine = BRepBuilderAPI_MakeEdge(vStart, vEnd).Edge();
    Handle(AIS_Shape) vLineAis = new AIS_Shape(vLine);
    vLineAis->SetColor(Quantity_NOC_GRAY50);
    vLineAis->SetWidth(1.0);
    m_context->Display(vLineAis, false);
  }

  // Display axes on plane (X=red, Y=green)
  gp_Pnt xAxisEnd = origin.Translated(gp_Vec(xDir) * (halfSize * 0.8));
  gp_Pnt yAxisEnd = origin.Translated(gp_Vec(yDir) * (halfSize * 0.8));

  TopoDS_Edge xAxis = BRepBuilderAPI_MakeEdge(origin, xAxisEnd).Edge();
  Handle(AIS_Shape) xAxisAis = new AIS_Shape(xAxis);
  xAxisAis->SetColor(Quantity_NOC_RED);
  xAxisAis->SetWidth(2.0);
  m_context->Display(xAxisAis, false);

  TopoDS_Edge yAxis = BRepBuilderAPI_MakeEdge(origin, yAxisEnd).Edge();
  Handle(AIS_Shape) yAxisAis = new AIS_Shape(yAxis);
  yAxisAis->SetColor(Quantity_NOC_GREEN);
  yAxisAis->SetWidth(2.0);
  m_context->Display(yAxisAis, false);
}

void Viewport3D::clearAll() {
  if (!m_context.IsNull()) {
    m_context->RemoveAll(true);
  }
  m_aisShapes.clear();
}

void Viewport3D::fitAll() {
  if (!m_view.IsNull()) {
    m_view->FitAll();
    m_view->ZFitAll();
    update();
  }
}

void Viewport3D::setViewFront() {
  if (!m_view.IsNull()) {
    m_view->SetProj(V3d_Yneg);
    fitAll();
  }
}

void Viewport3D::setViewBack() {
  if (!m_view.IsNull()) {
    m_view->SetProj(V3d_Ypos);
    fitAll();
  }
}

void Viewport3D::setViewTop() {
  if (!m_view.IsNull()) {
    m_view->SetProj(V3d_Zpos);
    fitAll();
  }
}

void Viewport3D::setViewBottom() {
  if (!m_view.IsNull()) {
    m_view->SetProj(V3d_Zneg);
    fitAll();
  }
}

void Viewport3D::setViewLeft() {
  if (!m_view.IsNull()) {
    m_view->SetProj(V3d_Xneg);
    fitAll();
  }
}

void Viewport3D::setViewRight() {
  if (!m_view.IsNull()) {
    m_view->SetProj(V3d_Xpos);
    fitAll();
  }
}

void Viewport3D::setViewIsometric() {
  if (!m_view.IsNull()) {
    m_view->SetProj(V3d_XposYposZpos);
    fitAll();
  }
}

// Face selection methods
void Viewport3D::setSelectionMode(SelectionMode mode) {
  m_selectionMode = mode;
  if (m_context.IsNull())
    return;

  // Deactivate all current selections first
  for (const auto &aisShape : m_aisShapes) {
    m_context->Deactivate(aisShape);
  }

  // Map selection mode to TopAbs type
  int selMode = 0;
  switch (mode) {
  case SelectionMode::None:
    return; // Leave deactivated
  case SelectionMode::Shape:
    selMode = AIS_Shape::SelectionMode(TopAbs_SHAPE);
    break;
  case SelectionMode::Face:
    selMode = AIS_Shape::SelectionMode(TopAbs_FACE);
    break;
  case SelectionMode::Edge:
    selMode = AIS_Shape::SelectionMode(TopAbs_EDGE);
    break;
  case SelectionMode::Vertex:
    selMode = AIS_Shape::SelectionMode(TopAbs_VERTEX);
    break;
  }

  // Activate selection for each shape
  for (const auto &aisShape : m_aisShapes) {
    m_context->Activate(aisShape, selMode);
  }
}

void Viewport3D::enableFaceSelection(bool enable) {
  m_faceSelectionEnabled = enable;
  if (enable) {
    setSelectionMode(SelectionMode::Face);
    setCursor(Qt::CrossCursor);
  } else {
    setSelectionMode(SelectionMode::Shape);
    setCursor(Qt::ArrowCursor);
    m_selectedFace.Nullify();
  }
  update();
}

void Viewport3D::enableEdgeSelection(bool enable) {
  m_edgeSelectionEnabled = enable;
  if (enable) {
    m_selectedEdges.clear();
    setSelectionMode(SelectionMode::Edge);
    setCursor(Qt::CrossCursor);
    // DEBUG: Show popup when edge selection is enabled
    QMessageBox::information(
        nullptr, "Edge Mode",
        QString("Edge selection ENABLED!\nm_aisShapes count: %1")
            .arg(m_aisShapes.size()));
  } else {
    setSelectionMode(SelectionMode::Shape);
    setCursor(Qt::ArrowCursor);
  }
  update();
}

void Viewport3D::handlePick(int x, int y) {
  if (m_context.IsNull() || m_view.IsNull())
    return;

  // Move to position and detect what's there
  m_context->MoveTo(x, y, m_view, true);

  // Only pick if we have a selection mode active
  if (m_selectionMode == SelectionMode::None)
    return;

  // Perform selection using new OCCT 7.8+ API
  m_context->SelectDetected();

  if (m_context->NbSelected() > 0) {
    m_context->InitSelected();
    if (m_context->MoreSelected()) {
      Handle(StdSelect_BRepOwner) owner =
          Handle(StdSelect_BRepOwner)::DownCast(m_context->SelectedOwner());

      if (!owner.IsNull() && owner->HasShape()) {
        TopoDS_Shape selected = owner->Shape();

        // Handle based on selection mode
        switch (m_selectionMode) {
        case SelectionMode::Face:
          if (selected.ShapeType() == TopAbs_FACE) {
            m_selectedFace = TopoDS::Face(selected);
            emit faceSelected();
            emit geometrySelected("Face");
          }
          break;
        case SelectionMode::Edge:
          if (selected.ShapeType() == TopAbs_EDGE) {
            TopoDS_Edge edge = TopoDS::Edge(selected);
            m_selectedEdges.push_back(edge);
            emit edgeSelected();
            emit geometrySelected(
                "Edge (" + QString::number(m_selectedEdges.size()) + ")");
            // DEBUG: Show popup when edge is selected
            QMessageBox::information(
                nullptr, "Edge Selected",
                QString("Edge added! Total: %1").arg(m_selectedEdges.size()));
          }
          break;
        case SelectionMode::Vertex:
          if (selected.ShapeType() == TopAbs_VERTEX) {
            emit geometrySelected("Vertex");
          }
          break;
        case SelectionMode::Shape:
          emit geometrySelected("Shape");
          break;
        default:
          break;
        }
        update();
      }
    }
  } else {
    m_selectedFace.Nullify();
    emit selectionCleared();
  }
}

bool Viewport3D::getSelectedFacePlane(gp_Pln &plane) const {
  if (m_selectedFace.IsNull())
    return false;

  try {
    BRepAdaptor_Surface adaptor(m_selectedFace);
    GeomAbs_SurfaceType surfType = adaptor.GetType();

    // Calculate face bounding box center for plane origin
    Bnd_Box faceBox;
    BRepBndLib::Add(m_selectedFace, faceBox);

    gp_Pnt faceCenter;
    if (!faceBox.IsVoid()) {
      double xMin, yMin, zMin, xMax, yMax, zMax;
      faceBox.Get(xMin, yMin, zMin, xMax, yMax, zMax);
      faceCenter =
          gp_Pnt((xMin + xMax) / 2.0, (yMin + yMax) / 2.0, (zMin + zMax) / 2.0);
    } else {
      // Fallback to UV mid point
      double uMid =
          (adaptor.FirstUParameter() + adaptor.LastUParameter()) / 2.0;
      double vMid =
          (adaptor.FirstVParameter() + adaptor.LastVParameter()) / 2.0;
      faceCenter = adaptor.Value(uMid, vMid);
    }

    // For explicit plane surfaces, use direct method
    if (surfType == GeomAbs_Plane) {
      gp_Pln originalPlane = adaptor.Plane();
      // Create new plane with same direction but origin at face center
      plane = gp_Pln(faceCenter, originalPlane.Axis().Direction());
      return true;
    }

    // For any other surface type, calculate plane from UV derivatives
    double uMid = (adaptor.FirstUParameter() + adaptor.LastUParameter()) / 2.0;
    double vMid = (adaptor.FirstVParameter() + adaptor.LastVParameter()) / 2.0;

    gp_Pnt point;
    gp_Vec d1u, d1v;
    adaptor.D1(uMid, vMid, point, d1u, d1v);

    // Calculate normal from cross product of derivatives
    gp_Vec normal = d1u.Crossed(d1v);
    double mag = normal.Magnitude();

    if (mag > 1e-10) {
      normal.Normalize();
      // Use face center as origin instead of UV mid point
      plane = gp_Pln(faceCenter, gp_Dir(normal));
      return true;
    }

    // Fallback: try corners
    double u1 = adaptor.FirstUParameter();
    double v1 = adaptor.FirstVParameter();
    adaptor.D1(u1, v1, point, d1u, d1v);
    normal = d1u.Crossed(d1v);
    mag = normal.Magnitude();

    if (mag > 1e-10) {
      normal.Normalize();
      plane = gp_Pln(faceCenter, gp_Dir(normal));
      return true;
    }

  } catch (...) {
    return false;
  }
  return false;
}

void Viewport3D::mousePressEvent(QMouseEvent *event) {
  m_lastMousePos = event->pos();

  // Handle left click for selection (in any selection mode)
  if (event->button() == Qt::LeftButton &&
      m_selectionMode != SelectionMode::None) {
    handlePick(event->pos().x(), event->pos().y());
    return;
  }

  if (event->button() == Qt::MiddleButton) {
    if (event->modifiers() & Qt::ShiftModifier) {
      m_panning = true;
    } else {
      m_rotating = true;
    }
  }
}

void Viewport3D::mouseReleaseEvent(QMouseEvent *event) {
  m_rotating = false;
  m_panning = false;
}

void Viewport3D::mouseMoveEvent(QMouseEvent *event) {
  if (m_view.IsNull())
    return;

  QPoint delta = event->pos() - m_lastMousePos;
  m_lastMousePos = event->pos();

  // Update highlight when in selection mode
  if (m_faceSelectionEnabled && !m_context.IsNull()) {
    m_context->MoveTo(event->pos().x(), event->pos().y(), m_view, true);
    update();
  }

  if (m_rotating) {
    m_view->Rotation(event->pos().x(), event->pos().y());
    update();
  } else if (m_panning) {
    m_view->Pan(delta.x(), -delta.y());
    update();
  }
}

void Viewport3D::wheelEvent(QWheelEvent *event) {
  if (m_view.IsNull())
    return;

  double delta = event->angleDelta().y();
  double factor = delta > 0 ? 1.1 : 0.9;

  m_view->SetZoom(factor);
  update();
}

} // namespace ui
} // namespace opencad
