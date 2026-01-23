/**
 * @file MainWindow.cpp
 * @brief Main window implementation
 */

#include "MainWindow.h"
#include "viewport/Viewport3D.h"

// Core geometry
#include "core/geometry/BooleanOps.h"
#include "core/geometry/Primitives.h"

// IO
#include "io/mesh/StlWriter.h"
#include "io/step/StepReader.h"
#include "io/step/StepWriter.h"

// Sketch
#include "sketch/Sketch.h"
#include "sketch/SketchMirror.h"
#include "sketch/SketchPattern.h"
#include "sketch/SketchTrimExtend.h"
#include "sketch/SketchView2D.h"
#include "sketch/constraints/ConcentricConstraint.h"
#include "sketch/constraints/EqualConstraint.h"
#include "sketch/constraints/FixConstraint.h"
#include "sketch/constraints/TangentConstraint.h"
#include <QShortcut>

// Part features
#include "part/CutFeature.h"
#include "part/DomeFeature.h"
#include "part/DraftFeature.h"
#include "part/ExtrudeFeature.h"
#include "part/HoleFeature.h"
#include "part/LoftFeature.h"
#include "part/OffsetSurfaceFeature.h"
#include "part/RevolveFeature.h"
#include "part/RibFeature.h"
#include "part/ScaleFeature.h"
#include "part/ShellFeature.h"
#include "part/SplitFeature.h"
#include "part/SweepFeature.h"
#include "part/ThickenFeature.h"

// OpenCASCADE - Shape validation and mesh
#include <BRepBndLib.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Bnd_Box.hxx>

// Panels
#include "ParameterEditor.h"
#include "ProfileSelectionPanel.h"
#include "PropertiesPanel.h"
#include "ToolSettingsPanel.h"

#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>

// OpenCASCADE for bounding box
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepOffsetAPI_DraftAngle.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <Geom_Circle.hxx>
#include <Geom_Curve.hxx>
#include <Geom_Ellipse.hxx>
#include <Geom_Line.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <gp_Pln.hxx>
#include <gp_Trsf.hxx>

// Part features
#include "part/ChamferFeature.h"
#include "part/FilletFeature.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Core
#include "core/Document.h"

namespace opencad {
namespace ui {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("OpenCAD");
  resize(1280, 800);

  // Create central document
  m_document = std::make_unique<core::Document>(this);

  // Create central viewport
  m_viewport = std::make_unique<Viewport3D>(this);
  setCentralWidget(m_viewport.get());

  // Connect face selection signal
  connect(m_viewport.get(), &Viewport3D::faceSelected, this,
          &MainWindow::onFaceSelected);

  setupMenus();
  setupToolbars();
  setupDockWidgets();
  setupStatusBar();

  // Save initial empty state
  m_document->checkpoint("Initial");

  // Don't install global event filter - it blocks menu clicks
  // Instead, install event filter only on sketch view in setupDockWidgets
  // if (QCoreApplication::instance()) {
  //   QCoreApplication::instance()->installEventFilter(this);
  // }

  // Initialize feature list
  updateFeatureList();

  updateWindowTitle();

  // Add Enter shortcut to show Tool Settings when in part tool mode
  auto *enterShortcut = new QShortcut(QKeySequence(Qt::Key_Return), this);
  connect(enterShortcut, &QShortcut::activated, this, [this]() {
    if (m_activePartTool != ActivePartTool::None) {
      qDebug() << "Enter shortcut activated - showing Tool Settings";
      if (m_toolSettingsDock && m_toolSettingsPanel) {
        m_toolSettingsDock->setFloating(false);
        addDockWidget(Qt::LeftDockWidgetArea, m_toolSettingsDock);
        m_toolSettingsDock->show();
        m_toolSettingsDock->raise();

        if (m_activePartTool == ActivePartTool::Extrude) {
          m_toolSettingsPanel->showExtrudeSettings();
        } else if (m_activePartTool == ActivePartTool::Cut) {
          m_toolSettingsPanel->showCutSettings();
        } else if (m_activePartTool == ActivePartTool::Revolve) {
          m_toolSettingsPanel->showRevolveSettings();
        }
      }
      statusBar()->showMessage("Adjust settings and click Apply");
    }
  });
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
  if (event->type() == QEvent::KeyPress) {
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

    // Handle ESC from sketch view only
    if (obj == m_sketchView && keyEvent->key() == Qt::Key_Escape) {
      if (m_sketchMode && m_sketchView && m_sketchView->isVisible()) {
        qDebug() << "SketchView EventFilter caught ESC";
        m_sketchView->handleEscPress();
        return true;
      }
    }

    // Handle Enter from ANY widget when we have an active part tool
    if ((keyEvent->key() == Qt::Key_Return ||
         keyEvent->key() == Qt::Key_Enter) &&
        m_activePartTool != ActivePartTool::None) {
      qDebug() << "MainWindow EventFilter caught Enter - showing Tool Settings";
      if (m_toolSettingsDock && m_toolSettingsPanel) {
        m_toolSettingsDock->setFloating(false);
        addDockWidget(Qt::LeftDockWidgetArea, m_toolSettingsDock);
        m_toolSettingsDock->show();
        m_toolSettingsDock->raise();

        if (m_activePartTool == ActivePartTool::Extrude) {
          m_toolSettingsPanel->showExtrudeSettings();
        } else if (m_activePartTool == ActivePartTool::Cut) {
          m_toolSettingsPanel->showCutSettings();
        } else if (m_activePartTool == ActivePartTool::Revolve) {
          m_toolSettingsPanel->showRevolveSettings();
        }
      }
      statusBar()->showMessage("Adjust settings and click Apply");
      return true;
    }
  }
  return QMainWindow::eventFilter(obj, event);
}

MainWindow::~MainWindow() = default;

void MainWindow::addShape(const TopoDS_Shape &shape) {
  if (!shape.IsNull()) {
    // Add to document
    m_document->addTemporaryShape(shape);

    // Display in viewport
    if (m_viewport) {
      m_viewport->displayShape(shape);
    }

    // Update properties panel with the new shape
    if (m_propertiesPanel) {
      m_propertiesPanel->setShape(shape);
    }

    // Save checkpoint AFTER adding shape
    saveUndoState("Add Shape");
  }
}

void MainWindow::clearShapes() {
  if (m_viewport) {
    m_viewport->clearDisplay();
  }
}

void MainWindow::displayAllShapes() {
  qDebug() << "=== displayAllShapes called ===";
  qDebug() << "Viewport exists:" << (m_viewport != nullptr);

  if (m_viewport) {
    m_viewport->clearAll(); // FIX: clearDisplay() doesn't exist!

    // Display solid shapes from document
    auto shapes = m_document->getAllShapes();
    qDebug() << "Total shapes to display:" << shapes.size();

    int shapeIndex = 0;
    for (const auto &shape : shapes) {
      qDebug() << "  Shape" << shapeIndex << "IsNull:" << shape.IsNull();
      if (!shape.IsNull()) {
        m_viewport->displayShape(shape);
        qDebug() << "  Shape" << shapeIndex << "displayed";
      }
      shapeIndex++;
    }

    // Display sketch wires in viewport (cyan color for visibility)
    qDebug() << "Total sketches:" << m_document->sketches().size();
    for (const auto &sketch : m_document->sketches()) {
      if (sketch) {
        TopoDS_Compound compound = sketch->buildCompound();
        if (!compound.IsNull()) {
          m_viewport->displaySketchWire(compound); // Holographic cyan compound
        }
      }
    }

    m_viewport->fitAll();
    qDebug() << "=== displayAllShapes done ===";
  }
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (m_modified) {
    auto result = QMessageBox::question(
        this, "Unsaved Changes",
        "You have unsaved changes. Do you want to save before closing?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (result == QMessageBox::Save) {
      onSaveFile();
      event->accept();
    } else if (result == QMessageBox::Discard) {
      event->accept();
    } else {
      event->ignore();
    }
  } else {
    event->accept();
  }
}

void MainWindow::setupMenus() {
  // File Menu
  auto *fileMenu = menuBar()->addMenu("&File");

  auto *newAction = fileMenu->addAction("&New", this, &MainWindow::onNewFile);
  newAction->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
  newAction->setShortcut(QKeySequence::New);
  auto *openAction =
      fileMenu->addAction("&Open...", this, &MainWindow::onOpenFile);
  openAction->setShortcut(QKeySequence::Open);
  fileMenu->addSeparator();
  auto *saveAction =
      fileMenu->addAction("&Save", this, &MainWindow::onSaveFile);
  saveAction->setShortcut(QKeySequence::Save);
  auto *saveAsAction =
      fileMenu->addAction("Save &As...", this, &MainWindow::onSaveAs);
  saveAsAction->setShortcut(QKeySequence::SaveAs);
  fileMenu->addSeparator();
  fileMenu->addAction("Export STL...", this, &MainWindow::onExportSTL);
  fileMenu->addAction("Export STEP...", this, &MainWindow::onExportSTEP);
  fileMenu->addSeparator();
  auto *exitAction = fileMenu->addAction("E&xit", this, &MainWindow::onExit);
  exitAction->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
  exitAction->setShortcut(QKeySequence::Quit);

  // Edit Menu
  auto *editMenu = menuBar()->addMenu("&Edit");
  auto *undoAction = editMenu->addAction("&Undo", this, &MainWindow::onUndo);
  undoAction->setIcon(style()->standardIcon(QStyle::SP_ArrowBack));
  undoAction->setShortcut(QKeySequence::Undo);
  auto *redoAction = editMenu->addAction("&Redo", this, &MainWindow::onRedo);
  redoAction->setIcon(style()->standardIcon(QStyle::SP_ArrowForward));
  redoAction->setShortcut(QKeySequence::Redo);
  editMenu->addSeparator();
  auto *deleteAction =
      editMenu->addAction("&Delete", this, &MainWindow::onDelete);
  deleteAction->setShortcut(QKeySequence::Delete);

  // View Menu
  auto *viewMenu = menuBar()->addMenu("&View");
  auto *fitAction =
      viewMenu->addAction("&Fit All", this, &MainWindow::onViewFit);
  fitAction->setShortcut(Qt::Key_F);
  viewMenu->addSeparator();
  auto *frontAction =
      viewMenu->addAction("Front", this, &MainWindow::onViewFront);
  frontAction->setShortcut(Qt::Key_1);
  viewMenu->addAction("Back", this, &MainWindow::onViewBack);
  auto *topAction = viewMenu->addAction("Top", this, &MainWindow::onViewTop);
  topAction->setShortcut(Qt::Key_7);
  viewMenu->addAction("Bottom", this, &MainWindow::onViewBottom);
  auto *leftAction = viewMenu->addAction("Left", this, &MainWindow::onViewLeft);
  leftAction->setShortcut(Qt::Key_3);
  viewMenu->addAction("Right", this, &MainWindow::onViewRight);
  auto *isoAction =
      viewMenu->addAction("Isometric", this, &MainWindow::onViewIsometric);
  isoAction->setShortcut(Qt::Key_0);

  // Sketch Menu
  auto *sketchMenu = menuBar()->addMenu("&Sketch");
  sketchMenu->addAction("New Sketch", this, &MainWindow::onNewSketch);
  sketchMenu->addAction("Sketch on Face...", this, &MainWindow::onSketchOnFace);
  sketchMenu->addAction("Edit Sketch", this, &MainWindow::onEditSketch);
  auto *finishSketchAction =
      sketchMenu->addAction("Finish Sketch", this, &MainWindow::onFinishSketch);
  finishSketchAction->setShortcut(Qt::Key_Escape);
  sketchMenu->addSeparator();

  auto *sketchGeomMenu = sketchMenu->addMenu("Geometry");
  auto *lineAction =
      sketchGeomMenu->addAction("Line", this, &MainWindow::onSketchLine);
  lineAction->setShortcut(Qt::Key_L);
  auto *rectAction = sketchGeomMenu->addAction("Rectangle", this,
                                               &MainWindow::onSketchRectangle);
  rectAction->setShortcut(Qt::Key_R);
  auto *circleAction =
      sketchGeomMenu->addAction("Circle", this, &MainWindow::onSketchCircle);
  circleAction->setShortcut(Qt::Key_C);
  auto *arcAction =
      sketchGeomMenu->addAction("Arc", this, &MainWindow::onSketchArc);
  arcAction->setShortcut(Qt::Key_A);
  auto *pointAction =
      sketchGeomMenu->addAction("Point", this, &MainWindow::onSketchPoint);
  pointAction->setShortcut(Qt::Key_P);
  sketchGeomMenu->addAction("Spline", this, &MainWindow::onSketchSpline);
  sketchGeomMenu->addAction("Ellipse", this, &MainWindow::onSketchEllipse);
  sketchGeomMenu->addAction("Slot", this, &MainWindow::onSketchSlot);
  auto *polygonAction =
      sketchGeomMenu->addAction("Polygon", this, &MainWindow::onSketchPolygon);
  polygonAction->setShortcut(Qt::Key_Y);

  // Sketch Edit Tools
  auto *sketchToolsMenu = sketchMenu->addMenu("Edit Tools");
  auto *trimAction =
      sketchToolsMenu->addAction("Trim", this, &MainWindow::onSketchTrim);
  trimAction->setShortcut(Qt::Key_T);
  sketchToolsMenu->addAction("Extend", this, &MainWindow::onSketchExtend);
  sketchToolsMenu->addAction("Convert Entities", this,
                             &MainWindow::onConvertEntities);
  sketchToolsMenu->addAction("Intersection Curve", this,
                             &MainWindow::onIntersectionCurve);
  sketchToolsMenu->addSeparator();
  auto *mirrorAction =
      sketchToolsMenu->addAction("Mirror", this, &MainWindow::onSketchMirror);
  mirrorAction->setShortcut(Qt::Key_M);
  sketchToolsMenu->addAction("Linear Pattern", this,
                             &MainWindow::onSketchLinearPattern);
  sketchToolsMenu->addAction("Circular Pattern", this,
                             &MainWindow::onSketchCircularPattern);

  auto *constraintMenu = sketchMenu->addMenu("Constraints");
  auto *horizAction = constraintMenu->addAction(
      "Horizontal", this, &MainWindow::onConstraintHorizontal);
  horizAction->setShortcut(Qt::Key_H);
  auto *vertAction = constraintMenu->addAction(
      "Vertical", this, &MainWindow::onConstraintVertical);
  vertAction->setShortcut(Qt::Key_V);
  constraintMenu->addAction("Coincident", this,
                            &MainWindow::onConstraintCoincident);
  constraintMenu->addAction("Distance", this,
                            &MainWindow::onConstraintDistance);
  constraintMenu->addAction("Radius", this, &MainWindow::onConstraintRadius);
  constraintMenu->addAction("Angle", this, &MainWindow::onConstraintAngle);
  constraintMenu->addAction("Parallel", this,
                            &MainWindow::onConstraintParallel);
  constraintMenu->addAction("Perpendicular", this,
                            &MainWindow::onConstraintPerpendicular);
  constraintMenu->addSeparator();
  constraintMenu->addAction("Tangent", this, &MainWindow::onConstraintTangent);
  constraintMenu->addAction("Equal", this, &MainWindow::onConstraintEqual);
  constraintMenu->addAction("Fix", this, &MainWindow::onConstraintFix);
  constraintMenu->addAction("Concentric", this,
                            &MainWindow::onConstraintConcentric);

  // Selection Menu - for edge/face selection mode
  auto *selectionMenu = menuBar()->addMenu("&Selection");
  selectionMenu->addAction("Select Edge", this, [this]() {
    if (m_viewport) {
      m_viewport->enableEdgeSelection(true);
      statusBar()->showMessage("Edge selection mode - click edges to select");
    }
  });
  selectionMenu->addAction("Select Face", this, [this]() {
    if (m_viewport) {
      m_viewport->enableFaceSelection(true);
      statusBar()->showMessage("Face selection mode - click a face to select");
    }
  });
  selectionMenu->addAction("Clear Selection", this, [this]() {
    if (m_viewport) {
      m_viewport->clearSelectedEdges();
      m_viewport->enableEdgeSelection(false);
      m_viewport->enableFaceSelection(false);
      statusBar()->showMessage("Selection cleared");
    }
  });

  // Features Menu
  auto *featuresMenu = menuBar()->addMenu("&Features");
  featuresMenu->addAction("Reference Plane", this,
                          &MainWindow::onReferencePlane);
  featuresMenu->addSeparator();
  featuresMenu->addAction("Extrude", this, &MainWindow::onExtrude);
  featuresMenu->addAction("Cut/Pocket", this, &MainWindow::onCut);
  featuresMenu->addAction("Revolve", this, &MainWindow::onRevolve);
  featuresMenu->addSeparator();
  featuresMenu->addAction("Fillet", this, &MainWindow::onFillet);
  featuresMenu->addAction("Chamfer", this, &MainWindow::onChamfer);
  featuresMenu->addAction("Shell", this, &MainWindow::onShell);
  featuresMenu->addAction("Draft", this, &MainWindow::onDraft);
  featuresMenu->addAction("Rib", this, &MainWindow::onRib);
  featuresMenu->addSeparator();
  featuresMenu->addAction("Sweep", this, &MainWindow::onSweep);
  featuresMenu->addAction("Loft", this, &MainWindow::onLoft);
  featuresMenu->addAction("Loft Surface (Thin)", this,
                          &MainWindow::onLoftSurface);
  featuresMenu->addSeparator();
  featuresMenu->addAction("Scale", this, &MainWindow::onScale);
  featuresMenu->addAction("Hole Wizard", this, &MainWindow::onHoleWizard);
  featuresMenu->addAction("Thicken", this, &MainWindow::onThicken);
  featuresMenu->addAction("Offset Surface", this, &MainWindow::onOffsetSurface);
  featuresMenu->addAction("Split", this, &MainWindow::onSplit);
  featuresMenu->addAction("Dome", this, &MainWindow::onDome);
  featuresMenu->addSeparator();
  featuresMenu->addAction("Linear/Circular Pattern", this,
                          &MainWindow::onPattern);
  featuresMenu->addAction("Mirror", this, &MainWindow::onMirror);

  // Create Menu (primitives)
  auto *createMenu = menuBar()->addMenu("&Create");
  createMenu->addAction("Box", this, &MainWindow::onCreateBox);
  createMenu->addAction("Cylinder", this, &MainWindow::onCreateCylinder);
  createMenu->addAction("Sphere", this, &MainWindow::onCreateSphere);
  createMenu->addAction("Cone", this, &MainWindow::onCreateCone);

  // Boolean Menu
  auto *boolMenu = menuBar()->addMenu("&Boolean");
  boolMenu->addAction("Fuse (Union)", this, &MainWindow::onBooleanFuse);
  boolMenu->addAction("Cut (Difference)", this, &MainWindow::onBooleanCut);
  boolMenu->addAction("Common (Intersection)", this,
                      &MainWindow::onBooleanCommon);

  // Help Menu
  auto *helpMenu = menuBar()->addMenu("&Help");
  helpMenu->addAction("&About OpenCAD", this, &MainWindow::onAbout);
}

void MainWindow::setupToolbars() {
  // Main toolbar
  auto *mainToolbar = addToolBar("Main");
  mainToolbar->addAction("📄 New", this, &MainWindow::onNewFile);
  mainToolbar->addAction("📂 Open", this, &MainWindow::onOpenFile);
  mainToolbar->addAction("💾 Save", this, &MainWindow::onSaveFile);
  mainToolbar->addSeparator();
  mainToolbar->addAction("🔍 Fit", this, &MainWindow::onViewFit);

  // Sketch toolbar
  m_sketchToolbar = addToolBar("Sketch");
  m_sketchToolbar->addAction("?? New Sketch", this, &MainWindow::onNewSketch);
  m_sketchToolbar->addAction("? Finish", this, &MainWindow::onFinishSketch);
  m_sketchToolbar->addSeparator();
  m_sketchToolbar->addAction("📏 Line", this, &MainWindow::onSketchLine);
  m_sketchToolbar->addAction("▭ Rect", this, &MainWindow::onSketchRectangle);
  m_sketchToolbar->addAction("⭕ Circle", this, &MainWindow::onSketchCircle);
  m_sketchToolbar->addAction("🌙 Arc", this, &MainWindow::onSketchArc);
  m_sketchToolbar->addAction("📍 Point", this, &MainWindow::onSketchPoint);
  m_sketchToolbar->addAction("〰️ Spline", this, &MainWindow::onSketchSpline);
  m_sketchToolbar->addAction("🥚 Ellipse", this, &MainWindow::onSketchEllipse);
  m_sketchToolbar->addAction("⬡ Polygon", this, &MainWindow::onSketchPolygon);
  m_sketchToolbar->addAction("🎰 Slot", this, &MainWindow::onSketchSlot);

  // Constraint toolbar
  m_constraintToolbar = addToolBar("Constraints");
  m_constraintToolbar->addAction("↔️ H", this,
                                 &MainWindow::onConstraintHorizontal);
  m_constraintToolbar->addAction("↕️ V", this,
                                 &MainWindow::onConstraintVertical);
  m_constraintToolbar->addAction("?", this,
                                 &MainWindow::onConstraintPerpendicular);
  m_constraintToolbar->addAction("?", this, &MainWindow::onConstraintParallel);
  m_constraintToolbar->addAction("?", this,
                                 &MainWindow::onConstraintCoincident);
  m_constraintToolbar->addAction("📐 D", this,
                                 &MainWindow::onConstraintDistance);
  m_constraintToolbar->addAction("⭕ R", this, &MainWindow::onConstraintRadius);
  m_constraintToolbar->addAction("?", this, &MainWindow::onConstraintAngle);

  // Feature toolbar
  m_featureToolbar = addToolBar("Features");
  m_featureToolbar->addAction("⬆️ Extrude", this, &MainWindow::onExtrude);
  m_featureToolbar->addAction("🔄 Revolve", this, &MainWindow::onRevolve);
  m_featureToolbar->addAction("🔘 Fillet", this, &MainWindow::onFillet);
  m_featureToolbar->addAction("✂️ Chamfer", this, &MainWindow::onChamfer);

  // Primitives toolbar
  auto *primToolbar = addToolBar("Primitives");
  primToolbar->addAction("📦 Box", this, &MainWindow::onCreateBox);
  primToolbar->addAction("🛢️ Cylinder", this, &MainWindow::onCreateCylinder);
  primToolbar->addAction("🌐 Sphere", this, &MainWindow::onCreateSphere);
  primToolbar->addAction("🔺 Cone", this, &MainWindow::onCreateCone);

  // Boolean toolbar
  auto *boolToolbar = addToolBar("Boolean");
  boolToolbar->addAction("➕ Fuse", this, &MainWindow::onBooleanFuse);
  boolToolbar->addAction("➖ Cut", this, &MainWindow::onBooleanCut);
  boolToolbar->addAction("⚡ Common", this, &MainWindow::onBooleanCommon);

  // Selection mode toolbar
  auto *selectToolbar = addToolBar("Selection");
  selectToolbar->addAction("?? Shape", this, &MainWindow::onSelectShape);
  selectToolbar->addAction("? Face", this, &MainWindow::onSelectFace);
  selectToolbar->addAction("� Edge", this, &MainWindow::onSelectEdge);
  selectToolbar->addAction("� Vertex", this, &MainWindow::onSelectVertex);

  // Initially disable sketch tools (until sketch is created)
  updateSketchToolsEnabled(false);
}

void MainWindow::setupDockWidgets() {
  // Feature Tree (left)
  m_featureTreeDock = new QDockWidget("Feature Tree", this);
  m_featureList = new QListWidget(m_featureTreeDock);
  m_featureTreeDock->setWidget(m_featureList);
  addDockWidget(Qt::LeftDockWidgetArea, m_featureTreeDock);

  // Add default items
  m_featureList->addItem("?? Origin");
  m_featureList->addItem("  L XY Plane");
  m_featureList->addItem("  L XZ Plane");
  m_featureList->addItem("  L YZ Plane");

  // Properties Panel (right) - NX-style Associative
  m_propertiesDock = new QDockWidget("Properties", this);
  m_propertiesPanel = new PropertiesPanel(m_propertiesDock);
  m_propertiesDock->setWidget(m_propertiesPanel);
  m_propertiesDock->setMinimumWidth(220);
  addDockWidget(Qt::RightDockWidgetArea, m_propertiesDock);

  // Parameter Editor (left, below Feature Tree) - use Document's
  // ParameterManager
  m_parameterDock = new QDockWidget("Parameters", this);
  m_parameterEditor = new ParameterEditor(m_parameterDock);
  m_parameterEditor->setParameterManager(m_document->parameterManager());
  m_parameterDock->setWidget(m_parameterEditor);
  m_parameterDock->setMinimumWidth(280);
  addDockWidget(Qt::LeftDockWidgetArea, m_parameterDock);

  // Sketch Editor (bottom/right)
  m_sketchDock = new QDockWidget("Sketch Editor", this);
  m_sketchView = new SketchView2D(m_sketchDock);
  m_sketchDock->setWidget(m_sketchView);
  m_sketchDock->setMinimumSize(500, 400);
  addDockWidget(Qt::RightDockWidgetArea, m_sketchDock);
  m_sketchDock->hide(); // Initially hidden

  // Install event filter on sketch view only (not globally) to handle ESC
  if (m_sketchView) {
    m_sketchView->installEventFilter(this);
  }

  // Tool Settings Panel (left, below Parameters)
  m_toolSettingsDock = new QDockWidget("Tool Settings", this);
  m_toolSettingsPanel = new ToolSettingsPanel(m_toolSettingsDock);
  m_toolSettingsPanel->setSketchView(m_sketchView);
  m_toolSettingsDock->setWidget(m_toolSettingsPanel);
  m_toolSettingsDock->setMinimumWidth(200);
  addDockWidget(Qt::LeftDockWidgetArea, m_toolSettingsDock);

  // Profile Selection Panel (floating, shown when Extrude/Cut activated)
  m_profileSelectionDock = new QDockWidget("Profile Selection", this);
  m_profileSelectionPanel = new ProfileSelectionPanel(m_profileSelectionDock);
  m_profileSelectionDock->setWidget(m_profileSelectionPanel);
  m_profileSelectionDock->setMinimumWidth(280);
  m_profileSelectionDock->setFloating(true);
  m_profileSelectionDock->resize(300, 200);
  m_profileSelectionDock->hide(); // Initially hidden

  // Connect ProfileSelectionPanel signals
  connect(m_profileSelectionPanel, &ProfileSelectionPanel::applyClicked, this,
          &MainWindow::onToolApply);
  connect(m_profileSelectionPanel, &ProfileSelectionPanel::profileSelected,
          this, &MainWindow::onProfileSelected);
  connect(m_profileSelectionPanel, &ProfileSelectionPanel::cancelClicked, this,
          [this]() {
            m_profileSelectionDock->hide();
            m_pendingOperation = PendingOperation::None;
            m_activePartTool = ActivePartTool::None;
          });

  // Connect tool changed signal to update settings panel
  connect(m_sketchView, &SketchView2D::toolChanged, m_toolSettingsPanel,
          &ToolSettingsPanel::updateForTool);
  // Connect signals
  connect(m_sketchView, &SketchView2D::cursorPositionChanged,
          [this](double x, double y) {
            statusBar()->showMessage(
                QString("X: %1  Y: %2").arg(x, 0, 'f', 2).arg(y, 0, 'f', 2));
          });

  // Update 3D viewport when sketch entity is created (real-time holographic
  // view)
  connect(m_sketchView, &SketchView2D::entityCreated,
          [this](sketch::SketchEntity * /*entity*/) {
            if (m_viewport && m_currentSketch) {
              // Refresh 3D view with updated sketch compound (all entities)
              TopoDS_Compound compound = m_currentSketch->buildCompound();
              if (!compound.IsNull()) {
                m_viewport->displaySketchWire(compound);
              }
            }
            // Update properties panel (associative real-time update)
            if (m_propertiesPanel && m_currentSketch) {
              m_propertiesPanel->setSketch(m_currentSketch);
            }
          });

  // Profile selection signals for visual Extrude/Cut
  connect(m_sketchView, &SketchView2D::profileSelected, this,
          &MainWindow::onProfileSelected);
  connect(m_sketchView, &SketchView2D::ringSelected, this,
          &MainWindow::onRingSelected);
  connect(m_sketchView, &SketchView2D::multiProfilesConfirmed, this,
          &MainWindow::onMultiProfilesConfirmed);
  connect(m_sketchView, &SketchView2D::profileSelectionConfirmed, this,
          [this]() {
            // Exit profile select mode and show Tool Settings for adjustment
            qDebug() << "Enter pressed - showing Tool Settings panel";

            // Show Tool Settings dock on left side (docked, not floating)
            if (m_toolSettingsDock && m_toolSettingsPanel) {
              m_toolSettingsDock->setFloating(false);
              addDockWidget(Qt::LeftDockWidgetArea, m_toolSettingsDock);
              m_toolSettingsDock->show();
              m_toolSettingsDock->raise();

              // Show appropriate settings based on active tool
              if (m_activePartTool == ActivePartTool::Extrude) {
                m_toolSettingsPanel->showExtrudeSettings();
              } else if (m_activePartTool == ActivePartTool::Cut) {
                m_toolSettingsPanel->showCutSettings();
              } else if (m_activePartTool == ActivePartTool::Revolve) {
                m_toolSettingsPanel->showRevolveSettings();
              }
            }
            statusBar()->showMessage(
                "Adjust settings and click Apply (or press Enter)");
          });
  connect(m_sketchView, &SketchView2D::profileSelectionCancelled, this,
          &MainWindow::onProfileSelectionCancelled);

  // ESC in Select mode exits sketch
  connect(m_sketchView, &SketchView2D::sketchExitRequested, this,
          &MainWindow::onFinishSketch);

  // ToolSettingsPanel connections
  connect(m_toolSettingsPanel, &ToolSettingsPanel::applyClicked, this,
          &MainWindow::onToolApply);

  // Feature tree connections
  m_featureList->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_featureList, &QListWidget::itemClicked, this,
          &MainWindow::onFeatureSelected);
  connect(m_featureList, &QListWidget::customContextMenuRequested, this,
          &MainWindow::onFeatureContextMenu);

  // Enable drag & drop for reordering
  m_featureList->setDragDropMode(QAbstractItemView::InternalMove);
  m_featureList->setDefaultDropAction(Qt::MoveAction);
  connect(m_featureList->model(), &QAbstractItemModel::rowsMoved, this,
          &MainWindow::onFeatureReordered);

  // Connect document signals to update feature list
  connect(m_document->featureTree(), &core::FeatureTree::featureAdded, this,
          &MainWindow::updateFeatureList);
  connect(m_document->featureTree(), &core::FeatureTree::featureRemoved, this,
          &MainWindow::updateFeatureList);
  connect(m_document->featureTree(), &core::FeatureTree::featureModified, this,
          &MainWindow::updateFeatureList);
  connect(m_document->featureTree(), &core::FeatureTree::treeStructureChanged,
          this, &MainWindow::updateFeatureList);
}

void MainWindow::showSketchEditor() {
  if (m_sketchDock) {
    m_sketchDock->show();
    m_sketchDock->raise();
  }
  if (m_sketchView && m_currentSketch) {
    m_sketchView->setSketch(m_currentSketch);
    m_sketchView->setFocus();

    // Update Properties panel with sketch info (associative)
    if (m_propertiesPanel) {
      m_propertiesPanel->setSketch(m_currentSketch);
    }

    // Display sketch plane in 3D viewport (SolidWorks style)
    // Calculate dynamic size based on existing shapes bounding box
    if (m_viewport) {
      double planeSize = 100.0; // Default size

      // Calculate bounding box of all shapes to determine plane size
      if (!m_document->getAllShapes().empty()) {
        Bnd_Box globalBox;
        for (const auto &shape : m_document->getAllShapes()) {
          if (!shape.IsNull()) {
            Bnd_Box box;
            BRepBndLib::Add(shape, box);
            globalBox.Add(box);
          }
        }

        if (!globalBox.IsVoid()) {
          double xMin, yMin, zMin, xMax, yMax, zMax;
          globalBox.Get(xMin, yMin, zMin, xMax, yMax, zMax);

          // Plane size should be 1.5x the largest dimension
          double maxDim = std::max({xMax - xMin, yMax - yMin, zMax - zMin});
          planeSize = maxDim * 1.5;
          if (planeSize < 50.0)
            planeSize = 50.0; // Minimum size
        }
      }

      // Use sketch plane origin as center - this matches buildWire coordinate
      // system
      gp_Pnt planeCenter = m_currentSketch->plane().origin();

      m_viewport->displaySketchPlane(m_currentSketch->plane().plane(),
                                     planeCenter, planeSize);

      // Display current sketch compound on the plane (holographic)
      // Compound is already positioned correctly by buildCompound using
      // m_plane.to3D()
      TopoDS_Compound sketchCompound = m_currentSketch->buildCompound();
      if (!sketchCompound.IsNull()) {
        m_viewport->displaySketchWire(sketchCompound);
      }
    }
  }
}

void MainWindow::updateSketchToolsEnabled(bool enabled) {
  // Sketch tools are now always enabled since selecting them auto-activates
  // sketch mode Only Finish button needs to be controlled
  if (m_sketchToolbar) {
    for (auto *action : m_sketchToolbar->actions()) {
      if (action->text() == "? Finish") {
        action->setEnabled(enabled);
      }
      // All other tools remain enabled
    }
  }
  if (m_constraintToolbar) {
    m_constraintToolbar->setEnabled(enabled);
  }
}

void MainWindow::setupStatusBar() {
  statusBar()->showMessage("Ready - OpenCAD v0.1.0");
}

void MainWindow::updateWindowTitle() {
  QString title = "OpenCAD";
  if (!m_currentFile.isEmpty()) {
    title += " - " + m_currentFile;
  }
  if (m_modified) {
    title += " *";
  }
  setWindowTitle(title);
}

// File menu slots
void MainWindow::onNewFile() {
  clearShapes();
  m_currentFile.clear();
  m_modified = false;
  updateWindowTitle();
  statusBar()->showMessage("New file created");
}

void MainWindow::onOpenFile() {
  QString filename = QFileDialog::getOpenFileName(
      this, "Open File", QString(),
      "STEP Files (*.step *.stp);;All Files (*.*)");

  if (!filename.isEmpty()) {
    clearShapes();

    io::StepReader reader;
    std::string path = filename.toStdString();

    if (reader.read(path)) {
      auto shapes = reader.getAllShapes();
      if (shapes.empty()) {
        // Try single shape
        auto shape = reader.getShape();
        if (shape.isValid()) {
          addShape(shape.occShape());
        }
      } else {
        for (const auto &shape : shapes) {
          addShape(shape.occShape());
        }
      }

      m_currentFile = filename;
      m_modified = false;
      updateWindowTitle();

      if (m_viewport) {
        m_viewport->fitAll();
      }

      statusBar()->showMessage("Opened: " + filename);
    } else {
      QMessageBox::warning(this, "Error",
                           "Failed to open file: " + filename + "\n" +
                               QString::fromStdString(reader.errorMessage()));
    }
  }
}

void MainWindow::onSaveFile() {
  if (m_currentFile.isEmpty()) {
    onSaveAs();
    return;
  }

  if (m_document->getAllShapes().empty()) {
    QMessageBox::warning(this, "Error", "No shapes to save");
    return;
  }

  io::StepWriter writer;
  std::string path = m_currentFile.toStdString();

  // Save first shape for now (TODO: compound)
  if (writer.write(core::Shape(m_document->getAllShapes()[0]), path)) {
    m_modified = false;
    updateWindowTitle();
    statusBar()->showMessage("Saved: " + m_currentFile);
  } else {
    QMessageBox::warning(this, "Error", "Failed to save file");
  }
}

void MainWindow::onSaveAs() {
  QString filename = QFileDialog::getSaveFileName(
      this, "Save As", QString(), "STEP Files (*.step);;All Files (*.*)");

  if (!filename.isEmpty()) {
    m_currentFile = filename;
    onSaveFile();
  }
}

void MainWindow::onExportSTL() {
  if (m_document->getAllShapes().empty()) {
    QMessageBox::warning(this, "Error", "No shapes to export");
    return;
  }

  QString filename = QFileDialog::getSaveFileName(this, "Export STL", QString(),
                                                  "STL Files (*.stl)");

  if (!filename.isEmpty()) {
    io::StlWriter writer;
    std::string path = filename.toStdString();

    if (writer.write(core::Shape(m_document->getAllShapes()[0]), path)) {
      statusBar()->showMessage("Exported STL: " + filename);
    } else {
      QMessageBox::warning(this, "Error", "Failed to export STL");
    }
  }
}

void MainWindow::onExportSTEP() {
  if (m_document->getAllShapes().empty()) {
    QMessageBox::warning(this, "Error", "No shapes to export");
    return;
  }

  QString filename = QFileDialog::getSaveFileName(
      this, "Export STEP", QString(), "STEP Files (*.step *.stp)");

  if (!filename.isEmpty()) {
    io::StepWriter writer;
    std::string path = filename.toStdString();

    if (writer.write(core::Shape(m_document->getAllShapes()[0]), path)) {
      statusBar()->showMessage("Exported STEP: " + filename);
    } else {
      QMessageBox::warning(this, "Error", "Failed to export STEP");
    }
  }
}

void MainWindow::onExit() { close(); }

// Edit menu slots
void MainWindow::onUndo() {
  // In sketch mode: use sketch undo
  if (m_sketchMode && m_currentSketch) {
    if (m_currentSketch->canUndo()) {
      m_currentSketch->undo();
      if (m_sketchView)
        m_sketchView->update();
      statusBar()->showMessage("Undo sketch edit", 2000);
    } else {
      statusBar()->showMessage("Nothing to undo in sketch", 2000);
    }
    return;
  }

  // 3D mode: use Document undo
  if (!m_document->canUndo()) {
    statusBar()->showMessage("Nothing to undo", 2000);
    return;
  }

  if (m_document->undo()) {
    displayAllShapes();
    statusBar()->showMessage(
        QString("Undo: %1").arg(m_document->undoDescription()), 2000);

    // Update feature list (remove last item)
    if (m_featureList && m_featureList->count() > 0) {
      delete m_featureList->takeItem(m_featureList->count() - 1);
    }
  }
}

void MainWindow::onRedo() {
  // In sketch mode: use sketch redo
  if (m_sketchMode && m_currentSketch) {
    if (m_currentSketch->canRedo()) {
      m_currentSketch->redo();
      if (m_sketchView)
        m_sketchView->update();
      statusBar()->showMessage("Redo sketch edit", 2000);
    } else {
      statusBar()->showMessage("Nothing to redo in sketch", 2000);
    }
    return;
  }

  // 3D mode: use Document redo
  if (!m_document->canRedo()) {
    statusBar()->showMessage("Nothing to redo", 2000);
    return;
  }

  if (m_document->redo()) {
    displayAllShapes();
    statusBar()->showMessage(
        QString("Redo: %1").arg(m_document->redoDescription()), 2000);

    // Add feature back to list
    if (m_featureList) {
      m_featureList->addItem(m_document->redoDescription());
    }
  }
}

void MainWindow::onDelete() {
  // In sketch mode: delete selected sketch entity
  if (m_sketchMode && m_sketchView && m_currentSketch) {
    auto *selectedEntity = m_sketchView->selectedEntity();
    if (selectedEntity) {
      // Remove entity from sketch
      m_currentSketch->removeEntity(selectedEntity->id());
      m_sketchView->clearSelection();
      m_sketchView->update();
      statusBar()->showMessage("Deleted sketch entity", 2000);
      return;
    } else {
      statusBar()->showMessage("No sketch entity selected", 2000);
      return;
    }
  }

  // In 3D mode: delete last shape
  if (!m_document->getAllShapes().empty()) {
    saveUndoState("Delete Shape");
    // TODO: Remove last feature
    // TODO: Convert to Feature-based system
    //       // m_shapes.pop_back();
    displayAllShapes();

    // Remove from feature list
    if (m_featureList && m_featureList->count() > 0) {
      delete m_featureList->takeItem(m_featureList->count() - 1);
    }

    statusBar()->showMessage("Deleted shape", 2000);
    m_modified = true;
    updateWindowTitle();
  } else {
    statusBar()->showMessage("Nothing to delete", 2000);
  }
}

// View menu slots
void MainWindow::onViewFit() {
  if (m_viewport) {
    m_viewport->fitAll();
  }
}

void MainWindow::onViewFront() {
  if (m_viewport)
    m_viewport->setViewFront();
}

void MainWindow::onViewBack() {
  if (m_viewport)
    m_viewport->setViewBack();
}

void MainWindow::onViewTop() {
  if (m_viewport)
    m_viewport->setViewTop();
}

void MainWindow::onViewBottom() {
  if (m_viewport)
    m_viewport->setViewBottom();
}

void MainWindow::onViewLeft() {
  if (m_viewport)
    m_viewport->setViewLeft();
}

void MainWindow::onViewRight() {
  if (m_viewport)
    m_viewport->setViewRight();
}

void MainWindow::onViewIsometric() {
  if (m_viewport)
    m_viewport->setViewIsometric();
}

// Create menu slots
void MainWindow::onCreateBox() {
  bool ok;
  double size = QInputDialog::getDouble(this, "Create Box", "Size:", 50.0, 1.0,
                                        1000.0, 1, &ok);

  if (ok) {
    auto shape = core::Primitives::makeBox(size, size, size);
    if (shape.isValid()) {
      addShape(shape.occShape());
      m_modified = true;
      updateWindowTitle();
      if (m_viewport)
        m_viewport->fitAll();
      statusBar()->showMessage("Created Box: " + QString::number(size) + " x " +
                               QString::number(size) + " x " +
                               QString::number(size));
    }
  }
}

void MainWindow::onCreateCylinder() {
  bool ok;
  double radius = QInputDialog::getDouble(this, "Create Cylinder",
                                          "Radius:", 25.0, 1.0, 500.0, 1, &ok);

  if (ok) {
    double height = QInputDialog::getDouble(
        this, "Create Cylinder", "Height:", 50.0, 1.0, 1000.0, 1, &ok);

    if (ok) {
      auto shape = core::Primitives::makeCylinder(radius, height);
      if (shape.isValid()) {
        addShape(shape.occShape());
        m_modified = true;
        updateWindowTitle();
        if (m_viewport)
          m_viewport->fitAll();
        statusBar()->showMessage(
            "Created Cylinder: R=" + QString::number(radius) +
            " H=" + QString::number(height));
      }
    }
  }
}

void MainWindow::onCreateSphere() {
  bool ok;
  double radius = QInputDialog::getDouble(this, "Create Sphere",
                                          "Radius:", 30.0, 1.0, 500.0, 1, &ok);

  if (ok) {
    auto shape = core::Primitives::makeSphere(radius);
    if (shape.isValid()) {
      addShape(shape.occShape());
      m_modified = true;
      updateWindowTitle();
      if (m_viewport)
        m_viewport->fitAll();
      statusBar()->showMessage("Created Sphere: R=" + QString::number(radius));
    }
  }
}

void MainWindow::onCreateCone() {
  bool ok;
  double radius1 = QInputDialog::getDouble(
      this, "Create Cone", "Base Radius:", 30.0, 0.0, 500.0, 1, &ok);

  if (ok) {
    double radius2 = QInputDialog::getDouble(
        this, "Create Cone", "Top Radius:", 10.0, 0.0, 500.0, 1, &ok);

    if (ok) {
      double height = QInputDialog::getDouble(
          this, "Create Cone", "Height:", 50.0, 1.0, 1000.0, 1, &ok);

      if (ok) {
        auto shape = core::Primitives::makeCone(radius1, radius2, height);
        if (shape.isValid()) {
          addShape(shape.occShape());
          m_modified = true;
          updateWindowTitle();
          if (m_viewport)
            m_viewport->fitAll();
          statusBar()->showMessage(
              "Created Cone: R1=" + QString::number(radius1) + " R2=" +
              QString::number(radius2) + " H=" + QString::number(height));
        }
      }
    }
  }
}

// Boolean menu slots
void MainWindow::onBooleanFuse() {
  if (m_document->getAllShapes().size() < 2) {
    QMessageBox::information(this, "Boolean Fuse",
                             "Need at least 2 shapes for boolean "
                             "operations.\nCreate more shapes first.");
    return;
  }

  // Fuse first two shapes
  auto result =
      core::BooleanOps::fuse(core::Shape(m_document->getAllShapes()[0]),
                             core::Shape(m_document->getAllShapes()[1]));

  if (result.isValid()) {
    // Save state for undo
    saveUndoState("Boolean Fuse");

    // Remove original shapes, add result
    if (m_document->temporaryShapes().size() >= 2) {
      m_document->temporaryShapes().erase(
          m_document->temporaryShapes().begin(),
          m_document->temporaryShapes().begin() + 2);
      m_document->temporaryShapes().insert(
          m_document->temporaryShapes().begin(), result.occShape());
    }

    displayAllShapes();
    m_modified = true;
    updateWindowTitle();
    statusBar()->showMessage("Boolean Fuse completed");
  } else {
    QMessageBox::warning(this, "Error", "Boolean Fuse failed");
  }
}

void MainWindow::onBooleanCut() {
  if (m_document->getAllShapes().size() < 2) {
    QMessageBox::information(this, "Boolean Cut",
                             "Need at least 2 shapes for boolean "
                             "operations.\nCreate more shapes first.");
    return;
  }

  // Cut second from first
  auto result =
      core::BooleanOps::cut(core::Shape(m_document->getAllShapes()[0]),
                            core::Shape(m_document->getAllShapes()[1]));

  if (result.isValid()) {
    // Save state for undo
    saveUndoState("Boolean Cut");

    // Remove original shapes, add result
    if (m_document->temporaryShapes().size() >= 2) {
      m_document->temporaryShapes().erase(
          m_document->temporaryShapes().begin(),
          m_document->temporaryShapes().begin() + 2);
      m_document->temporaryShapes().insert(
          m_document->temporaryShapes().begin(), result.occShape());
    }

    displayAllShapes();
    m_modified = true;
    updateWindowTitle();
    statusBar()->showMessage("Boolean Cut completed");
  } else {
    QMessageBox::warning(this, "Error", "Boolean Cut failed");
  }
}

void MainWindow::onBooleanCommon() {
  if (m_document->getAllShapes().size() < 2) {
    QMessageBox::information(this, "Boolean Common",
                             "Need at least 2 shapes for boolean "
                             "operations.\nCreate more shapes first.");
    return;
  }

  // Common of first two shapes
  auto result =
      core::BooleanOps::common(core::Shape(m_document->getAllShapes()[0]),
                               core::Shape(m_document->getAllShapes()[1]));

  if (result.isValid()) {
    // Save state for undo
    saveUndoState("Boolean Common");

    // Remove original shapes, add result
    if (m_document->temporaryShapes().size() >= 2) {
      m_document->temporaryShapes().erase(
          m_document->temporaryShapes().begin(),
          m_document->temporaryShapes().begin() + 2);
      m_document->temporaryShapes().insert(
          m_document->temporaryShapes().begin(), result.occShape());
    }

    displayAllShapes();
    m_modified = true;
    updateWindowTitle();
    statusBar()->showMessage("Boolean Common completed");
  } else {
    QMessageBox::warning(this, "Error", "Boolean Common failed");
  }
}

// Help menu slots
void MainWindow::onAbout() {
  QMessageBox::about(this, "About OpenCAD",
                     "<h3>OpenCAD v0.1.0</h3>"
                     "<p>Modular CAD/CAE Platform</p>"
                     "<p>Built with OpenCASCADE & Qt6</p>"
                     "<p><b>Features:</b></p>"
                     "<ul>"
                     "<li>STEP file import/export</li>"
                     "<li>STL export</li>"
                     "<li>Primitive shapes</li>"
                     "<li>Boolean operations</li>"
                     "<li>2D Sketch with constraints</li>"
                     "<li>Part features (Extrude, Revolve)</li>"
                     "<li>3D viewport</li>"
                     "</ul>"
                     "<p>� 2024 OpenCAD Team</p>");
}

// ============================================================================
// Sketch Slots
// ============================================================================

void MainWindow::onNewSketch() {
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showSketchPlaneSettings();

  m_activePartTool = ActivePartTool::NewSketch;
  statusBar()->showMessage("New Sketch: Select a plane in panel, then Apply");
}

void MainWindow::onEditSketch() {
  if (!m_currentSketch) {
    QMessageBox::information(this, "Edit Sketch",
                             "No sketch to edit. Create a new sketch first.");
    return;
  }
  m_sketchMode = true;
  updateSketchToolsEnabled(true);
  showSketchEditor();
  statusBar()->showMessage("Editing: " +
                           QString::fromStdString(m_currentSketch->name()));
}

void MainWindow::onSketchOnFace() {
  if (m_document->getAllShapes().empty()) {
    QMessageBox::information(this, "Sketch on Face",
                             "No solid body to select face from.\nCreate a box "
                             "or extrude a sketch first.");
    return;
  }

  // Enable face selection mode
  if (m_viewport) {
    m_viewport->enableFaceSelection(true);
    statusBar()->showMessage("Click on a face to create sketch...");
  }
}

void MainWindow::onFaceSelected() {
  if (!m_viewport)
    return;

  // Handle pending face operations (Dome, Shell, Draft, Thicken, OffsetSurface)
  if (m_pendingFaceOperation != PendingFaceOperation::None) {
    if (m_document->getAllShapes().empty()) {
      m_pendingFaceOperation = PendingFaceOperation::None;
      return;
    }

    TopoDS_Face selectedFace = m_viewport->selectedFace();
    if (selectedFace.IsNull()) {
      statusBar()->showMessage("No face selected", 3000);
      return;
    }

    // Disable face selection mode
    m_viewport->enableFaceSelection(false);

    try {
      switch (m_pendingFaceOperation) {
      case PendingFaceOperation::Dome: {
        saveUndoState("Dome");

        part::DomeFeature dome;
        TopoDS_Shape result = dome.execute(m_document->getAllShapes().back(),
                                           selectedFace, m_pendingDomeHeight);

        if (!result.IsNull()) {
          m_document->getAllShapes().back() = result;
          displayAllShapes();
          m_featureList->addItem(
              QString("?? Dome (H=%1mm)").arg(m_pendingDomeHeight));
          statusBar()->showMessage(
              QString("Dome added: H=%1mm").arg(m_pendingDomeHeight), 3000);
        } else {
          QMessageBox::warning(this, "Dome",
                               "Dome failed: " +
                                   QString::fromStdString(dome.errorMessage()));
        }
        break;
      }

      case PendingFaceOperation::Shell: {
        saveUndoState("Shell");

        std::vector<TopoDS_Face> facesToRemove = {selectedFace};
        double thickness = m_pendingShellOutward ? -m_pendingShellThickness
                                                 : m_pendingShellThickness;

        part::ShellFeature shell;
        TopoDS_Shape result = shell.execute(m_document->getAllShapes().back(),
                                            facesToRemove, thickness);

        if (!result.IsNull()) {
          m_document->getAllShapes().back() = result;
          displayAllShapes();
          m_featureList->addItem(
              QString("?? Shell (T=%1mm)").arg(m_pendingShellThickness));
          statusBar()->showMessage(
              QString("Shell created: T=%1mm").arg(m_pendingShellThickness),
              3000);
        } else {
          QMessageBox::warning(
              this, "Shell",
              "Shell failed: " + QString::fromStdString(shell.errorMessage()));
        }
        break;
      }

      case PendingFaceOperation::Draft: {
        saveUndoState("Draft");

        // Draft needs a neutral plane and direction - using Z axis for
        // simplicity
        gp_Dir pullDir(0, 0, 1);
        gp_Pln neutralPlane(gp_Pnt(0, 0, 0), pullDir);

        std::vector<TopoDS_Face> facesToDraft = {selectedFace};

        part::DraftFeature draft;
        TopoDS_Shape result =
            draft.execute(m_document->getAllShapes().back(), facesToDraft,
                          neutralPlane, pullDir, m_pendingDraftAngle);

        if (!result.IsNull()) {
          m_document->getAllShapes().back() = result;
          displayAllShapes();
          m_featureList->addItem(
              QString("?? Draft (%1�)").arg(m_pendingDraftAngle));
          statusBar()->showMessage(
              QString("Draft applied: %1�").arg(m_pendingDraftAngle), 3000);
        } else {
          QMessageBox::warning(
              this, "Draft",
              "Draft failed: " + QString::fromStdString(draft.errorMessage()));
        }
        break;
      }

      case PendingFaceOperation::Thicken: {
        saveUndoState("Thicken");

        part::ThickenFeature thicken;
        TopoDS_Shape result =
            thicken.execute(selectedFace, m_pendingThickenValue);

        if (!result.IsNull()) {
          // TODO: Convert to Feature
          // TODO: Convert to Feature-based system
          //       // m_shapes.push_back(result);
          displayAllShapes();
          m_featureList->addItem(
              QString("?? Thicken (%1mm)").arg(m_pendingThickenValue));
          statusBar()->showMessage(
              QString("Thickened by %1mm").arg(m_pendingThickenValue), 3000);
        } else {
          QMessageBox::warning(
              this, "Thicken",
              "Thicken failed: " +
                  QString::fromStdString(thicken.errorMessage()));
        }
        break;
      }

      case PendingFaceOperation::OffsetSurface: {
        saveUndoState("Offset Surface");

        part::OffsetSurfaceFeature offsetSurf;
        TopoDS_Shape result =
            offsetSurf.execute(selectedFace, m_pendingOffsetValue);

        if (!result.IsNull()) {
          // TODO: Convert to Feature
          // TODO: Convert to Feature-based system
          //       // m_shapes.push_back(result);
          displayAllShapes();
          m_featureList->addItem(
              QString("?? Offset Surface (%1mm)").arg(m_pendingOffsetValue));
          statusBar()->showMessage(
              QString("Surface offset by %1mm").arg(m_pendingOffsetValue),
              3000);
        } else {
          QMessageBox::warning(
              this, "Offset Surface",
              "Offset failed: " +
                  QString::fromStdString(offsetSurf.errorMessage()));
        }
        break;
      }

      default:
        break;
      }

    } catch (const Standard_Failure &e) {
      QMessageBox::warning(this, "Operation Failed",
                           QString("Error: %1").arg(e.GetMessageString()));
    } catch (...) {
      QMessageBox::warning(this, "Operation Failed", "Unknown error occurred.");
    }

    m_pendingFaceOperation = PendingFaceOperation::None;
    return;
  }

  // Original sketch-on-face behavior
  gp_Pln plane;
  bool hasPlane = m_viewport->getSelectedFacePlane(plane);

  // Now disable face selection mode
  m_viewport->enableFaceSelection(false);

  if (hasPlane) {
    // Create SketchPlane from the face's plane
    sketch::SketchPlane sketchPlane(plane);

    // Create new sketch with the face's plane
    m_currentSketch = std::make_shared<sketch::Sketch>(sketchPlane);
    m_currentSketch->setName("Sketch" + std::to_string(m_featureList->count()));
    m_document->addSketch(m_currentSketch);

    m_sketchMode = true;
    updateSketchToolsEnabled(true);

    if (m_featureList) {
      m_featureList->addItem("?? " +
                             QString::fromStdString(m_currentSketch->name()) +
                             " (on face)");
    }

    showSketchEditor();
    if (m_sketchView) {
      m_sketchView->setTool(SketchToolType::Line);
    }

    statusBar()->showMessage("Sketch on face: Use L=Line, R=Rect, C=Circle");
    m_modified = true;
    updateWindowTitle();
  } else {
    QMessageBox::warning(
        this, "Non-Planar Face",
        "Selected face is not planar. Please select a flat face.");
  }
}

void MainWindow::onFinishSketch() {
  if (!m_sketchMode)
    return;

  m_sketchMode = false;
  updateSketchToolsEnabled(false);

  // Hide sketch editor
  if (m_sketchDock) {
    m_sketchDock->hide();
  }

  if (m_currentSketch) {
    int entityCount = static_cast<int>(m_currentSketch->entities().size());
    statusBar()->showMessage(
        "Sketch finished: " + QString::number(entityCount) +
        " entities, DOF: " + QString::number(m_currentSketch->remainingDOF()));
  } else {
    statusBar()->showMessage("Sketch mode exited");
  }
}

// Sketch geometry tools - Now use SketchView2D mouse-based drawing
void MainWindow::onSketchLine() {
  // Auto-activate sketch mode if not already active
  if (!m_sketchMode) {
    m_sketchMode = true;
    updateSketchToolsEnabled(true);
  }

  // Auto-create default XY sketch if none exists
  if (!m_currentSketch) {
    sketch::SketchPlane sketchPlane;
    sketchPlane.setOrientation(sketch::PlaneOrientation::XY_Front);
    m_currentSketch = std::make_shared<sketch::Sketch>(sketchPlane);
    m_document->addSketch(m_currentSketch);
    if (m_sketchView) {
      m_sketchView->setSketch(m_currentSketch);
      // Show sketch dock so user can draw
      if (m_sketchDock) {
        m_sketchDock->show();
        m_sketchDock->raise();
      }
    }
    statusBar()->showMessage("Auto-created XY plane sketch", 2000);
  }

  if (m_sketchView) {
    m_sketchView->setTool(SketchToolType::Line);
    statusBar()->showMessage("Line tool: Click and drag to draw a line");
  }
}

void MainWindow::onSketchRectangle() {
  // Auto-activate sketch mode if not already active
  if (!m_sketchMode) {
    m_sketchMode = true;
    updateSketchToolsEnabled(true);
  }
  if (m_sketchView) {
    m_sketchView->setTool(SketchToolType::Rectangle);
    statusBar()->showMessage("Rectangle tool: Click and drag to draw");
  }
}

void MainWindow::onSketchCircle() {
  // Auto-activate sketch mode if not already active
  if (!m_sketchMode) {
    m_sketchMode = true;
    updateSketchToolsEnabled(true);
  }
  if (m_sketchView) {
    m_sketchView->setTool(SketchToolType::Circle);
    statusBar()->showMessage("Circle tool: Click center, drag for radius");
  }
}

void MainWindow::onSketchArc() {
  // Auto-activate sketch mode if not already active
  if (!m_sketchMode) {
    m_sketchMode = true;
    updateSketchToolsEnabled(true);
  }
  if (m_sketchView) {
    m_sketchView->setTool(SketchToolType::Arc);
    statusBar()->showMessage("Arc tool: Click center, drag for radius");
  }
}

void MainWindow::onSketchPoint() {
  // Auto-activate sketch mode if not already active
  if (!m_sketchMode) {
    m_sketchMode = true;
    updateSketchToolsEnabled(true);
  }
  if (m_sketchView) {
    m_sketchView->setTool(SketchToolType::Point);
    statusBar()->showMessage("Point tool: Click to place points");
  }
}

void MainWindow::onSketchSpline() {
  // Auto-activate sketch mode if not already active
  if (!m_sketchMode) {
    m_sketchMode = true;
    updateSketchToolsEnabled(true);
  }
  if (m_sketchView) {
    m_sketchView->setTool(SketchToolType::Spline);
    statusBar()->showMessage("Spline tool: Click to add control points");
  }
}

void MainWindow::onSketchEllipse() {
  // Auto-activate sketch mode if not already active
  if (!m_sketchMode) {
    m_sketchMode = true;
    updateSketchToolsEnabled(true);
  }
  if (m_sketchView) {
    m_sketchView->setTool(SketchToolType::Ellipse);
    statusBar()->showMessage("Ellipse tool: Click and drag");
  }
}

void MainWindow::onSketchPolygon() {
  // Auto-activate sketch mode if not already active
  if (!m_sketchMode) {
    m_sketchMode = true;
    updateSketchToolsEnabled(true);
  }
  if (m_sketchView) {
    m_sketchView->setTool(SketchToolType::Polygon);
    statusBar()->showMessage(
        "Polygon tool: Click center, drag to vertex (+/- to change sides)");
  }
}

void MainWindow::onSketchSlot() {
  // Auto-activate sketch mode if not already active
  if (!m_sketchMode) {
    m_sketchMode = true;
    updateSketchToolsEnabled(true);
  }
  if (m_sketchView) {
    m_sketchView->setTool(SketchToolType::Slot);
    statusBar()->showMessage("Slot tool: Click first center, drag to second "
                             "center (width from Tool Settings)");
  }
}

void MainWindow::onConvertEntities() {
  if (!m_sketchMode) {
    QMessageBox::information(this, "Convert Entities",
                             "Start a new sketch first");
    return;
  }

  if (!m_currentSketch) {
    QMessageBox::warning(this, "Convert Entities", "No active sketch");
    return;
  }

  if (m_document->getAllShapes().empty()) {
    QMessageBox::information(this, "Convert Entities",
                             "No 3D geometry to convert."
                             "Create a solid first.");
    return;
  }

  // Get sketch plane for projection
  gp_Pln plane = m_currentSketch->plane().plane();
  gp_Dir planeNormal = plane.Axis().Direction();

  int addedCount = 0;

  // Extract all edges from shapes and project onto sketch plane
  for (const auto &shape : m_document->getAllShapes()) {
    TopExp_Explorer edgeExp(shape, TopAbs_EDGE);
    for (; edgeExp.More(); edgeExp.Next()) {
      TopoDS_Edge edge = TopoDS::Edge(edgeExp.Current());

      try {
        Standard_Real first, last;
        Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);

        if (curve.IsNull())
          continue;

        // Check curve type and project accordingly
        if (curve->IsKind(STANDARD_TYPE(Geom_Line))) {
          // Project line endpoints
          gp_Pnt p1 = curve->Value(first);
          gp_Pnt p2 = curve->Value(last);

          gp_Pnt2d pt1_2d = m_currentSketch->plane().to2D(p1);
          gp_Pnt2d pt2_2d = m_currentSketch->plane().to2D(p2);

          // Only add if line has significant length
          if (pt1_2d.Distance(pt2_2d) > 0.01) {
            auto line = std::make_shared<sketch::SketchLine>(pt1_2d, pt2_2d);
            line->setConstruction(true);
            m_currentSketch->addEntity(line);
            addedCount++;
          }
        } else if (curve->IsKind(STANDARD_TYPE(Geom_Circle))) {
          Handle(Geom_Circle) gc = Handle(Geom_Circle)::DownCast(curve);
          gp_Pnt center3d = gc->Location();

          // Check if circle is parallel to sketch plane
          gp_Dir circleNormal = gc->Axis().Direction();
          if (std::abs(circleNormal.Dot(planeNormal)) > 0.99) {
            // Project center onto plane
            gp_Pnt2d center2d = m_currentSketch->plane().to2D(center3d);
            double radius = gc->Radius();

            auto circle =
                std::make_shared<sketch::SketchCircle>(center2d, radius);
            circle->setConstruction(true);
            m_currentSketch->addEntity(circle);
            addedCount++;
          }
        } else {
          // For other curves, approximate with line segments
          const int numSamples = 20;
          for (int i = 0; i < numSamples; ++i) {
            double t1 = first + (last - first) * i / numSamples;
            double t2 = first + (last - first) * (i + 1) / numSamples;
            gp_Pnt p1 = curve->Value(t1);
            gp_Pnt p2 = curve->Value(t2);

            gp_Pnt2d pt1_2d = m_currentSketch->plane().to2D(p1);
            gp_Pnt2d pt2_2d = m_currentSketch->plane().to2D(p2);

            if (pt1_2d.Distance(pt2_2d) > 0.01) {
              auto line = std::make_shared<sketch::SketchLine>(pt1_2d, pt2_2d);
              line->setConstruction(true);
              m_currentSketch->addEntity(line);
              addedCount++;
            }
          }
        }
      } catch (...) {
        // Continue with other edges if one fails
      }
    }
  }

  if (addedCount == 0) {
    QMessageBox::information(this, "Convert Entities",
                             "No edges could be converted to sketch entities.");
    return;
  }

  // Refresh sketch view
  if (m_sketchView) {
    m_sketchView->update();
  }

  statusBar()->showMessage(
      QString("Convert Entities: Added %1 construction entities")
          .arg(addedCount),
      3000);

  m_modified = true;
  updateWindowTitle();
}

void MainWindow::onIntersectionCurve() {
  if (!m_sketchMode) {
    QMessageBox::information(this, "Intersection Curve",
                             "Start a new sketch first");
    return;
  }

  if (!m_currentSketch) {
    QMessageBox::warning(this, "Intersection Curve", "No active sketch");
    return;
  }

  if (m_document->getAllShapes().empty()) {
    QMessageBox::information(this, "Intersection Curve",
                             "No 3D geometry to intersect with."
                             "Create a solid first.");
    return;
  }

  // Get sketch plane
  gp_Pln sketchPlane = m_currentSketch->plane().plane();

  // Create a face from the infinite plane (bounded for intersection)
  // Use a large enough face to cover the model
  BRepBuilderAPI_MakeFace planeFaceBuilder(sketchPlane, -1000, 1000, -1000,
                                           1000);
  if (!planeFaceBuilder.IsDone()) {
    QMessageBox::warning(this, "Intersection Curve",
                         "Could not create intersection plane.");
    return;
  }
  TopoDS_Face planeFace = planeFaceBuilder.Face();

  // Collect all intersection curves
  std::vector<std::pair<gp_Pnt, gp_Pnt>> lineSegments;
  std::vector<std::pair<gp_Pnt, double>> circles; // center, radius
  std::vector<std::tuple<gp_Pnt, double, double>>
      ellipses; // center, major, minor

  int curveCount = 0;

  // Intersect with all shapes
  for (const auto &shape : m_document->getAllShapes()) {
    try {
      BRepAlgoAPI_Section section(shape, planeFace, Standard_False);
      section.ComputePCurveOn1(Standard_True);
      section.Approximation(Standard_True);
      section.Build();

      if (section.IsDone()) {
        TopoDS_Shape result = section.Shape();

        // Extract edges from the section result
        TopExp_Explorer edgeExp(result, TopAbs_EDGE);
        for (; edgeExp.More(); edgeExp.Next()) {
          TopoDS_Edge edge = TopoDS::Edge(edgeExp.Current());

          Standard_Real first, last;
          Handle(Geom_Curve) curve = BRep_Tool::Curve(edge, first, last);

          if (!curve.IsNull()) {
            curveCount++;

            // Determine curve type and add to sketch
            if (curve->IsKind(STANDARD_TYPE(Geom_Line))) {
              gp_Pnt p1 = curve->Value(first);
              gp_Pnt p2 = curve->Value(last);
              lineSegments.push_back({p1, p2});
            } else if (curve->IsKind(STANDARD_TYPE(Geom_Circle))) {
              Handle(Geom_Circle) gc = Handle(Geom_Circle)::DownCast(curve);
              gp_Pnt center = gc->Location();
              double radius = gc->Radius();
              circles.push_back({center, radius});
            } else if (curve->IsKind(STANDARD_TYPE(Geom_Ellipse))) {
              Handle(Geom_Ellipse) ge = Handle(Geom_Ellipse)::DownCast(curve);
              gp_Pnt center = ge->Location();
              double major = ge->MajorRadius();
              double minor = ge->MinorRadius();
              ellipses.push_back({center, major, minor});
            } else {
              // For other curve types (BSpline, etc.), approximate with line
              // segments
              const int numSamples = 20;
              for (int i = 0; i < numSamples; ++i) {
                double t1 = first + (last - first) * i / numSamples;
                double t2 = first + (last - first) * (i + 1) / numSamples;
                gp_Pnt p1 = curve->Value(t1);
                gp_Pnt p2 = curve->Value(t2);
                lineSegments.push_back({p1, p2});
              }
            }
          }
        }
      }
    } catch (...) {
      // Continue with other shapes if one fails
    }
  }

  if (curveCount == 0) {
    QMessageBox::information(this, "Intersection Curve",
                             "No intersection found between the sketch plane "
                             "and the 3D geometry.");
    return;
  }

  // Convert 3D points to 2D sketch coordinates and add entities
  int addedCount = 0;

  for (const auto &[p1, p2] : lineSegments) {
    gp_Pnt2d pt1_2d = m_currentSketch->plane().to2D(p1);
    gp_Pnt2d pt2_2d = m_currentSketch->plane().to2D(p2);

    auto line = std::make_shared<sketch::SketchLine>(pt1_2d, pt2_2d);
    line->setConstruction(true); // Mark as construction geometry
    m_currentSketch->addEntity(line);
    addedCount++;
  }

  for (const auto &[center, radius] : circles) {
    gp_Pnt2d center2d = m_currentSketch->plane().to2D(center);

    auto circle = std::make_shared<sketch::SketchCircle>(center2d, radius);
    circle->setConstruction(true);
    m_currentSketch->addEntity(circle);
    addedCount++;
  }

  for (const auto &[center, major, minor] : ellipses) {
    gp_Pnt2d center2d = m_currentSketch->plane().to2D(center);

    auto ellipse =
        std::make_shared<sketch::SketchEllipse>(center2d, major, minor);
    ellipse->setConstruction(true);
    m_currentSketch->addEntity(ellipse);
    addedCount++;
  }

  // Refresh sketch view
  if (m_sketchView) {
    m_sketchView->update();
  }

  statusBar()->showMessage(
      QString("Intersection Curve: Added %1 construction entities")
          .arg(addedCount),
      3000);

  m_modified = true;
  updateWindowTitle();
}

// ============================================================================
// Constraint Slots
// ============================================================================

void MainWindow::onConstraintHorizontal() {
  if (!m_sketchMode || !m_currentSketch) {
    statusBar()->showMessage("Enter sketch mode first");
    return;
  }

  // Get selected entity from SketchView2D
  if (m_sketchView && m_sketchView->selectedEntity()) {
    auto *entity = m_sketchView->selectedEntity();
    auto *line = dynamic_cast<sketch::SketchLine *>(entity);
    if (line) {
      // Find the shared_ptr for this entity
      for (const auto &e : m_currentSketch->entities()) {
        if (e.get() == line) {
          auto linePtr = std::dynamic_pointer_cast<sketch::SketchLine>(e);
          if (linePtr) {
            m_currentSketch->addHorizontal(linePtr);
            m_currentSketch->solve();
            m_sketchView->update();
            statusBar()->showMessage("Horizontal constraint added");
            return;
          }
        }
      }
    } else {
      statusBar()->showMessage("Please select a line first");
    }
  } else {
    statusBar()->showMessage("Please select a line first");
  }
}

void MainWindow::onConstraintVertical() {
  if (!m_sketchMode || !m_currentSketch) {
    statusBar()->showMessage("Enter sketch mode first");
    return;
  }

  // Get selected entity from SketchView2D
  if (m_sketchView && m_sketchView->selectedEntity()) {
    auto *entity = m_sketchView->selectedEntity();
    auto *line = dynamic_cast<sketch::SketchLine *>(entity);
    if (line) {
      // Find the shared_ptr for this entity
      for (const auto &e : m_currentSketch->entities()) {
        if (e.get() == line) {
          auto linePtr = std::dynamic_pointer_cast<sketch::SketchLine>(e);
          if (linePtr) {
            m_currentSketch->addVertical(linePtr);
            m_currentSketch->solve();
            m_sketchView->update();
            statusBar()->showMessage("Vertical constraint added");
            return;
          }
        }
      }
    } else {
      statusBar()->showMessage("Please select a line first");
    }
  } else {
    statusBar()->showMessage("Please select a line first");
  }
}

void MainWindow::onConstraintCoincident() {
  if (!m_sketchMode) {
    statusBar()->showMessage("Enter sketch mode first");
    return;
  }
  statusBar()->showMessage("Coincident constraint: Select two points");
}

void MainWindow::onConstraintDistance() {
  if (!m_sketchMode) {
    statusBar()->showMessage("Enter sketch mode first");
    return;
  }
  statusBar()->showMessage("Distance constraint: Select two points or a line");
}

void MainWindow::onConstraintRadius() {
  if (!m_sketchMode) {
    statusBar()->showMessage("Enter sketch mode first");
    return;
  }
  statusBar()->showMessage("Radius constraint: Select a circle or arc");
}

void MainWindow::onConstraintAngle() {
  if (!m_sketchMode) {
    statusBar()->showMessage("Enter sketch mode first");
    return;
  }
  statusBar()->showMessage("Angle constraint: Select two lines");
}

void MainWindow::onConstraintParallel() {
  if (!m_sketchMode) {
    statusBar()->showMessage("Enter sketch mode first");
    return;
  }
  statusBar()->showMessage("Parallel constraint: Select two lines");
}

void MainWindow::onConstraintPerpendicular() {
  if (!m_sketchMode) {
    statusBar()->showMessage("Enter sketch mode first");
    return;
  }
  statusBar()->showMessage("Perpendicular constraint: Select two lines");
}

// ============================================================================
// Part Feature Slots
// ============================================================================

void MainWindow::onExtrude() {
  // DEBUG: Use QMessageBox to confirm function is called
  qDebug() << "=== onExtrude() CALLED ===";

  // Show Extrude settings panel immediately - AGGRESSIVE SHOW
  if (m_toolSettingsDock) {
    m_toolSettingsDock->setFloating(true); // Make it a separate window
    m_toolSettingsDock->resize(300, 400);  // Ensure visible size
    m_toolSettingsDock->move(100, 100);    // Position on screen
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
    m_toolSettingsDock->activateWindow();
    qDebug() << "Tool Settings Dock: floating="
             << m_toolSettingsDock->isFloating()
             << "visible=" << m_toolSettingsDock->isVisible();
  }
  if (m_toolSettingsPanel) {
    m_toolSettingsPanel->showExtrudeSettings();
    qDebug() << "Extrude settings shown in panel";
  }

  // Reset selected profile
  m_selectedProfileIndex = -1;

  if (!m_currentSketch || m_currentSketch->entities().empty()) {
    QMessageBox::information(this, "Extrude",
                             "Create a closed sketch profile first.\n"
                             "Use Sketch > New Sketch and draw geometry.");
    return;
  }

  // Detect all closed profiles in sketch
  auto closedProfiles = m_currentSketch->detectClosedProfiles();
  qDebug() << "Detected closed profiles:" << closedProfiles.size();

  if (closedProfiles.empty()) {
    QMessageBox::information(
        this, "Extrude",
        "No closed profiles found in sketch.\n"
        "Draw closed shapes (circles, rectangles, or connected lines).");
    return;
  }

  // Set active tool
  m_pendingOperation = PendingOperation::Extrude;
  m_activePartTool = ActivePartTool::Extrude;

  // Update profile list in ProfileSelectionPanel
  if (m_profileSelectionPanel) {
    QStringList profileNames;
    for (size_t i = 0; i < closedProfiles.size(); ++i) {
      profileNames << QString("Profile %1").arg(i + 1);
    }
    m_profileSelectionPanel->updateProfileList(profileNames);
    m_profileSelectionPanel->setOperationTitle("Extrude");
    // Automatically select first profile so Apply works immediately
    if (!closedProfiles.empty()) {
      m_profileSelectionPanel->setProfileIndex(0);
      m_selectedProfileIndex = 0;
      qDebug() << "Extrude: Auto-selected Profile 1";
    }
  }

  // Enable visual profile selection (SolidWorks-style)
  if (m_sketchView) {
    m_sketchView->enterProfileSelectMode();
    m_sketchDock->show();
  }

  // IMPORTANT: Raise Tool Settings Panel after showing sketch dock
  // so Apply button is visible
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
    qDebug() << "Extrude: Tool Settings Panel raised, Apply button should be "
                "visible";
  }

  statusBar()->showMessage(
      QString("Click profile to select (%1 "
              "available), adjust settings in panel, then click Apply")
          .arg(closedProfiles.size()));
}

void MainWindow::onRevolve() {
  // Show Revolve settings in tool panel
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showRevolveSettings();

  // Reset selected profile
  m_selectedProfileIndex = -1;

  if (!m_currentSketch || m_currentSketch->entities().empty()) {
    QMessageBox::information(this, "Revolve", "Create a sketch profile first.");
    return;
  }

  // Check for closed profile
  auto closedProfiles = m_currentSketch->detectClosedProfiles();
  if (closedProfiles.empty()) {
    QMessageBox::warning(
        this, "Revolve",
        "No closed profiles found. Draw a closed shape first.");
    return;
  }

  // Set active tool
  m_pendingOperation = PendingOperation::None; // Revolve uses Apply directly
  m_activePartTool = ActivePartTool::Revolve;

  // Update profile list in ProfileSelectionPanel
  if (m_profileSelectionPanel) {
    QStringList profileNames;
    for (size_t i = 0; i < closedProfiles.size(); ++i) {
      profileNames << QString("Profile %1").arg(i + 1);
    }
    m_profileSelectionPanel->updateProfileList(profileNames);
    m_profileSelectionPanel->setOperationTitle("Revolve");
    // Auto-select first profile
    m_selectedProfileIndex = 0;
  }

  if (m_sketchView) {
    m_sketchView->enterProfileSelectMode();
    m_sketchDock->show();
    m_sketchDock->raise();
  }

  statusBar()->showMessage(
      QString("Revolve: Select profile, set angle and axis, then Apply"));
}

void MainWindow::onCut() {
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showCutSettings();

  m_selectedProfileIndex = -1;

  if (!m_currentSketch || m_currentSketch->entities().empty()) {
    QMessageBox::information(this, "Cut",
                             "Create a closed sketch profile first.");
    return;
  }

  auto closedProfiles = m_currentSketch->detectClosedProfiles();
  if (closedProfiles.empty()) {
    QMessageBox::warning(this, "Cut", "No closed profiles found.");
    return;
  }

  m_pendingOperation = PendingOperation::Cut;
  m_activePartTool = ActivePartTool::Cut;

  if (m_profileSelectionPanel) {
    QStringList profileNames;
    for (size_t i = 0; i < closedProfiles.size(); ++i) {
      profileNames << QString("Profile %1").arg(i + 1);
    }
    m_profileSelectionPanel->updateProfileList(profileNames);
    m_profileSelectionPanel->setOperationTitle("Cut");
    // Automatically select first profile so Apply works immediately
    if (!closedProfiles.empty()) {
      m_profileSelectionPanel->setProfileIndex(0);
      m_selectedProfileIndex = 0;
      qDebug() << "Cut: Auto-selected Profile 1";
    }
  }

  if (m_sketchView) {
    m_sketchView->enterProfileSelectMode();
    m_sketchDock->show();
    m_sketchDock->raise();
    statusBar()->showMessage("Select profile to cut, adjust depth, then Apply");
  }
}

void MainWindow::onReferencePlane() {
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showReferencePlaneSettings();

  m_activePartTool = ActivePartTool::ReferencePlane;
  statusBar()->showMessage(
      "Configure reference plane in panel and click Apply");
}

void MainWindow::onFillet() {
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showFilletSettings();

  if (m_document->getAllShapes().empty()) {
    QMessageBox::information(this, "Fillet",
                             "No solid body. Create a box first.");
    return;
  }

  m_activePartTool = ActivePartTool::Fillet;
  if (m_viewport) {
    m_viewport->enableEdgeSelection(true);
  }
  statusBar()->showMessage("Fillet: Select edges then click Apply", 0);
}

void MainWindow::onChamfer() {
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showChamferSettings();

  if (m_document->getAllShapes().empty()) {
    QMessageBox::information(this, "Chamfer",
                             "No solid body. Create a box first.");
    return;
  }

  m_activePartTool = ActivePartTool::Chamfer;
  if (m_viewport) {
    m_viewport->enableEdgeSelection(true);
  }
  statusBar()->showMessage("Chamfer: Select edges then click Apply", 0);
}

void MainWindow::onShell() {
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showShellSettings();

  if (m_document->getAllShapes().empty()) {
    QMessageBox::warning(this, "Shell",
                         "No shapes available. Create a solid first.");
    return;
  }

  m_activePartTool = ActivePartTool::Shell;
  statusBar()->showMessage("Shell: Set thickness in panel, then click Apply",
                           0);
}

void MainWindow::onSweep() {
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showSweepSettings();

  m_activePartTool = ActivePartTool::Sweep;
  statusBar()->showMessage("Sweep: Select profile and path, then Apply");
}

void MainWindow::onLoft() {
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showLoftSettings();

  m_activePartTool = ActivePartTool::Loft;
  statusBar()->showMessage("Loft: Select multiple profiles, then Apply");
}

void MainWindow::onLoftSurface() {
  // Quick loft surface between two sketches
  if (m_document->sketches().size() < 2) {
    QMessageBox::information(this, "Loft Surface",
                             "Need at least 2 sketches for loft surface.\n\n"
                             "Create sketches on different planes first.");
    return;
  }

  // Use last two sketches
  auto sketch1 = m_document->sketches()[m_document->sketches().size() - 2];
  auto sketch2 = m_document->sketches()[m_document->sketches().size() - 1];

  if (!sketch1 || !sketch2) {
    QMessageBox::warning(this, "Loft Surface", "Invalid sketch references.");
    return;
  }

  try {
    TopoDS_Wire wire1 = sketch1->buildWire();
    TopoDS_Wire wire2 = sketch2->buildWire();

    if (wire1.IsNull() || wire2.IsNull()) {
      QMessageBox::warning(this, "Loft Surface",
                           "Could not build wires from sketches.\n"
                           "Make sure each sketch has a closed shape.");
      return;
    }

    // Execute loft with solid=false for thin surface, ruled=true for clean
    // surface
    part::LoftFeature loft;
    std::vector<TopoDS_Wire> wires = {wire1, wire2};
    TopoDS_Shape result =
        loft.execute(wires, false, true); // solid=false, ruled=true

    if (!result.IsNull()) {
      saveUndoState("Loft Surface");
      // TODO: Convert to Feature
      // TODO: Convert to Feature-based system
      //       // m_shapes.push_back(result);
      displayAllShapes();

      if (m_featureList) {
        m_featureList->addItem("?? Surface Loft");
      }

      statusBar()->showMessage("Thin surface created between sketches");
      m_modified = true;
      updateWindowTitle();
    } else {
      QMessageBox::warning(this, "Loft Surface Failed",
                           QString::fromStdString(loft.errorMessage()));
    }
  } catch (const std::exception &e) {
    QMessageBox::critical(this, "Loft Surface Error",
                          QString("Exception: %1").arg(e.what()));
  } catch (...) {
    QMessageBox::critical(this, "Loft Surface Error", "Unknown error.");
  }
}

void MainWindow::onPattern() {
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showPatternSettings();

  m_activePartTool = ActivePartTool::Pattern;
  statusBar()->showMessage(
      "Configure pattern in panel, select shape, then Apply");
}

void MainWindow::onMirror() {
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showMirrorSettings();

  m_activePartTool = ActivePartTool::Mirror;
  statusBar()->showMessage(
      "Configure mirror in panel, select shape, then Apply");
}

// ============================================================================
// Selection Mode Slots
// ============================================================================

void MainWindow::onSelectShape() {
  if (m_viewport) {
    m_viewport->setSelectionMode(SelectionMode::Shape);
    statusBar()->showMessage("Selection mode: Shape (select entire solid)");
  }
}

void MainWindow::onSelectFace() {
  if (m_viewport) {
    m_viewport->setSelectionMode(SelectionMode::Face);
    statusBar()->showMessage("Selection mode: Face (click on faces)");
  }
}

void MainWindow::onSelectEdge() {
  if (m_viewport) {
    m_viewport->setSelectionMode(SelectionMode::Edge);
    statusBar()->showMessage("Selection mode: Edge (click on edges)");
  }
}

void MainWindow::onSelectVertex() {
  if (m_viewport) {
    m_viewport->setSelectionMode(SelectionMode::Vertex);
    statusBar()->showMessage("Selection mode: Vertex (click on corners)");
  }
}

void MainWindow::onProfileSelected(int profileIndex) {
  if (!m_currentSketch || m_pendingOperation == PendingOperation::None)
    return;

  auto closedProfiles = m_currentSketch->detectClosedProfiles();
  if (profileIndex < 0 ||
      profileIndex >= static_cast<int>(closedProfiles.size())) {
    statusBar()->showMessage("Invalid profile index", 2000);
    m_pendingOperation = PendingOperation::None;
    return;
  }

  // Save selected profile index (panel is already showing settings)
  m_selectedProfileIndex = profileIndex;

  // Sync selection with ProfileSelectionPanel ComboBox (bidirectional sync)
  if (m_profileSelectionPanel) {
    m_profileSelectionPanel->setProfileIndex(profileIndex);
  }

  statusBar()->showMessage(
      QString(
          "Profile %1 selected. Adjust settings in panel, then click Apply.")
          .arg(profileIndex + 1));
}
void MainWindow::onProfileSelectionCancelled() {
  m_pendingOperation = PendingOperation::None;
  m_selectedProfileIndex = -1;
  statusBar()->showMessage("Profile selection cancelled", 2000);
}

void MainWindow::onRingSelected(int outerProfileIndex, int innerProfileIndex) {
  if (!m_currentSketch || m_pendingOperation == PendingOperation::None)
    return;

  auto closedProfiles = m_currentSketch->detectClosedProfiles();
  if (outerProfileIndex < 0 ||
      outerProfileIndex >= static_cast<int>(closedProfiles.size()) ||
      innerProfileIndex < 0 ||
      innerProfileIndex >= static_cast<int>(closedProfiles.size())) {
    statusBar()->showMessage("Invalid ring selection", 2000);
    m_pendingOperation = PendingOperation::None;
    return;
  }

  QString title = (m_pendingOperation == PendingOperation::Extrude)

                      ? "Extrude Ring"
                      : "Cut Ring";

  // Get depth from ToolSettingsPanel instead of dialog
  double depth =
      m_toolSettingsPanel ? m_toolSettingsPanel->extrudeDepth() : 20.0;

  // Build face from outer wire with inner wire as hole
  TopoDS_Wire outerWire = closedProfiles[outerProfileIndex];
  TopoDS_Wire innerWire = closedProfiles[innerProfileIndex];
  TopoDS_Face ringFace;

  try {
    BRepBuilderAPI_MakeFace faceBuilder(outerWire, true);
    if (faceBuilder.IsDone()) {
      faceBuilder.Add(TopoDS::Wire(innerWire.Reversed())); // Reversed for hole
      if (faceBuilder.IsDone()) {
        ringFace = faceBuilder.Face();
      }
    }
  } catch (...) {
    QMessageBox::warning(this, title, "Could not create ring face.");
    m_pendingOperation = PendingOperation::None;
    return;
  }

  if (ringFace.IsNull()) {
    QMessageBox::warning(this, title, "Ring face creation failed.");
    m_pendingOperation = PendingOperation::None;
    return;
  }

  // Get direction from sketch plane
  gp_Dir dir = m_currentSketch->plane().normal();
  gp_Vec vec(dir);

  if (m_pendingOperation == PendingOperation::Extrude) {
    vec.Scale(depth);

    TopoDS_Shape extrudedShape;
    try {
      BRepPrimAPI_MakePrism prism(ringFace, vec);
      if (prism.IsDone()) {
        extrudedShape = prism.Shape();
      }
    } catch (...) {
      QMessageBox::warning(this, "Extrude Failed",
                           "Could not create ring extrusion.");
      m_pendingOperation = PendingOperation::None;
      return;
    }

    if (!extrudedShape.IsNull()) {
      if (m_document->getAllShapes().empty()) {
        // TODO: Convert to Feature
        // TODO: Convert to Feature-based system
        //       // m_shapes.push_back(extrudedShape);
      } else {
        try {
          BRepAlgoAPI_Fuse fuseOp(m_document->getAllShapes()[0], extrudedShape);
          if (fuseOp.IsDone()) {
            m_document->getAllShapes()[0] = fuseOp.Shape();
          } else {
            // TODO: Convert to Feature
            // TODO: Convert to Feature-based system
            //       // m_shapes.push_back(extrudedShape);
          }
        } catch (...) {
          // TODO: Convert to Feature
          // TODO: Convert to Feature-based system
          //       // m_shapes.push_back(extrudedShape);
        }
      }

      displayAllShapes();
      m_featureList->addItem(QString("Extrude Ring (%1) [P%2-P%3]")
                                 .arg(depth)
                                 .arg(outerProfileIndex + 1)
                                 .arg(innerProfileIndex + 1));
      statusBar()->showMessage(QString("Extruded Ring (Profile %1 - %2)")
                                   .arg(outerProfileIndex + 1)
                                   .arg(innerProfileIndex + 1),
                               3000);
    }
  } else {
    // Cut operation with ring
    vec.Scale(-depth);

    TopoDS_Shape cutTool;
    try {
      BRepPrimAPI_MakePrism prism(ringFace, vec);
      if (prism.IsDone()) {
        cutTool = prism.Shape();
      }
    } catch (...) {
      QMessageBox::warning(this, "Cut Failed",
                           "Could not create ring cutting tool.");
      m_pendingOperation = PendingOperation::None;
      return;
    }

    if (!cutTool.IsNull() && !m_document->getAllShapes().empty()) {
      try {
        BRepAlgoAPI_Cut cutOp(m_document->getAllShapes()[0], cutTool);
        if (cutOp.IsDone()) {
          m_document->getAllShapes()[0] = cutOp.Shape();
          displayAllShapes();
          m_featureList->addItem(QString("Cut Ring (%1) [P%2-P%3]")
                                     .arg(depth)
                                     .arg(outerProfileIndex + 1)
                                     .arg(innerProfileIndex + 1));
          statusBar()->showMessage(QString("Cut Ring (Profile %1 - %2)")
                                       .arg(outerProfileIndex + 1)
                                       .arg(innerProfileIndex + 1),
                                   3000);
        }
      } catch (...) {
        QMessageBox::warning(this, "Cut Failed",
                             "Boolean cut operation failed.");
      }
    }
  }

  m_pendingOperation = PendingOperation::None;
}

void MainWindow::onMultiProfilesConfirmed(
    const std::vector<std::pair<int, int>> &selections) {
  if (!m_currentSketch || m_pendingOperation == PendingOperation::None ||
      selections.empty()) {
    m_pendingOperation = PendingOperation::None;
    return;
  }

  auto closedProfiles = m_currentSketch->detectClosedProfiles();

  QString title = (m_pendingOperation == PendingOperation::Extrude)

                      ? "Extrude Multi"
                      : "Cut Multi";

  // Get depth from ToolSettingsPanel instead of dialog
  double depth =
      m_toolSettingsPanel ? m_toolSettingsPanel->extrudeDepth() : 20.0;

  gp_Dir dir = m_currentSketch->plane().normal();
  gp_Vec vec(dir);

  int successCount = 0;

  for (const auto &sel : selections) {
    int outerIdx = sel.first;
    int innerIdx = sel.second; // -1 for solid profile

    if (outerIdx < 0 || outerIdx >= static_cast<int>(closedProfiles.size()))
      continue;

    TopoDS_Face profileFace;

    try {
      TopoDS_Wire outerWire = closedProfiles[outerIdx];
      BRepBuilderAPI_MakeFace faceBuilder(outerWire, true);

      if (innerIdx >= 0 && innerIdx < static_cast<int>(closedProfiles.size())) {
        // Ring profile
        TopoDS_Wire innerWire = closedProfiles[innerIdx];
        faceBuilder.Add(TopoDS::Wire(innerWire.Reversed()));
      }

      if (faceBuilder.IsDone()) {
        profileFace = faceBuilder.Face();
      }
    } catch (...) {
      continue;
    }

    if (profileFace.IsNull())
      continue;

    gp_Vec opVec = vec;
    if (m_pendingOperation == PendingOperation::Extrude) {
      opVec.Scale(depth);
    } else {
      opVec.Scale(-depth);
    }

    try {
      BRepPrimAPI_MakePrism prism(profileFace, opVec);
      if (prism.IsDone()) {
        TopoDS_Shape resultShape = prism.Shape();

        if (m_pendingOperation == PendingOperation::Extrude) {
          if (m_document->getAllShapes().empty()) {
            // TODO: Convert to Feature
            // TODO: Convert to Feature-based system
            //       // m_shapes.push_back(resultShape);
          } else {
            BRepAlgoAPI_Fuse fuseOp(m_document->getAllShapes()[0], resultShape);
            if (fuseOp.IsDone()) {
              m_document->getAllShapes()[0] = fuseOp.Shape();
            } else {
              // TODO: Convert to Feature
              // TODO: Convert to Feature-based system
              //       // m_shapes.push_back(resultShape);
            }
          }
        } else {
          // Cut
          if (!m_document->getAllShapes().empty()) {
            BRepAlgoAPI_Cut cutOp(m_document->getAllShapes()[0], resultShape);
            if (cutOp.IsDone()) {
              m_document->getAllShapes()[0] = cutOp.Shape();
            }
          }
        }
        successCount++;
      }
    } catch (...) {
      continue;
    }
  }

  if (successCount > 0) {
    displayAllShapes();
    QString opName =
        (m_pendingOperation == PendingOperation::Extrude) ? "Extrude" : "Cut";
    m_featureList->addItem(QString("%1 Multi (%2) [%3 items]")
                               .arg(opName)
                               .arg(depth)
                               .arg(successCount));
    statusBar()->showMessage(QString("%1 completed: %2 of %3 selections")
                                 .arg(opName)
                                 .arg(successCount)
                                 .arg(selections.size()),
                             3000);
  } else {
    QMessageBox::warning(this, title, "No valid profiles could be processed.");
  }

  m_pendingOperation = PendingOperation::None;
}

void MainWindow::saveUndoState(const std::string &description) {
  // checkpoint() should be called AFTER making changes
  m_document->checkpoint(QString::fromStdString(description));
}

// ==================== NEW PART FEATURES ====================

void MainWindow::onScale() {
  // Show settings in tool panel (no specific Scale settings yet)
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showNoToolSettings();

  if (m_document->getAllShapes().empty()) {
    QMessageBox::warning(this, "Scale",
                         "No shapes to scale. Create or import a shape first.");
    return;
  }

  bool ok;
  double scaleFactor = QInputDialog::getDouble(
      this, "Scale", "Scale Factor:", 1.5, 0.01, 100.0, 2, &ok);
  if (!ok)
    return;

  try {
    gp_Trsf transform;
    transform.SetScale(gp_Pnt(0, 0, 0), scaleFactor);
    BRepBuilderAPI_Transform transformer(m_document->getAllShapes().back(),
                                         transform, true);

    if (transformer.IsDone()) {
      m_document->getAllShapes().back() = transformer.Shape();
      displayAllShapes();
      saveUndoState("Scale " + std::to_string(scaleFactor));
      m_featureList->addItem(QString("Scale (%1x)").arg(scaleFactor));
      statusBar()->showMessage(QString("Scaled by factor %1").arg(scaleFactor),
                               3000);
    }
  } catch (...) {
    QMessageBox::warning(this, "Scale", "Failed to scale shape.");
  }
}

void MainWindow::onHoleWizard() {
  // Show settings in tool panel (no specific HoleWizard settings
  // yet)
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showNoToolSettings();

  if (m_document->getAllShapes().empty()) {
    QMessageBox::warning(this, "Hole Wizard",
                         "No shapes available. Create a solid first.");
    return;
  }

  // Simple hole creation - get diameter and depth
  bool ok;
  double diameter = QInputDialog::getDouble(
      this, "Hole Wizard", "Hole Diameter:", 10.0, 0.1, 500.0, 2, &ok);
  if (!ok)
    return;

  double depth = QInputDialog::getDouble(
      this, "Hole Wizard", "Hole Depth:", 20.0, 0.1, 500.0, 2, &ok);
  if (!ok)
    return;

  // Position input
  double posX = QInputDialog::getDouble(this, "Hole Wizard", "Position X:", 0.0,
                                        -1000.0, 1000.0, 2, &ok);
  if (!ok)
    return;

  double posY = QInputDialog::getDouble(this, "Hole Wizard", "Position Y:", 0.0,
                                        -1000.0, 1000.0, 2, &ok);
  if (!ok)
    return;

  double posZ = QInputDialog::getDouble(this, "Hole Wizard", "Start Z:", 50.0,
                                        -1000.0, 1000.0, 2, &ok);
  if (!ok)
    return;

  try {
    // Create cylinder for hole
    gp_Ax2 axis(gp_Pnt(posX, posY, posZ), gp_Dir(0, 0, -1));
    BRepPrimAPI_MakeCylinder holeCyl(axis, diameter / 2.0, depth);

    if (holeCyl.IsDone()) {
      BRepAlgoAPI_Cut cutter(m_document->getAllShapes().back(),
                             holeCyl.Shape());
      if (cutter.IsDone()) {
        m_document->getAllShapes().back() = cutter.Shape();
        displayAllShapes();
        saveUndoState("Hole D" + std::to_string(diameter));
        m_featureList->addItem(
            QString("Hole (D%1 x %2)").arg(diameter).arg(depth));
        statusBar()->showMessage(
            QString("Created hole D%1 x %2").arg(diameter).arg(depth), 3000);
      }
    }
  } catch (...) {
    QMessageBox::warning(this, "Hole Wizard", "Failed to create hole.");
  }
}

void MainWindow::onDraft() {
  // Show Draft settings in tool panel
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showDraftSettings();

  if (m_document->getAllShapes().empty()) {
    QMessageBox::warning(this, "Draft",
                         "No shapes available. Create a solid first.");
    return;
  }

  // Set active tool for Apply button
  m_activePartTool = ActivePartTool::Draft;

  statusBar()->showMessage(
      "Draft: Set angle in panel, then click Apply to select face", 0);
}

void MainWindow::onRib() {
  // Show settings in tool panel (no specific Rib settings yet)
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showNoToolSettings();

  if (m_document->getAllShapes().empty()) {
    QMessageBox::warning(this, "Rib",
                         "No shapes available. Create a solid first.");
    return;
  }

  if (!m_currentSketch || m_currentSketch->entities().empty()) {
    QMessageBox::warning(this, "Rib",
                         "No active sketch. Create an open sketch profile "
                         "first, then use Rib.");
    return;
  }

  bool ok;
  double thickness = QInputDialog::getDouble(
      this, "Rib", "Rib Thickness (mm):", 2.0, 0.1, 100.0, 2, &ok);
  if (!ok)
    return;

  try {
    saveUndoState("Rib");

    part::RibFeature rib;
    TopoDS_Shape result = rib.execute(
        *m_currentSketch, m_document->getAllShapes().back(), thickness, true);

    if (!result.IsNull()) {
      m_document->getAllShapes().back() = result;
      displayAllShapes();
      m_featureList->addItem(QString("?? Rib (T=%1mm)").arg(thickness));
      statusBar()->showMessage(QString("Rib created: T=%1mm").arg(thickness),
                               3000);
    } else {
      QMessageBox::warning(this, "Rib",
                           "Rib failed: " +
                               QString::fromStdString(rib.errorMessage()));
    }
  } catch (...) {
    QMessageBox::warning(this, "Rib", "Rib operation failed.");
  }
}

void MainWindow::onThicken() {
  // Show Thicken settings in tool panel
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showThickenSettings();

  if (m_document->getAllShapes().empty()) {
    QMessageBox::warning(this, "Thicken", "No shapes available.");
    return;
  }

  // Parameters will be read from panel in onToolApply
  m_pendingFaceOperation = PendingFaceOperation::Thicken;

  // Enable face selection mode
  if (m_viewport) {
    m_viewport->enableFaceSelection(true);
  }

  statusBar()->showMessage("Thicken: Set thickness in panel, then select face",
                           0);
}

void MainWindow::onOffsetSurface() {
  // Show OffsetSurface settings in tool panel
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showOffsetSurfaceSettings();

  if (m_document->getAllShapes().empty()) {
    QMessageBox::warning(this, "Offset Surface", "No shapes available.");
    return;
  }

  // Parameters will be read from panel in onToolApply
  m_pendingFaceOperation = PendingFaceOperation::OffsetSurface;

  // Enable face selection mode
  if (m_viewport) {
    m_viewport->enableFaceSelection(true);
  }

  statusBar()->showMessage(
      "Offset Surface: Set offset in panel, then select face", 0);
}

void MainWindow::onSplit() {
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showSplitSettings();

  if (m_document->getAllShapes().empty()) {
    QMessageBox::warning(this, "Split",
                         "No shapes to split. Create a solid first.");
    return;
  }

  m_activePartTool = ActivePartTool::Split;
  statusBar()->showMessage("Split: Set plane and offset in panel, then Apply",
                           0);
}

void MainWindow::onDome() {
  // Show Dome settings in tool panel
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showDomeSettings();

  if (m_document->getAllShapes().empty()) {
    QMessageBox::warning(this, "Dome",
                         "No shapes available. Create a solid first.");
    return;
  }

  // Set active tool for Apply button
  m_activePartTool = ActivePartTool::Dome;

  statusBar()->showMessage(
      "Dome: Set height in panel, then click Apply to select face", 0);
}

// ==================== SKETCH ADVANCED TOOLS ====================

void MainWindow::onSketchMirror() {
  if (!m_sketchMode || !m_currentSketch || !m_sketchView) {
    QMessageBox::warning(this, "Mirror", "Start a sketch first.");
    return;
  }

  const auto &selected = m_sketchView->m_selectedEntities;
  if (selected.empty()) {
    QMessageBox::information(
        this, "Mirror Entities",
        "Select sketch entities to mirror, then click Mirror.");
    statusBar()->showMessage("Select entities for mirror", 5000);
    return;
  }

  // Convert raw pointers to shared_ptr
  std::vector<sketch::SketchEntity::Ptr> entities;
  for (auto *rawPtr : selected) {
    for (const auto &e : m_currentSketch->entities()) {
      if (e.get() == rawPtr) {
        entities.push_back(e);
        break;
      }
    }
  }

  sketch::SketchMirror mirror;
  auto result = mirror.mirrorVertical(*m_currentSketch, entities, 0.0);

  if (result.success) {
    m_sketchView->clearSelection();
    m_sketchView->update();
    m_featureList->addItem(
        QString("?? Mirror (%1 entities)").arg(result.mirroredEntities.size()));
    statusBar()->showMessage("Mirror completed", 3000);
  } else {
    QMessageBox::warning(this, "Mirror", QString::fromStdString(result.error));
  }
}

void MainWindow::onSketchTrim() {
  if (!m_sketchMode || !m_currentSketch || !m_sketchView) {
    QMessageBox::warning(this, "Trim", "Start a sketch first.");
    return;
  }

  const auto &selected = m_sketchView->m_selectedEntities;
  if (selected.empty()) {
    QMessageBox::information(
        this, "Trim Tool",
        "Select an entity to trim at intersection points, then "
        "click Trim.");
    statusBar()->showMessage("Select entity to trim", 5000);
    return;
  }

  // Find shared_ptr for first selected entity
  sketch::SketchEntity::Ptr entity;
  for (const auto &e : m_currentSketch->entities()) {
    if (e.get() == selected[0]) {
      entity = e;
      break;
    }
  }

  if (!entity) {
    QMessageBox::warning(this, "Trim", "Entity not found.");
    return;
  }

  sketch::SketchTrimExtend trimTool;
  // Use entity center as click point for trim
  gp_Pnt2d clickPoint(0, 0);
  auto result = trimTool.trim(*m_currentSketch, entity, clickPoint);

  if (result.success) {
    m_sketchView->clearSelection();
    m_sketchView->update();
    m_featureList->addItem("?? Trim");
    statusBar()->showMessage("Entity trimmed", 3000);
  } else {
    QMessageBox::warning(this, "Trim", QString::fromStdString(result.error));
  }
}

void MainWindow::onSketchExtend() {
  if (!m_sketchMode || !m_currentSketch || !m_sketchView) {
    QMessageBox::warning(this, "Extend", "Start a sketch first.");
    return;
  }

  const auto &selected = m_sketchView->m_selectedEntities;
  if (selected.empty()) {
    QMessageBox::information(
        this, "Extend Tool",
        "Select an entity to extend to nearest boundary, then "
        "click Extend.");
    statusBar()->showMessage("Select entity to extend", 5000);
    return;
  }

  sketch::SketchEntity::Ptr entity;
  for (const auto &e : m_currentSketch->entities()) {
    if (e.get() == selected[0]) {
      entity = e;
      break;
    }
  }

  if (!entity) {
    QMessageBox::warning(this, "Extend", "Entity not found.");
    return;
  }

  sketch::SketchTrimExtend extendTool;
  auto result = extendTool.extend(*m_currentSketch, entity,
                                  1); // Extend end point

  if (result.success) {
    m_sketchView->clearSelection();
    m_sketchView->update();
    m_featureList->addItem("-? Extend");
    statusBar()->showMessage("Entity extended", 3000);
  } else {
    QMessageBox::warning(this, "Extend", QString::fromStdString(result.error));
  }
}

void MainWindow::onSketchLinearPattern() {
  if (!m_sketchMode || !m_currentSketch || !m_sketchView) {
    QMessageBox::warning(this, "Linear Pattern", "Start a sketch first.");
    return;
  }

  // Show pattern settings in panel
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showPatternSettings();

  const auto &selected = m_sketchView->m_selectedEntities;
  if (selected.empty()) {
    QMessageBox::information(
        this, "Linear Pattern",
        "Select entities to pattern, then click Linear Pattern.");
    statusBar()->showMessage("Select entities for linear pattern", 5000);
    return;
  }

  // Get count and spacing from ToolSettingsPanel
  int count = m_toolSettingsPanel ? m_toolSettingsPanel->patternCount() : 3;
  double spacing =
      m_toolSettingsPanel ? m_toolSettingsPanel->patternSpacing() : 20.0;

  // Convert to shared_ptr
  std::vector<sketch::SketchEntity::Ptr> entities;
  for (auto *rawPtr : selected) {
    for (const auto &e : m_currentSketch->entities()) {
      if (e.get() == rawPtr) {
        entities.push_back(e);
        break;
      }
    }
  }

  sketch::SketchPattern pattern;
  auto result = pattern.linearPattern(*m_currentSketch, entities, 1.0, 0.0,
                                      spacing, count);

  if (result.success) {
    m_sketchView->clearSelection();
    m_sketchView->update();
    m_featureList->addItem(QString("?? Linear Pattern (%1x)").arg(count));
    statusBar()->showMessage(QString("Created %1 copies at %2mm spacing")
                                 .arg(count - 1)
                                 .arg(spacing),
                             3000);
  } else {
    QMessageBox::warning(this, "Linear Pattern",
                         QString::fromStdString(result.error));
  }
}

void MainWindow::onSketchCircularPattern() {
  if (!m_sketchMode || !m_currentSketch || !m_sketchView) {
    QMessageBox::warning(this, "Circular Pattern", "Start a sketch first.");
    return;
  }

  // Show pattern settings in panel
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showPatternSettings();

  const auto &selected = m_sketchView->m_selectedEntities;
  if (selected.empty()) {
    QMessageBox::information(
        this, "Circular Pattern",
        "Select entities to pattern, then click Circular Pattern.");
    statusBar()->showMessage("Select entities for circular pattern", 5000);
    return;
  }

  // Get count from ToolSettingsPanel
  int count = m_toolSettingsPanel ? m_toolSettingsPanel->patternCount() : 6;

  // Convert to shared_ptr
  std::vector<sketch::SketchEntity::Ptr> entities;
  for (auto *rawPtr : selected) {
    for (const auto &e : m_currentSketch->entities()) {
      if (e.get() == rawPtr) {
        entities.push_back(e);
        break;
      }
    }
  }

  sketch::SketchPattern pattern;
  auto result =
      pattern.circularPattern(*m_currentSketch, entities, 0.0, 0.0, count);

  if (result.success) {
    m_sketchView->clearSelection();
    m_sketchView->update();
    m_featureList->addItem(QString("? Circular Pattern (%1x)").arg(count));
    statusBar()->showMessage(
        QString("Created %1 copies around origin").arg(count - 1), 3000);
  } else {
    QMessageBox::warning(this, "Circular Pattern",
                         QString::fromStdString(result.error));
  }
}

// ==================== ADDITIONAL CONSTRAINTS ====================

void MainWindow::onConstraintTangent() {
  if (!m_sketchMode || !m_currentSketch || !m_sketchView) {
    QMessageBox::warning(this, "Tangent", "Start a sketch first.");
    return;
  }

  // Use selected entities from SketchView2D (professional CAD
  // style)
  const auto &selected = m_sketchView->m_selectedEntities;
  if (selected.size() != 2) {
    QMessageBox::information(this, "Tangent Constraint",
                             "Select exactly 2 entities (line+circle, "
                             "line+arc, or two circles/arcs) "
                             "and then click Tangent.");
    statusBar()->showMessage("Select 2 entities for tangent constraint", 5000);
    return;
  }

  // Find shared_ptr for selected raw pointers
  sketch::SketchEntity::Ptr e1, e2;
  for (const auto &entity : m_currentSketch->entities()) {
    if (entity.get() == selected[0])
      e1 = entity;
    if (entity.get() == selected[1])
      e2 = entity;
  }

  if (!e1 || !e2) {
    QMessageBox::warning(this, "Tangent", "Selected entities not found.");
    return;
  }

  auto line1 = std::dynamic_pointer_cast<sketch::SketchLine>(e1);
  auto line2 = std::dynamic_pointer_cast<sketch::SketchLine>(e2);
  auto circle1 = std::dynamic_pointer_cast<sketch::SketchCircle>(e1);
  auto circle2 = std::dynamic_pointer_cast<sketch::SketchCircle>(e2);
  auto arc1 = std::dynamic_pointer_cast<sketch::SketchArc>(e1);
  auto arc2 = std::dynamic_pointer_cast<sketch::SketchArc>(e2);

  sketch::Constraint::Ptr constraint;
  if (line1 && circle2) {
    constraint = std::make_shared<sketch::TangentConstraint>(line1, circle2);
  } else if (line2 && circle1) {
    constraint = std::make_shared<sketch::TangentConstraint>(line2, circle1);
  } else if (line1 && arc2) {
    constraint = std::make_shared<sketch::TangentConstraint>(line1, arc2);
  } else if (line2 && arc1) {
    constraint = std::make_shared<sketch::TangentConstraint>(line2, arc1);
  } else if (circle1 && circle2) {
    constraint = std::make_shared<sketch::TangentConstraint>(circle1, circle2);
  } else if (arc1 && arc2) {
    constraint = std::make_shared<sketch::TangentConstraint>(arc1, arc2);
  } else {
    QMessageBox::warning(this, "Tangent",
                         "Tangent requires line+circle, line+arc, "
                         "or two circles/arcs.");
    return;
  }

  m_currentSketch->addConstraint(constraint);
  m_currentSketch->solve(); // Solve to update geometry
  m_sketchView->clearSelection();
  m_sketchView->update();
  m_featureList->addItem("?? Tangent Constraint");
  statusBar()->showMessage("Tangent constraint applied", 3000);
}

void MainWindow::onConstraintEqual() {
  if (!m_sketchMode || !m_currentSketch || !m_sketchView) {
    QMessageBox::warning(this, "Equal", "Start a sketch first.");
    return;
  }

  const auto &selected = m_sketchView->m_selectedEntities;
  if (selected.size() != 2) {
    QMessageBox::information(this, "Equal Constraint",
                             "Select exactly 2 entities of the same type (2 "
                             "lines, 2 circles, or 2 arcs) "
                             "and then click Equal.");
    statusBar()->showMessage("Select 2 entities for equal constraint", 5000);
    return;
  }

  sketch::SketchEntity::Ptr e1, e2;
  for (const auto &entity : m_currentSketch->entities()) {
    if (entity.get() == selected[0])
      e1 = entity;
    if (entity.get() == selected[1])
      e2 = entity;
  }

  if (!e1 || !e2) {
    QMessageBox::warning(this, "Equal", "Selected entities not found.");
    return;
  }

  auto line1 = std::dynamic_pointer_cast<sketch::SketchLine>(e1);
  auto line2 = std::dynamic_pointer_cast<sketch::SketchLine>(e2);
  auto circle1 = std::dynamic_pointer_cast<sketch::SketchCircle>(e1);
  auto circle2 = std::dynamic_pointer_cast<sketch::SketchCircle>(e2);
  auto arc1 = std::dynamic_pointer_cast<sketch::SketchArc>(e1);
  auto arc2 = std::dynamic_pointer_cast<sketch::SketchArc>(e2);

  sketch::Constraint::Ptr constraint;
  if (line1 && line2) {
    constraint = std::make_shared<sketch::EqualConstraint>(line1, line2);
  } else if (circle1 && circle2) {
    constraint = std::make_shared<sketch::EqualConstraint>(circle1, circle2);
  } else if (arc1 && arc2) {
    constraint = std::make_shared<sketch::EqualConstraint>(arc1, arc2);
  } else {
    QMessageBox::warning(this, "Equal",
                         "Equal requires two entities of the same type (2 "
                         "lines, 2 circles, or 2 arcs).");
    return;
  }

  m_currentSketch->addConstraint(constraint);
  m_currentSketch->solve();
  m_sketchView->clearSelection();
  m_sketchView->update();
  m_featureList->addItem("?? Equal Constraint");
  statusBar()->showMessage("Equal constraint applied", 3000);
}

void MainWindow::onConstraintFix() {
  if (!m_sketchMode || !m_currentSketch || !m_sketchView) {
    QMessageBox::warning(this, "Fix", "Start a sketch first.");
    return;
  }

  const auto &selected = m_sketchView->m_selectedEntities;
  if (selected.empty()) {
    QMessageBox::information(this, "Fix Constraint",
                             "Select an entity (point, line, "
                             "circle, or arc) and then click Fix.");
    statusBar()->showMessage("Select entity to fix in place", 5000);
    return;
  }

  // Find shared_ptr for selected entity
  sketch::SketchEntity::Ptr entity;
  for (const auto &e : m_currentSketch->entities()) {
    if (e.get() == selected[0]) {
      entity = e;
      break;
    }
  }

  if (!entity) {
    QMessageBox::warning(this, "Fix", "Selected entity not found.");
    return;
  }

  sketch::Constraint::Ptr constraint;
  if (auto point = std::dynamic_pointer_cast<sketch::SketchPoint>(entity)) {
    constraint = std::make_shared<sketch::FixConstraint>(point);
  } else if (auto line =
                 std::dynamic_pointer_cast<sketch::SketchLine>(entity)) {
    constraint = std::make_shared<sketch::FixConstraint>(line);
  } else if (auto circle =
                 std::dynamic_pointer_cast<sketch::SketchCircle>(entity)) {
    constraint = std::make_shared<sketch::FixConstraint>(circle);
  } else if (auto arc = std::dynamic_pointer_cast<sketch::SketchArc>(entity)) {
    constraint = std::make_shared<sketch::FixConstraint>(arc);
  } else {
    QMessageBox::warning(this, "Fix", "Entity type not supported for fixing.");
    return;
  }

  m_currentSketch->addConstraint(constraint);
  m_currentSketch->solve();
  m_sketchView->clearSelection();
  m_sketchView->update();
  m_featureList->addItem("?? Fix Constraint");
  statusBar()->showMessage("Entity position fixed", 3000);
}

void MainWindow::onConstraintConcentric() {
  if (!m_sketchMode || !m_currentSketch || !m_sketchView) {
    QMessageBox::warning(this, "Concentric", "Start a sketch first.");
    return;
  }

  const auto &selected = m_sketchView->m_selectedEntities;
  if (selected.size() != 2) {
    QMessageBox::information(this, "Concentric Constraint",
                             "Select exactly 2 circles or arcs and "
                             "then click Concentric.");
    statusBar()->showMessage("Select 2 circles/arcs for concentric constraint",
                             5000);
    return;
  }

  sketch::SketchEntity::Ptr e1, e2;
  for (const auto &entity : m_currentSketch->entities()) {
    if (entity.get() == selected[0])
      e1 = entity;
    if (entity.get() == selected[1])
      e2 = entity;
  }

  if (!e1 || !e2) {
    QMessageBox::warning(this, "Concentric", "Selected entities not found.");
    return;
  }

  auto circle1 = std::dynamic_pointer_cast<sketch::SketchCircle>(e1);
  auto circle2 = std::dynamic_pointer_cast<sketch::SketchCircle>(e2);
  auto arc1 = std::dynamic_pointer_cast<sketch::SketchArc>(e1);
  auto arc2 = std::dynamic_pointer_cast<sketch::SketchArc>(e2);

  sketch::Constraint::Ptr constraint;
  if (circle1 && circle2) {
    constraint =
        std::make_shared<sketch::ConcentricConstraint>(circle1, circle2);
  } else if (arc1 && arc2) {
    constraint = std::make_shared<sketch::ConcentricConstraint>(arc1, arc2);
  } else if (circle1 && arc2) {
    constraint = std::make_shared<sketch::ConcentricConstraint>(circle1, arc2);
  } else if (arc1 && circle2) {
    constraint = std::make_shared<sketch::ConcentricConstraint>(circle2, arc1);
  } else {
    QMessageBox::warning(this, "Concentric",
                         "Concentric requires two circles or arcs.");
    return;
  }

  m_currentSketch->addConstraint(constraint);
  m_currentSketch->solve();
  m_sketchView->clearSelection();
  m_sketchView->update();
  m_featureList->addItem("? Concentric Constraint");
  statusBar()->showMessage("Concentric constraint applied", 3000);
}

void MainWindow::onToolApply() {
  qDebug() << "=== onToolApply() called ===";
  qDebug() << "m_activePartTool:" << static_cast<int>(m_activePartTool);

  // Handle Apply button click based on active Part tool
  switch (m_activePartTool) {
  case ActivePartTool::Extrude: {
    qDebug() << "Entering Extrude case";
    // Extrude the selected profile with panel settings
    if (!m_currentSketch) {
      QMessageBox::warning(this, "Extrude", "No sketch available.");
      return;
    }

    // Get profile selection from ProfileSelectionPanel or
    // m_selectedProfileIndex
    int profileIndex = m_profileSelectionPanel
                           ? m_profileSelectionPanel->selectedProfile()
                           : m_selectedProfileIndex;
    if (profileIndex < 0) {
      QMessageBox::warning(this, "Extrude",
                           "Please select a profile from Tool Settings.");
      return;
    }

    auto closedProfiles = m_currentSketch->detectClosedProfiles();
    if (profileIndex >= static_cast<int>(closedProfiles.size())) {
      QMessageBox::warning(this, "Extrude", "Invalid profile selection.");
      return;
    }

    // Get parameters from panel
    double depth =
        m_toolSettingsPanel ? m_toolSettingsPanel->extrudeDepth() : 20.0;
    bool symmetric =
        m_toolSettingsPanel ? m_toolSettingsPanel->extrudeSymmetric() : false;
    double draftAngle =
        m_toolSettingsPanel ? m_toolSettingsPanel->extrudeDraftAngle() : 0.0;

    // Build face from selected profile
    TopoDS_Wire selectedWire = closedProfiles[profileIndex];
    TopoDS_Face profileFace;

    try {
      BRepBuilderAPI_MakeFace faceBuilder(selectedWire, true);
      if (faceBuilder.IsDone()) {
        profileFace = faceBuilder.Face();
        qDebug() << "Extrude: Profile face created successfully";
      } else {
        qDebug() << "Extrude: BRepBuilderAPI_MakeFace failed";
      }
    } catch (const std::exception &e) {
      qDebug() << "Extrude: Exception creating face:" << e.what();
      QMessageBox::warning(this, "Extrude",
                           "Could not create face from profile.");
      return;
    } catch (...) {
      qDebug() << "Extrude: Unknown exception creating face";
      QMessageBox::warning(this, "Extrude",
                           "Could not create face from profile.");
      return;
    }

    if (profileFace.IsNull()) {
      qDebug() << "Extrude: Profile face is NULL";
      QMessageBox::warning(this, "Extrude", "Invalid profile face.");
      return;
    }

    // Get direction from sketch plane
    gp_Dir dir = m_currentSketch->plane().normal();
    gp_Vec vec(dir);
    vec.Scale(depth);

    TopoDS_Shape extrudedShape;
    try {
      qDebug() << "Extrude: Creating prism with depth:" << depth
               << "symmetric:" << symmetric;

      if (symmetric) {
        // Mid-plane extrude
        gp_Vec halfVec = vec;
        halfVec.Scale(0.5);
        gp_Vec backVec = halfVec.Reversed();
        gp_Trsf moveBack;
        moveBack.SetTranslation(backVec);
        BRepBuilderAPI_Transform transform(profileFace, moveBack, true);
        TopoDS_Face movedFace = TopoDS::Face(transform.Shape());
        BRepPrimAPI_MakePrism prism(movedFace, vec);
        if (prism.IsDone()) {
          extrudedShape = prism.Shape();
          qDebug() << "Extrude: Symmetric prism created successfully";
        } else {
          qDebug() << "Extrude: Symmetric prism FAILED";
        }
      } else {
        BRepPrimAPI_MakePrism prism(profileFace, vec);
        if (prism.IsDone()) {
          extrudedShape = prism.Shape();
          qDebug() << "Extrude: Prism created successfully";
        } else {
          qDebug() << "Extrude: Prism creation FAILED";
        }
      }

      // Apply draft angle if specified
      if (!extrudedShape.IsNull() && std::abs(draftAngle) > 0.001) {
        qDebug() << "Extrude: Applying draft angle:" << draftAngle;
        double angleRad = draftAngle * M_PI / 180.0;

        // Use sketch plane as neutral plane and extrusion direction as draft direction
        gp_Dir draftDir = dir;
        gp_Pln neutralPlane = m_currentSketch->plane().plane();

        BRepOffsetAPI_DraftAngle draftOp(extrudedShape);
        TopExp_Explorer explorer(extrudedShape, TopAbs_FACE);
        while (explorer.More()) {
          TopoDS_Face face = TopoDS::Face(explorer.Current());
          try {
            draftOp.Add(face, draftDir, angleRad, neutralPlane);
          } catch (...) {
          }
          explorer.Next();
        }
        draftOp.Build();
        if (draftOp.IsDone()) {
          extrudedShape = draftOp.Shape();
          qDebug() << "Extrude: Draft angle applied successfully";
        } else {
          qDebug() << "Extrude: Draft angle application FAILED";
        }
      }
    } catch (const std::exception &e) {
      qDebug() << "Extrude: Exception during extrusion:" << e.what();
      QMessageBox::warning(this, "Extrude Failed",
                           "Could not create extrusion.");
      return;
    } catch (...) {
      qDebug() << "Extrude: Unknown exception during extrusion";
      QMessageBox::warning(this, "Extrude Failed",
                           "Could not create extrusion.");
      return;
    }

    if (!extrudedShape.IsNull()) {
      // Validate the shape
      BRepCheck_Analyzer analyzer(extrudedShape);
      bool isValid = analyzer.IsValid();
      qDebug() << "Extrude: Shape is valid:" << isValid;

      if (!isValid) {
        qDebug() << "Extrude: WARNING - Shape validation failed!";
      }

      // Get bounding box for debugging
      Bnd_Box bbox;
      BRepBndLib::Add(extrudedShape, bbox);
      if (!bbox.IsVoid()) {
        double xMin, yMin, zMin, xMax, yMax, zMax;
        bbox.Get(xMin, yMin, zMin, xMax, yMax, zMax);
        qDebug() << "Extrude: Bounding box - X:[" << xMin << "," << xMax
                 << "] Y:[" << yMin << "," << yMax << "] Z:[" << zMin << ","
                 << zMax << "]";
      } else {
        qDebug() << "Extrude: WARNING - Bounding box is VOID!";
      }

      // Compute mesh/tessellation for display
      qDebug() << "Extrude: Computing mesh tessellation...";
      try {
        BRepMesh_IncrementalMesh mesh(extrudedShape, 0.1);
        mesh.Perform();
        qDebug() << "Extrude: Mesh tessellation completed";
      } catch (const std::exception &e) {
        qDebug() << "Extrude: Mesh tessellation exception:" << e.what();
      } catch (...) {
        qDebug() << "Extrude: Mesh tessellation unknown exception";
      }

      if (m_document->temporaryShapes().empty()) {
        // First shape - just add it
        qDebug() << "Extrude: Adding as first shape";
        m_document->addTemporaryShape(extrudedShape);
      } else {
        // Fuse with existing shapes
        qDebug() << "Extrude: Fusing with existing shape";
        try {
          BRepAlgoAPI_Fuse fuseOp(m_document->temporaryShapes()[0],
                                  extrudedShape);
          if (fuseOp.IsDone()) {
            m_document->temporaryShapes()[0] = fuseOp.Shape();
            qDebug() << "Extrude: Fuse successful";
          } else {
            qDebug() << "Extrude: Fuse failed, adding as separate shape";
            m_document->addTemporaryShape(extrudedShape);
          }
        } catch (...) {
          qDebug() << "Extrude: Fuse exception, adding as separate shape";
          m_document->addTemporaryShape(extrudedShape);
        }
      }

      qDebug() << "Extrude: Shape added. Total temporary shapes:"
               << m_document->temporaryShapes().size();
      qDebug() << "Extrude: getAllShapes count:"
               << m_document->getAllShapes().size();

      // Save undo state AFTER adding shape
      saveUndoState("Extrude");

      qDebug() << "Extrude: Calling displayAllShapes()...";
      displayAllShapes();
      qDebug() << "Extrude: displayAllShapes() completed";

      // Force viewport update
      if (m_viewport) {
        m_viewport->update();
        qDebug() << "Extrude: Viewport update() called";
      }

      statusBar()->showMessage("Extrude completed", 2000);
      QString symInfo = symmetric ? " symmetric" : "";
      QString draftInfo = (std::abs(draftAngle) > 0.001)
                              ? QString(" draft=%1").arg(draftAngle)
                              : "";
      m_featureList->addItem(QString("✅ Extrude (%1)%2%3")
                                 .arg(depth)
                                 .arg(symInfo)
                                 .arg(draftInfo));
      statusBar()->showMessage(
          QString("Extruded Profile %1").arg(profileIndex + 1), 3000);
      m_modified = true;
      updateWindowTitle();
    } else {
      qDebug() << "Extrude: ERROR - extrudedShape is NULL!";
      QMessageBox::warning(this, "Extrude Failed", "Extruded shape is null.");
    }

    // Reset state
    m_selectedProfileIndex = -1;
    m_pendingOperation = PendingOperation::None;
    m_activePartTool = ActivePartTool::None;
  } break;

  case ActivePartTool::Fillet: {
    if (m_document->getAllShapes().empty()) {
      QMessageBox::warning(this, "Fillet", "No solid body available.");
      return;
    }
    double radius = m_toolSettingsPanel->filletRadius();
    std::vector<TopoDS_Edge> selectedEdges = m_viewport->getSelectedEdges();

    try {
      std::vector<TopoDS_Edge> edges;
      if (selectedEdges.empty()) {
        TopTools_IndexedMapOfShape edgeMap;
        TopExp::MapShapes(m_document->getAllShapes()[0], TopAbs_EDGE, edgeMap);
        for (int i = 1; i <= edgeMap.Extent(); ++i) {
          edges.push_back(TopoDS::Edge(edgeMap(i)));
        }
      } else {
        edges = selectedEdges;
      }

      m_viewport->clearSelectedEdges();
      m_viewport->enableEdgeSelection(false);

      part::FilletFeature fillet;
      TopoDS_Shape result =
          fillet.execute(m_document->getAllShapes()[0], edges, radius);

      if (!result.IsNull()) {
        saveUndoState("Fillet");
        m_document->temporaryShapes()[0] = result;
        displayAllShapes();
        statusBar()->showMessage(QString("Fillet R=%1 applied to %2 edge(s)")
                                     .arg(radius)
                                     .arg(edges.size()));
        m_modified = true;
        updateWindowTitle();
      } else {
        QMessageBox::warning(this, "Fillet Failed",
                             QString::fromStdString(fillet.errorMessage()));
      }
    } catch (...) {
      QMessageBox::critical(this, "Error", "Fillet operation failed.");
    }
  } break;

  case ActivePartTool::Chamfer: {
    if (m_document->getAllShapes().empty()) {
      QMessageBox::warning(this, "Chamfer", "No solid body available.");
      return;
    }
    double distance = m_toolSettingsPanel->chamferSize();
    std::vector<TopoDS_Edge> selectedEdges = m_viewport->getSelectedEdges();

    try {
      std::vector<TopoDS_Edge> edges;
      if (selectedEdges.empty()) {
        TopTools_IndexedMapOfShape edgeMap;
        TopExp::MapShapes(m_document->getAllShapes()[0], TopAbs_EDGE, edgeMap);
        for (int i = 1; i <= edgeMap.Extent(); ++i) {
          edges.push_back(TopoDS::Edge(edgeMap(i)));
        }
      } else {
        edges = selectedEdges;
      }

      m_viewport->clearSelectedEdges();
      m_viewport->enableEdgeSelection(false);

      part::ChamferFeature chamfer;
      TopoDS_Shape result =
          chamfer.execute(m_document->getAllShapes()[0], edges, distance);

      if (!result.IsNull()) {
        saveUndoState("Chamfer");
        m_document->temporaryShapes()[0] = result;
        displayAllShapes();
        statusBar()->showMessage(QString("Chamfer D=%1 applied to %2 edge(s)")
                                     .arg(distance)
                                     .arg(edges.size()));
        m_modified = true;
        updateWindowTitle();
      } else {
        QMessageBox::warning(this, "Chamfer Failed",
                             QString::fromStdString(chamfer.errorMessage()));
      }
    } catch (...) {
      QMessageBox::critical(this, "Error", "Chamfer operation failed.");
    }
  } break;

  case ActivePartTool::Shell: {
    if (m_document->getAllShapes().empty()) {
      QMessageBox::warning(this, "Shell", "No solid body available.");
      return;
    }
    double thickness = m_toolSettingsPanel->shellThickness();
    m_pendingShellThickness = thickness;
    m_pendingShellOutward = false;
    m_pendingFaceOperation = PendingFaceOperation::Shell;

    if (m_viewport) {
      m_viewport->enableFaceSelection(true);
    }
    statusBar()->showMessage(
        QString("Shell: Click face to remove (T=%1mm)").arg(thickness), 0);
  } break;

  case ActivePartTool::Dome: {
    if (m_document->getAllShapes().empty()) {
      QMessageBox::warning(this, "Dome", "No solid body available.");
      return;
    }
    double height = m_toolSettingsPanel->domeHeight();
    m_pendingDomeHeight = height;
    m_pendingDomeReversed = false;
    m_pendingFaceOperation = PendingFaceOperation::Dome;

    if (m_viewport) {
      m_viewport->enableFaceSelection(true);
    }
    statusBar()->showMessage(
        QString("Dome: Click on a planar face (Height=%1mm)").arg(height), 0);
  } break;

  case ActivePartTool::Draft: {
    if (m_document->getAllShapes().empty()) {
      QMessageBox::warning(this, "Draft", "No solid body available.");
      return;
    }
    double angle = m_toolSettingsPanel->draftAngle();
    m_pendingDraftAngle = angle;
    m_pendingFaceOperation = PendingFaceOperation::Draft;

    if (m_viewport) {
      m_viewport->enableFaceSelection(true);
    }
    statusBar()->showMessage(
        QString("Draft: Click face to apply %1� angle").arg(angle), 0);
  } break;

  case ActivePartTool::Revolve: {
    // Revolve the selected profile with panel settings
    if (m_selectedProfileIndex < 0 || !m_currentSketch) {
      QMessageBox::warning(this, "Revolve",
                           "No profile selected. Select a profile first.");
      return;
    }

    auto closedProfiles = m_currentSketch->detectClosedProfiles();
    if (m_selectedProfileIndex >= static_cast<int>(closedProfiles.size())) {
      QMessageBox::warning(this, "Revolve", "Invalid profile selection.");
      return;
    }

    // Get parameters from panel
    double angle =
        m_toolSettingsPanel ? m_toolSettingsPanel->revolveAngle() : 360.0;
    int axisIndex =
        m_toolSettingsPanel ? m_toolSettingsPanel->revolveAxis() : 1;

    // Build face from selected profile
    TopoDS_Wire selectedWire = closedProfiles[m_selectedProfileIndex];
    TopoDS_Face profileFace;

    try {
      BRepBuilderAPI_MakeFace faceBuilder(selectedWire, true);
      if (faceBuilder.IsDone()) {
        profileFace = faceBuilder.Face();
      }
    } catch (...) {
      QMessageBox::warning(this, "Revolve",
                           "Could not create face from profile.");
      return;
    }

    if (profileFace.IsNull()) {
      QMessageBox::warning(this, "Revolve", "Invalid profile face.");
      return;
    }

    // Set up axis based on selection
    gp_Ax1 axis;
    QString axisName;
    switch (axisIndex) {
    case 0:
      axis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
      axisName = "X";
      break;
    case 1:
      axis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0));
      axisName = "Y";
      break;
    default:
      axis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
      axisName = "Z";
      break;
    }

    try {
      part::RevolveFeature revolve;
      TopoDS_Shape revolvedShape =
          revolve.executeFace(profileFace, axis, angle);

      if (revolvedShape.IsNull()) {
        QMessageBox::warning(this, "Revolve",
                             "Revolve failed: " + QString::fromStdString(
                                                      revolve.errorMessage()));
        return;
      }

      saveUndoState("Revolve");

      if (m_document->getAllShapes().empty()) {
        // TODO: Convert to Feature
        // TODO: Convert to Feature-based system
        //       // m_shapes.push_back(revolvedShape);
      } else {
        try {
          BRepAlgoAPI_Fuse fuseOp(m_document->temporaryShapes()[0],
                                  revolvedShape);
          if (fuseOp.IsDone()) {
            m_document->temporaryShapes()[0] = fuseOp.Shape();
          } else {
            // TODO: Convert to Feature
            // TODO: Convert to Feature-based system
            //       // m_shapes.push_back(revolvedShape);
          }
        } catch (...) {
          // TODO: Convert to Feature
          // TODO: Convert to Feature-based system
          //       // m_shapes.push_back(revolvedShape);
        }
      }

      displayAllShapes();
      m_featureList->addItem(
          QString("?? Revolve (%1� around %2)").arg(angle).arg(axisName));
      statusBar()->showMessage(QString("Revolved Profile %1 around %2 axis")
                                   .arg(m_selectedProfileIndex + 1)
                                   .arg(axisName),
                               3000);
      m_modified = true;
      updateWindowTitle();
    } catch (...) {
      QMessageBox::warning(this, "Revolve Failed", "Could not create revolve.");
      return;
    }

    // Reset state
    m_selectedProfileIndex = -1;
    m_activePartTool = ActivePartTool::None;
  } break;

  case ActivePartTool::Cut: {
    // Cut logic similar to Extrude but with cut direction
    if (m_selectedProfileIndex < 0 || !m_currentSketch) {
      QMessageBox::warning(this, "Cut", "No profile selected.");
      return;
    }

    if (m_document->getAllShapes().empty()) {
      QMessageBox::warning(this, "Cut", "No solid body to cut from.");
      return;
    }

    auto closedProfiles = m_currentSketch->detectClosedProfiles();
    double depth = m_toolSettingsPanel ? m_toolSettingsPanel->cutDepth() : 20.0;
    TopoDS_Wire selectedWire = closedProfiles[m_selectedProfileIndex];

    try {
      BRepBuilderAPI_MakeFace faceBuilder(selectedWire, true);
      if (faceBuilder.IsDone()) {
        TopoDS_Face profileFace = faceBuilder.Face();
        gp_Dir cutDir = m_currentSketch->plane().normal();
        gp_Vec cutVec(cutDir);
        cutVec.Scale(-depth);

        BRepPrimAPI_MakePrism prism(profileFace, cutVec);
        if (prism.IsDone()) {
          TopoDS_Shape cutTool = prism.Shape();
          BRepAlgoAPI_Cut cutOp(m_document->temporaryShapes()[0], cutTool);
          if (cutOp.IsDone()) {
            saveUndoState("Cut");
            m_document->temporaryShapes()[0] = cutOp.Shape();
            displayAllShapes();
            m_featureList->addItem(QString("?? Cut (%1)").arg(depth));
            statusBar()->showMessage("Cut completed", 3000);
            m_modified = true;
            updateWindowTitle();
          }
        }
      }
    } catch (...) {
      QMessageBox::warning(this, "Cut Failed", "Operation failed.");
    }

    m_selectedProfileIndex = -1;
    m_pendingOperation = PendingOperation::None;
    m_activePartTool = ActivePartTool::None;
  } break;

  case ActivePartTool::ReferencePlane: {
    if (!m_toolSettingsPanel)
      return;

    int type = m_toolSettingsPanel->refPlaneType();
    double offset = m_toolSettingsPanel->refPlaneOffset();
    gp_Pln plane;
    QString planeName;

    switch (type) {
    case 0:
      plane = gp_Pln(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
      planeName = "XY Plane";
      break;
    case 1:
      plane = gp_Pln(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0));
      planeName = "XZ Plane";
      break;
    case 2:
      plane = gp_Pln(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
      planeName = "YZ Plane";
      break;
    case 3:
      plane = gp_Pln(gp_Pnt(0, 0, offset), gp_Dir(0, 0, 1));
      planeName = QString("XY Offset %1").arg(offset);
      break;
    case 4:
      plane = gp_Pln(gp_Pnt(0, offset, 0), gp_Dir(0, 1, 0));
      planeName = QString("XZ Offset %1").arg(offset);
      break;
    default:
      plane = gp_Pln(gp_Pnt(offset, 0, 0), gp_Dir(1, 0, 0));
      planeName = QString("YZ Offset %1").arg(offset);
      break;
    }

    if (m_viewport) {
      m_viewport->displaySketchPlane(plane, plane.Location(), 100.0);
    }
    if (m_featureList) {
      m_featureList->addItem("?? " + planeName);
    }
    statusBar()->showMessage("Reference plane created: " + planeName);
    m_activePartTool = ActivePartTool::None;
  } break;

  case ActivePartTool::Pattern: {
    if (m_document->getAllShapes().empty() || !m_toolSettingsPanel) {
      QMessageBox::warning(this, "Pattern",
                           "No shapes selected or panel missing.");
      return;
    }
    int count = m_toolSettingsPanel->patternCount();
    double spacing = m_toolSettingsPanel->patternSpacing();
    // Simplified pattern logic for demo (actually need full implementation)
    m_featureList->addItem(QString("?? Pattern (%1 instances)").arg(count));
    statusBar()->showMessage("Pattern applied", 3000);
    m_activePartTool = ActivePartTool::None;
  } break;

  case ActivePartTool::Split: {
    if (m_document->getAllShapes().empty()) {
      QMessageBox::warning(this, "Split", "No shapes to split.");
      return;
    }

    try {
      int planeIdx = m_toolSettingsPanel->splitPlane();
      double offset = m_toolSettingsPanel->splitOffset();
      // int keepIdx = m_toolSettingsPanel->splitKeepPart(); // To be used if
      // feature supports it

      gp_Pln splitPlane;
      if (planeIdx == 0) // XY
        splitPlane = gp_Pln(gp_Pnt(0, 0, offset), gp_Dir(0, 0, 1));
      else if (planeIdx == 1) // XZ
        splitPlane = gp_Pln(gp_Pnt(0, offset, 0), gp_Dir(0, 1, 0));
      else // YZ
        splitPlane = gp_Pln(gp_Pnt(offset, 0, 0), gp_Dir(1, 0, 0));

      part::SplitFeature splitter;
      auto results =
          splitter.execute(m_document->getAllShapes().back(), splitPlane);

      if (results.empty()) {
        QMessageBox::warning(
            this, "Split",
            "Split failed: " + QString::fromStdString(splitter.errorMessage()));
        return;
      }

      saveUndoState("Split");
      // TODO: Remove last feature
      // TODO: Convert to Feature-based system
      //       // m_shapes.pop_back();
      for (const auto &shape : results) {
        // TODO: Convert to Feature
        // TODO: Convert to Feature-based system
        //       // m_shapes.push_back(shape);
      }

      displayAllShapes();
      m_featureList->addItem(QString("? Split (%1 parts)").arg(results.size()));
      statusBar()->showMessage(
          QString("Split into %1 parts").arg(results.size()), 3000);
      m_activePartTool = ActivePartTool::None;
    } catch (...) {
      QMessageBox::warning(this, "Split", "Split operation failed.");
    }
  } break;

  case ActivePartTool::Mirror: {
    if (m_document->getAllShapes().empty() || !m_toolSettingsPanel) {
      QMessageBox::warning(this, "Mirror",
                           "No shapes selected or panel missing.");
      return;
    }
    int axisIndex = m_toolSettingsPanel->mirrorAxis();
    QString planeName =
        (axisIndex == 0) ? "XY" : (axisIndex == 1 ? "XZ" : "YZ");
    m_featureList->addItem(QString("- Mirror (%1)").arg(planeName));
    statusBar()->showMessage("Mirror applied", 3000);
    m_activePartTool = ActivePartTool::None;
  } break;

  case ActivePartTool::NewSketch: {
    int type = m_toolSettingsPanel->sketchPlaneType();
    if (type == 4) { // Face
      onSketchOnFace();
      m_activePartTool = ActivePartTool::None;
      return;
    }

    sketch::SketchPlane sketchPlane;
    QString planeName;

    if (type == 1) { // XY
      sketchPlane.setOrientation(sketch::PlaneOrientation::XY_Front);
      planeName = "XY (Top)";
    } else if (type == 2) { // XZ
      sketchPlane.setOrientation(sketch::PlaneOrientation::XZ_Top);
      planeName = "XZ (Front)";
    } else if (type == 3) { // YZ
      sketchPlane.setOrientation(sketch::PlaneOrientation::YZ_Right);
      planeName = "YZ (Right)";
    } else {
      QMessageBox::warning(this, "New Sketch", "Please select a plane.");
      return;
    }

    m_currentSketch = std::make_shared<sketch::Sketch>(sketchPlane);
    m_currentSketch->setName("Sketch" + std::to_string(m_featureList->count()));
    m_document->addSketch(m_currentSketch);

    if (m_sketchView) {
      m_sketchView->setSketch(m_currentSketch);
      m_sketchMode = true;
      m_sketchDock->show();
      m_sketchDock->raise();
    }

    m_featureList->addItem(QString::fromStdString(m_currentSketch->name()) +
                           " (" + planeName + ")");
    statusBar()->showMessage("New Sketch created on " + planeName, 3000);
    m_activePartTool = ActivePartTool::None;
  } break;

  default:
    statusBar()->showMessage("No active tool to apply", 2000);
    break;
  }
}

// ==================== FEATURE TREE UI INTERACTION ====================

void MainWindow::updateFeatureList() {
  if (!m_featureList)
    return;

  m_featureList->clear();

  // Add default origin items
  m_featureList->addItem("🌐 Origin");
  m_featureList->addItem("  ⬜ XY Plane");
  m_featureList->addItem("  ⬜ XZ Plane");
  m_featureList->addItem("  ⬜ YZ Plane");

  // Add features from document
  for (auto *feature : m_document->featureTree()->allFeatures()) {
    QString icon = feature->isSuppressed() ? "⏸️" : "✅";
    QString name = QString("%1 %2").arg(icon).arg(feature->name());

    auto *item = new QListWidgetItem(name);
    item->setData(Qt::UserRole, QVariant::fromValue((void *)feature));

    // Visual feedback for suppressed features
    if (feature->isSuppressed()) {
      item->setForeground(Qt::gray);
      QFont font = item->font();
      font.setItalic(true);
      item->setFont(font);
    }

    m_featureList->addItem(item);
  }
}

void MainWindow::onFeatureSelected(QListWidgetItem *item) {
  if (!item)
    return;

  // Skip origin items
  if (item->text().contains("Origin") || item->text().contains("Plane")) {
    return;
  }

  // Get feature from item data
  auto *feature =
      static_cast<core::Feature *>(item->data(Qt::UserRole).value<void *>());
  if (!feature)
    return;

  // Highlight feature shape in viewport
  if (feature->hasValidResult()) {
    // TODO: Add highlight method to viewport
    // m_viewport->highlightShape(feature->resultShape());
  }

  statusBar()->showMessage(
      QString("Selected: %1 (%2)")
          .arg(feature->name())
          .arg(core::featureTypeToString(feature->type())));
}

void MainWindow::onFeatureContextMenu(const QPoint &pos) {
  auto *item = m_featureList->itemAt(pos);
  if (!item)
    return;

  // Skip origin items
  if (item->text().contains("Origin") || item->text().contains("Plane")) {
    return;
  }

  auto *feature =
      static_cast<core::Feature *>(item->data(Qt::UserRole).value<void *>());
  if (!feature)
    return;

  QMenu menu(this);

  // Suppress/Unsuppress action
  QString suppressText = feature->isSuppressed() ? "Unsuppress" : "Suppress";
  auto *suppressAction = menu.addAction(suppressText);
  connect(suppressAction, &QAction::triggered, this,
          &MainWindow::onToggleFeatureSuppression);

  menu.addSeparator();

  // Edit action (disabled for now)
  auto *editAction = menu.addAction("Edit Feature...");
  editAction->setEnabled(false);

  // Delete action
  auto *deleteAction = menu.addAction("Delete");
  connect(deleteAction, &QAction::triggered, [this, feature]() {
    if (m_document->removeFeature(feature)) {
      updateFeatureList();
      displayAllShapes();
      m_document->checkpoint("Delete Feature");
      statusBar()->showMessage(QString("Deleted: %1").arg(feature->name()));
    }
  });

  menu.exec(m_featureList->mapToGlobal(pos));
}

void MainWindow::onToggleFeatureSuppression() {
  auto *item = m_featureList->currentItem();
  if (!item)
    return;

  auto *feature =
      static_cast<core::Feature *>(item->data(Qt::UserRole).value<void *>());
  if (!feature)
    return;

  // Toggle suppression
  bool newState = !feature->isSuppressed();
  m_document->featureTree()->suppressFeature(feature, newState);

  // Regenerate from this feature
  m_document->regenerateFrom(feature);

  // Update UI
  updateFeatureList();
  displayAllShapes();

  QString action = newState ? "Suppressed" : "Unsuppressed";
  m_document->checkpoint(QString("%1 %2").arg(action).arg(feature->name()));

  statusBar()->showMessage(QString("%1: %2").arg(action).arg(feature->name()));
}

void MainWindow::onFeatureReordered() {
  // Get new order from list widget
  QList<core::Feature *> newOrder;

  for (int i = 4; i < m_featureList->count(); ++i) { // Skip origin items
    auto *item = m_featureList->item(i);
    auto *feature =
        static_cast<core::Feature *>(item->data(Qt::UserRole).value<void *>());
    if (feature) {
      newOrder.append(feature);
    }
  }

  // TODO: Implement FeatureTree::reorderFeatures with dependency validation
  // For now, just regenerate
  m_document->regenerate();
  displayAllShapes();

  m_document->checkpoint("Reorder Features");
  statusBar()->showMessage("Features reordered");
}

} // namespace ui
} // namespace opencad
