/**
 * @file MainWindow.cpp
 * @brief Main window implementation
 */

#include "MainWindow.h"
#include "AssemblyTreeWidget.h"
#include "viewport/Viewport3D.h"
#include <QStackedWidget>

#include <BRepBndLib.hxx>
#include <BRepGProp.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <QApplication>
#include <QMessageBox>
#include <QStandardPaths>
#include <QString>
#include <QTimer>
#include <ShapeFix_Face.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>

// Core geometry
#include "core/geometry/BooleanOps.h"
#include "core/geometry/Primitives.h"

// IO
#include "io/brep/BRepReader.h"
#include "io/iges/IgesReader.h"
#include "io/mesh/StlReader.h"
#include "io/mesh/StlWriter.h"
#include "io/parasolid/ParasolidReader.h"
#include "io/solidworks/SolidWorksConverter.h"
#include "io/solidworks/SolidWorksReader.h"
#include "io/step/StepReader.h"
#include "io/step/StepWriter.h"
#include <filesystem>

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
#include "sketch/entities/SketchLine.h" // Added for Revolve axis selection
#include <QShortcut>

// Part features
#include "part/CutFeature.h"
#include "part/DomeFeature.h"
#include "part/DraftFeature.h"
#include "part/ExtrudeFeature.h"
#include "part/HoleFeature.h"
#include "part/LoftFeature.h"
#include "part/MirrorFeature.h"
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
#include "AIChatPanel.h" // Added
#include "ParameterEditor.h"
#include "ProfileSelectionPanel.h"
#include "PropertiesPanel.h"
#include "ToolSettingsPanel.h"
#include "dialogs/MateDialog.h"
#include "dialogs/NewDocumentDialog.h"

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>

// OpenCASCADE for bounding box
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
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
#include <TopLoc_Location.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Pln.hxx>
#include <gp_Trsf.hxx>

// Part features
#include "part/ChamferFeature.h"
#include "part/FilletFeature.h"
#include "part/GearFeature.h"
#include "part/PatternFeature.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Core
#include "core/Document.h"

// Assembly
#include "assembly/Assembly.h"
#include "assembly/AssemblyConstraint.h"
#include "assembly/Component.h"
#include "assembly/ComponentGroup.h"
#include "assembly/ConstraintSolver.h"

// IO
#include "io/step/StepReader.h"

#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS_Wire.hxx>

#include <BRepBuilderAPI_MakeFace.hxx>
#include <vector>

namespace {
TopoDS_Face enforceFaceHoles(const TopoDS_Face &face) {
  if (face.IsNull())
    return face;
  TopoDS_Wire outerWire = BRepTools::OuterWire(face);
  if (outerWire.IsNull())
    return face;

  TopExp_Explorer exp(face, TopAbs_WIRE);
  std::vector<TopoDS_Wire> innerWires;
  while (exp.More()) {
    TopoDS_Wire wire = TopoDS::Wire(exp.Current());
    if (!wire.IsSame(outerWire)) {
      innerWires.push_back(wire);
    }
    exp.Next();
  }

  BRepBuilderAPI_MakeFace mkFace(BRep_Tool::Surface(face), outerWire, true);
  if (!mkFace.IsDone())
    return face;

  for (const auto &w : innerWires) {
    mkFace.Add(w);
  }

  if (mkFace.IsDone()) {
    return mkFace.Face();
  }
  return face;
}
} // namespace

namespace opencad {
namespace ui {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setWindowTitle("OpenCAD");
  resize(1280, 800);

  // Create TabWidget
  m_tabWidget = new QTabWidget(this);
  m_tabWidget->setTabsClosable(true);
  setCentralWidget(m_tabWidget);

  // Connect tab signals
  connect(m_tabWidget, &QTabWidget::currentChanged, this,
          &MainWindow::onTabChanged);
  connect(m_tabWidget, &QTabWidget::tabCloseRequested, this,
          &MainWindow::onCloseTab);

  // Initialize AI Clients
  m_cqClient = std::make_unique<opencad::ai::CadQueryClient>(this);

  // Initialize AIChatPanel
  m_aiChatPanel = new AIChatPanel(this);

  // Create Dock for Chat Panel
  QDockWidget *chatDock = new QDockWidget("AI Agent", this);
  chatDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::LeftDockWidgetArea);
  chatDock->setWidget(m_aiChatPanel);
  addDockWidget(Qt::RightDockWidgetArea, chatDock);

  // Initialize LLM Client
  m_llmClient = std::make_unique<opencad::ai::LLMClient>(this);

  // Connect AIChatPanel -> MainWindow/LLM
  connect(m_aiChatPanel, &AIChatPanel::promptSubmitted, this,
          &MainWindow::onAiRun);

  // Connect LLM Client -> AIChatPanel
  connect(m_llmClient.get(), &opencad::ai::LLMClient::codeGenerated, this,
          [this](const QString &code) {
            QApplication::restoreOverrideCursor();

            // Display success in chat
            if (m_aiChatPanel) {
              m_aiChatPanel->addMessage(
                  "System", "Code generated. Building model...", false);
              m_aiChatPanel->setStatus("Building...");
            }

            statusBar()->showMessage("AI generated code. Executing...");
            qDebug() << "LLM Generated Code:" << code;

            // Execute the generated code via CadQueryClient
            if (m_cqClient) {
              QString tempDir = QStandardPaths::writableLocation(
                  QStandardPaths::TempLocation);
              QString outputPath = tempDir + "/cq_result.step";
              m_cqClient->runScript(code, outputPath);
            }
          });

  connect(m_llmClient.get(), &opencad::ai::LLMClient::errorOccurred, this,
          [this](const QString &error) {
            QApplication::restoreOverrideCursor();
            if (m_aiChatPanel) {
              m_aiChatPanel->addMessage("System", "Error: " + error, false);
              m_aiChatPanel->setStatus("Error");
            }
            statusBar()->showMessage("AI Error: " + error);
          });

  // Connect CQ Client -> AIChatPanel
  connect(
      m_cqClient.get(), &opencad::ai::CadQueryClient::executionFinished, this,
      [this](bool success, const QString &msg, const QString &stepPath) {
        QApplication::restoreOverrideCursor();
        if (success) {
          if (m_aiChatPanel) {
            m_aiChatPanel->addMessage("System", "Model built successfully.",
                                      false);
            m_aiChatPanel->setStatus("Ready");
          }
          statusBar()->showMessage("CadQuery execution successful");
          // Import the STEP file
          opencad::io::StepReader reader;
          if (reader.read(stepPath.toStdString())) {
            addShape(reader.getShape().occShape());
            displayAllShapes();
            onViewFit();
          } else {
            if (m_aiChatPanel)
              m_aiChatPanel->addMessage("System", "Failed to import STEP file.",
                                        false);
          }
        } else {
          if (m_aiChatPanel) {
            m_aiChatPanel->addMessage("System", "Build Error: " + msg, false);
            m_aiChatPanel->setStatus("Build Failed");
          }
          QMessageBox::warning(this, "CadQuery Error", msg);
        }
      });

  // Initialize CQ Editor
  m_cqEditor = new CadQueryEditorDialog(this);
  connect(m_cqEditor, &CadQueryEditorDialog::runRequested, this,
          [this](const QString &script) {
            if (m_cqClient) {
              QString tempDir = QStandardPaths::writableLocation(
                  QStandardPaths::TempLocation);
              QString outputPath = tempDir + "/cq_result.step";
              m_cqEditor->appendLog("Running script...");
              m_cqClient->runScript(script, outputPath);
              // The output/errors will be handled by the client's signals,
              // we might want to pipe them back to the editor console too?
              // For now, simpler is better.
            }
          });

  // Viewport connections are now handled in createNewTab/onTabChanged

  setupMenus();
  setupToolbars();
  setupDockWidgets();
  setupStatusBar();

  // Initial state is set by createNewTab

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

  // Create initial empty tab
  createNewTab("Untitled");
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

    // Clear visual map
    m_visualMap.clear();

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

    // Display assembly components
    if (m_document->assembly()) {
      for (const auto &component : m_document->assembly()->getComponents()) {
        if (component && component->isVisible()) {
          // Use base shape + SetLocation for performance optimization
          // This allows us to update position without re-meshing/re-displaying
          if (component->getShape()) {
            TopoDS_Shape baseShape = component->getShape()->occShape();
            if (!baseShape.IsNull()) {
              auto aisShape = m_viewport->displayShape(baseShape);
              if (!aisShape.IsNull()) {
                // Apply current placement
                m_viewport->context()->SetLocation(aisShape,
                                                   component->getPlacement());
                m_visualMap[aisShape] = component;
              }
            }
          }
        }
      }
    }

    // Display sketch wires in viewport (cyan color for visibility)
    qDebug() << "Total sketches:" << m_document->sketches().size();
    for (const auto &sketch : m_document->sketches()) {
      if (sketch && sketch->isVisible()) {
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

void MainWindow::updateAssemblyVisuals() {
  if (!m_viewport)
    return;

  auto ctx = m_viewport->context();
  if (ctx.IsNull())
    return;

  // Batch update locations without creating new objects
  for (const auto &pair : m_visualMap) {
    Handle(AIS_InteractiveObject) aisObj = pair.first;
    auto weakComp = pair.second;
    auto comp = weakComp.lock();

    if (comp && !aisObj.IsNull()) {
      TopLoc_Location loc(comp->getPlacement());
      ctx->SetLocation(aisObj, loc);
    }
  }
  ctx->UpdateCurrentViewer();
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

  auto *newAction =
      fileMenu->addAction("&New Part", this, &MainWindow::onNewFile);
  newAction->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
  newAction->setShortcut(QKeySequence::New);

  auto *newAssemblyAction =
      fileMenu->addAction("New &Assembly", this, &MainWindow::onNewAssembly);
  newAssemblyAction->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
  // Ctrl+Shift+N for New Assembly
  newAssemblyAction->setShortcut(Qt::CTRL | Qt::SHIFT | Qt::Key_N);

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
  viewMenu->addSeparator();
  m_actionSectionView =
      viewMenu->addAction("Section View", this, &MainWindow::onSectionView);
  m_actionSectionView->setCheckable(true);

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
      statusBar()->showMessage("Face selection mode - click faces to select");
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
  // featuresMenu->addAction("✨ AI Segment", this, &MainWindow::onAiSegment);
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
  createMenu->addAction("Gear", this, &MainWindow::onGear);

  // Boolean Menu
  auto *boolMenu = menuBar()->addMenu("&Boolean");
  boolMenu->addAction("Fuse (Union)", this, &MainWindow::onBooleanFuse);
  boolMenu->addAction("Cut (Difference)", this, &MainWindow::onBooleanCut);
  boolMenu->addAction("Common (Intersection)", this,
                      &MainWindow::onBooleanCommon);

  // Tools Menu
  auto *toolsMenu = menuBar()->addMenu("&Tools");
  toolsMenu->addAction("CadQuery Editor", this, [this]() {
    if (m_cqEditor) {
      m_cqEditor->show();
      m_cqEditor->raise();
      m_cqEditor->activateWindow();
    }
  });

  // Geometry Linker (User requested)
  toolsMenu->addAction("Geometry Linker", this,
                       &MainWindow::onCopyGeometryToNewPart);

  toolsMenu->addAction("Measure", this, [this]() {
    if (m_viewport) {
      statusBar()->showMessage("Measure mode not implemented yet");
    }
  });
  toolsMenu->addAction("AI Agent Chat", this, [this]() {
    if (m_aiChatPanel) {
      auto *dock = qobject_cast<QDockWidget *>(m_aiChatPanel->parentWidget());
      if (dock) {
        dock->setVisible(true);
        dock->raise();
      }
    }
  });

  // Assembly Menu
  auto *assemblyMenu = menuBar()->addMenu("&Assembly");
  assemblyMenu->addAction("New Assembly", this, &MainWindow::onNewAssembly);
  assemblyMenu->addAction("Insert Component...", this,
                          &MainWindow::onInsertComponent);
  assemblyMenu->addSeparator();
  assemblyMenu->addAction("Move Component", this, &MainWindow::onMoveComponent);
  assemblyMenu->addAction("Add Constraint", this,
                          &MainWindow::onAssemblyConstraint);
  assemblyMenu->addSeparator();
  assemblyMenu->addAction("Solve Constraints", this,
                          &MainWindow::onSolveConstraints);
  assemblyMenu->addAction("Move Multiple...", this,
                          &MainWindow::onMoveMultipleComponents);
  assemblyMenu->addAction("Group Selected...", this,
                          &MainWindow::onGroupSelectedComponents);
  assemblyMenu->addSeparator();
  assemblyMenu->addAction("Precise Move...", this,
                          &MainWindow::onParametricMove);
  assemblyMenu->addAction("Rotate...", this, &MainWindow::onRotateComponent);
  assemblyMenu->addAction("Copy...", this, &MainWindow::onCopyComponent);
  assemblyMenu->addAction("Copy to New Part...", this,
                          &MainWindow::onCopyGeometryToNewPart);
  assemblyMenu->addAction("Move to Origin", this, &MainWindow::onMoveToOrigin);
}

void MainWindow::setupToolbars() {
  // Main toolbar
  auto *mainToolbar = addToolBar("Main");
  mainToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  mainToolbar->setIconSize(QSize(24, 24));
  mainToolbar->addAction(QIcon(":/icons/file.svg"), "New", this,
                         &MainWindow::onNewFile);
  mainToolbar->addAction(QIcon(":/icons/folder-open.svg"), "Open", this,
                         &MainWindow::onOpenFile);
  mainToolbar->addAction(QIcon(":/icons/device-floppy.svg"), "Save", this,
                         &MainWindow::onSaveFile);
  mainToolbar->addSeparator();
  mainToolbar->addAction(QIcon(":/icons/maximize.svg"), "Fit", this,
                         &MainWindow::onViewFit);
  mainToolbar->addSeparator();
  if (m_actionSectionView) {
      mainToolbar->addAction(m_actionSectionView);
  }

  // Sketch toolbar
  m_sketchToolbar = addToolBar("Sketch");
  m_sketchToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  m_sketchToolbar->setIconSize(QSize(24, 24));
  m_sketchToolbar->addAction(QIcon(":/icons/pencil.svg"), "New Sketch", this,
                             &MainWindow::onNewSketch);
  m_sketchToolbar->addAction(QIcon(":/icons/check.svg"), "Finish", this,
                             &MainWindow::onFinishSketch);
  m_sketchToolbar->addSeparator();
  m_sketchToolbar->addAction(QIcon(":/icons/slash.svg"), "Line", this,
                             &MainWindow::onSketchLine);
  m_sketchToolbar->addAction(QIcon(":/icons/square.svg"), "Rect", this,
                             &MainWindow::onSketchRectangle);
  m_sketchToolbar->addAction(QIcon(":/icons/circle.svg"), "Circle", this,
                             &MainWindow::onSketchCircle);
  m_sketchToolbar->addAction(QIcon(":/icons/circle-dashed.svg"), "Arc", this,
                             &MainWindow::onSketchArc);
  m_sketchToolbar->addAction(QIcon(":/icons/point.svg"), "Point", this,
                             &MainWindow::onSketchPoint);
  m_sketchToolbar->addAction(QIcon(":/icons/scribble.svg"), "Spline", this,
                             &MainWindow::onSketchSpline);
  m_sketchToolbar->addAction(QIcon(":/icons/oval-vertical.svg"), "Ellipse",
                             this, &MainWindow::onSketchEllipse);
  m_sketchToolbar->addAction(QIcon(":/icons/hexagon.svg"), "Polygon", this,
                             &MainWindow::onSketchPolygon);
  m_sketchToolbar->addAction(QIcon(":/icons/pill.svg"), "Slot", this,
                             &MainWindow::onSketchSlot);
  m_sketchToolbar->addSeparator();
  m_sketchToolbar->addAction(QIcon(":/icons/ruler-measure.svg"), "Dimension",
                             this, &MainWindow::onSketchDimension);
  m_sketchToolbar->addSeparator();
  m_sketchToolbar->addAction(QIcon(":/icons/arrows-right.svg"), "Convert", this,
                             &MainWindow::onConvertEntities);

  // Constraint toolbar
  m_constraintToolbar = addToolBar("Constraints");
  m_constraintToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  m_constraintToolbar->setIconSize(QSize(24, 24));
  m_constraintToolbar->addAction(QIcon(":/icons/arrows-horizontal.svg"), "H",
                                 this, &MainWindow::onConstraintHorizontal);
  m_constraintToolbar->addAction(QIcon(":/icons/arrows-vertical.svg"), "V",
                                 this, &MainWindow::onConstraintVertical);
  m_constraintToolbar->addAction(QIcon(":/icons/math-perpendicular.svg"),
                                 "Perp", this,
                                 &MainWindow::onConstraintPerpendicular);
  m_constraintToolbar->addAction(QIcon(":/icons/math-equal.svg"), "Parallel",
                                 this, &MainWindow::onConstraintParallel);
  m_constraintToolbar->addAction(QIcon(":/icons/focus-2.svg"), "Coincident",
                                 this, &MainWindow::onConstraintCoincident);
  m_constraintToolbar->addAction(QIcon(":/icons/ruler-measure.svg"), "D", this,
                                 &MainWindow::onConstraintDistance);
  m_constraintToolbar->addAction(QIcon(":/icons/radius.svg"), "R", this,
                                 &MainWindow::onConstraintRadius);
  m_constraintToolbar->addAction(QIcon(":/icons/angle.svg"), "Angle", this,
                                 &MainWindow::onConstraintAngle);

  // Feature toolbar
  m_featureToolbar = addToolBar("Features");
  m_featureToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  m_featureToolbar->setIconSize(QSize(24, 24));
  m_featureToolbar->addAction(QIcon(":/icons/box-margin.svg"), "Extrude", this,
                              &MainWindow::onExtrude);
  m_featureToolbar->addAction(QIcon(":/icons/3d-rotate.svg"), "Revolve", this,
                              &MainWindow::onRevolve);
  m_featureToolbar->addAction(QIcon(":/icons/border-radius.svg"), "Fillet",
                              this, &MainWindow::onFillet);
  m_featureToolbar->addAction(QIcon(":/icons/cut.svg"), "Chamfer", this,
                              &MainWindow::onChamfer);
  m_featureToolbar->addAction(QIcon(":/icons/target.svg"), "Hole", this,
                              &MainWindow::onHoleWizard);

  // Primitives toolbar
  auto *primToolbar = addToolBar("Primitives");
  primToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  primToolbar->setIconSize(QSize(24, 24));
  primToolbar->addAction(QIcon(":/icons/box.svg"), "Box", this,
                         &MainWindow::onCreateBox);
  primToolbar->addAction(QIcon(":/icons/cylinder.svg"), "Cylinder", this,
                         &MainWindow::onCreateCylinder);
  primToolbar->addAction(QIcon(":/icons/sphere.svg"), "Sphere", this,
                         &MainWindow::onCreateSphere);

  primToolbar->addAction(QIcon(":/icons/cone.svg"), "Cone", this,
                         &MainWindow::onCreateCone);
  primToolbar->addAction(QIcon(":/icons/settings.svg"), "Gear", this,
                         &MainWindow::onGear);

  // Boolean toolbar
  auto *boolToolbar = addToolBar("Boolean");
  boolToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  boolToolbar->setIconSize(QSize(24, 24));
  boolToolbar->addAction(QIcon(":/icons/plus.svg"), "Fuse", this,
                         &MainWindow::onBooleanFuse);
  boolToolbar->addAction(QIcon(":/icons/minus.svg"), "Cut", this,
                         &MainWindow::onBooleanCut);
  boolToolbar->addAction(QIcon(":/icons/math-symbols.svg"), "Common", this,
                         &MainWindow::onBooleanCommon);

  // Selection mode toolbar
  auto *selectToolbar = addToolBar("Selection");
  selectToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  selectToolbar->setIconSize(QSize(24, 24));
  selectToolbar->addAction(QIcon(":/icons/box-model.svg"), "Shape", this,
                           &MainWindow::onSelectShape);
  selectToolbar->addAction(QIcon(":/icons/square.svg"), "Face", this,
                           &MainWindow::onSelectFace);
  selectToolbar->addAction(QIcon(":/icons/minus.svg"), "Edge", this,
                           &MainWindow::onSelectEdge);
  selectToolbar->addAction(QIcon(":/icons/point.svg"), "Vertex", this,
                           &MainWindow::onSelectVertex);

  // Assembly toolbar
  m_assemblyToolbar = addToolBar("Assembly");
  m_assemblyToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  m_assemblyToolbar->setIconSize(QSize(24, 24));
  m_assemblyToolbar->addAction(QIcon(":/icons/layout-board.svg"), "Asm", this,
                               &MainWindow::onNewAssembly);
  m_assemblyToolbar->addAction(QIcon(":/icons/file-import.svg"), "Insert", this,
                               &MainWindow::onInsertComponent);
  m_assemblyToolbar->addAction(QIcon(":/icons/hand-grab.svg"), "Move", this,
                               &MainWindow::onMoveComponent);
  m_assemblyToolbar->addAction(QIcon(":/icons/link.svg"), "Mate", this,
                               &MainWindow::onAssemblyConstraint);
  m_assemblyToolbar->addAction(QIcon(":/icons/calculator.svg"), "Solve", this,
                               &MainWindow::onSolveConstraints);
  m_assemblyToolbar->addAction(QIcon(":/icons/arrows-maximize.svg"),
                               "Multi-Move", this,
                               &MainWindow::onMoveMultipleComponents);
  m_assemblyToolbar->addAction(QIcon(":/icons/folder.svg"), "Group", this,
                               &MainWindow::onGroupSelectedComponents);
  m_assemblyToolbar->addSeparator();
  m_assemblyToolbar->addAction(QIcon(":/icons/crosshair.svg"), "Precise", this,
                               &MainWindow::onParametricMove);
  m_assemblyToolbar->addAction(QIcon(":/icons/rotate.svg"), "Rotate", this,
                               &MainWindow::onRotateComponent);
  m_assemblyToolbar->addAction(QIcon(":/icons/copy.svg"), "Copy", this,
                               &MainWindow::onCopyComponent);
  m_assemblyToolbar->addAction(QIcon(":/icons/target.svg"), "Origin", this,
                               &MainWindow::onMoveToOrigin);

  // Initially disable sketch tools (until sketch is created)
  updateSketchToolsEnabled(false);
  updateInterfaceMode();
}

void MainWindow::setupDockWidgets() {
  qDebug() << "setupDockWidgets: Start";
  // Feature Tree (left)
  m_featureTreeDock = new QDockWidget("Feature Tree", this);
  m_treeStack = new QStackedWidget(m_featureTreeDock);
  m_featureTreeDock->setWidget(m_treeStack);

  qDebug() << "setupDockWidgets: Feature Tree Created";

  // 1. Part Feature List
  m_featureList = new QListWidget(m_treeStack);
  m_featureList->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_featureList, &QListWidget::customContextMenuRequested,
          this, &MainWindow::onFeatureListContextMenu);
  m_treeStack->addWidget(m_featureList);

  qDebug() << "setupDockWidgets: Feature List Created";

  // 2. Assembly Tree Widget
  m_assemblyTree = new AssemblyTreeWidget(m_treeStack);
  qDebug() << "setupDockWidgets: AssemblyTreeWidget Created";
  // moved to onTabChanged
  m_treeStack->addWidget(m_assemblyTree);

  connect(m_assemblyTree, &AssemblyTreeWidget::componentSelected, this,
          &MainWindow::onAssemblyTreeSelection);
  connect(m_assemblyTree, &AssemblyTreeWidget::visibilityChanged, this,
          &MainWindow::displayAllShapes);
  connect(m_assemblyTree, &AssemblyTreeWidget::structChanged, this, [this]() {
    // Structure changed might assume display doesn't change,
    displayAllShapes();
  });

  addDockWidget(Qt::LeftDockWidgetArea, m_featureTreeDock);

  qDebug() << "setupDockWidgets: FeatureTreeDock Added";

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

  qDebug() << "setupDockWidgets: PropertiesDock Added";

  // Parameter Editor (left, below Feature Tree) - use Document's
  // ParameterManager
  m_parameterDock = new QDockWidget("Parameters", this);
  m_parameterEditor = new ParameterEditor(m_parameterDock);
  // moved to onTabChanged
  m_parameterDock->setWidget(m_parameterEditor);
  m_parameterDock->setMinimumWidth(280);
  addDockWidget(Qt::LeftDockWidgetArea, m_parameterDock);

  qDebug() << "setupDockWidgets: ParameterDock Added";

  // Sketch Editor (bottom/right)
  m_sketchDock = new QDockWidget("Sketch Editor", this);
  m_sketchView = new SketchView2D(m_sketchDock);
  m_sketchDock->setWidget(m_sketchView);
  m_sketchDock->setMinimumSize(500, 400);
  addDockWidget(Qt::RightDockWidgetArea, m_sketchDock);
  m_sketchDock->hide(); // Initially hidden

  qDebug() << "setupDockWidgets: SketchDock Added";

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

  qDebug() << "setupDockWidgets: ToolSettingsDock Added";

  // Profile Selection Panel (docked below Tool Settings, shown when Extrude/Cut
  // activated)
  m_profileSelectionDock = new QDockWidget("Profile Selection", this);
  m_profileSelectionPanel = new ProfileSelectionPanel(m_profileSelectionDock);
  m_profileSelectionDock->setWidget(m_profileSelectionPanel);
  m_profileSelectionDock->setMinimumWidth(280);
  addDockWidget(Qt::LeftDockWidgetArea, m_profileSelectionDock);
  m_profileSelectionDock->hide(); // Initially hidden

  qDebug() << "setupDockWidgets: ProfileSelectionDock Added";

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
              } else if (m_activePartTool == ActivePartTool::Sweep) {
                m_toolSettingsPanel->showSweepSettings();
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
  connect(m_toolSettingsPanel, &ToolSettingsPanel::settingsChanged, this,
          &MainWindow::updateExtrudePreview);

  // Feature tree connections
  m_featureList->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(m_featureList, &QListWidget::itemClicked, this,
          &MainWindow::onFeatureSelected);
  connect(m_featureList, &QListWidget::customContextMenuRequested, this,
          &MainWindow::onFeatureContextMenu);

  // ── Rollback Bar setup ───────────────────────────────────────────────────
  // Only the rollback bar is draggable; regular feature items are NOT.
  m_featureList->setDragDropMode(QAbstractItemView::InternalMove);
  m_featureList->setDefaultDropAction(Qt::MoveAction);
  m_featureList->setDragEnabled(true);
  m_featureList->setAcceptDrops(true);
  m_featureList->setDropIndicatorShown(true);

  // Mark all existing items (origin planes) as non-draggable
  for (int i = 0; i < m_featureList->count(); ++i) {
    if (auto *itm = m_featureList->item(i)) {
      itm->setFlags(itm->flags() & ~Qt::ItemIsDragEnabled);
    }
  }

  // Create the rollback bar and add it at the bottom
  m_rollbackBar = createRollbackBarItem();
  m_featureList->addItem(m_rollbackBar);

  // rowsMoved → only the rollback bar can move, so always call onRollbackBarMoved
  connect(m_featureList->model(), &QAbstractItemModel::rowsMoved, this,
          [this](const QModelIndex &/*parent*/, int srcRow, int /*srcLast*/,
                 const QModelIndex &/*dstParent*/, int dstRow) {
            onRollbackBarMoved(srcRow, dstRow);
          });

  // moved to onTabChanged

  // Connect sketch entity selection for tools (e.g. Revolve Axis, Hole Wizard)
  connect(
      m_sketchView, &SketchView2D::entitySelected, this,
      [this](sketch::SketchEntity *entity) {
        // Auto-select "Selected Line" as Revolve Axis if a line is selected
        if (m_activePartTool == ActivePartTool::Revolve &&
            m_toolSettingsPanel) {
          auto *line = dynamic_cast<sketch::SketchLine *>(entity);
          if (line) {
            // Set Revolve Axis Combo to "Selected Line" (Index 3)
            m_toolSettingsPanel->setRevolveAxis(3);
            statusBar()->showMessage(
                "Revolve Axis set to selected line. Click Apply to Finish.");
          }
        }
      });

  // Handle Hole Wizard Sketch Point selection via PointSelect tool
  connect(
      m_sketchView, &SketchView2D::pointSelected, this,
      [this](double x, double y) {
        if (m_activePartTool == ActivePartTool::HoleWizard) {
          // Convert 2D sketch point to 3D world coordinates
          gp_Pnt p3d = m_currentSketch->plane().to3D(gp_Pnt2d(x, y));

          // Toggle selection: Search if this point is already selected (with
          // small tolerance)
          bool removed = false;
          for (auto it = m_holePoints.begin(); it != m_holePoints.end(); ++it) {
            if (it->Distance(p3d) < 1e-6) {
              m_holePoints.erase(it);
              removed = true;
              break;
            }
          }

          if (!removed) {
            m_holePoints.push_back(p3d);
          }

          // Update sketch view's preview points
          std::vector<gp_Pnt2d> previewPoints;
          for (const auto &p : m_holePoints) {
            previewPoints.push_back(m_currentSketch->plane().to2D(p));
          }
          m_sketchView->setPreviewPoints(previewPoints);

          statusBar()->showMessage(
              QString(
                  "Hole Wizard: %1 point(s) selected. Click Apply when done.")
                  .arg(m_holePoints.size()));
        }
      });
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

  m_progressBar = new QProgressBar(this);
  m_progressBar->setRange(0, 100);
  m_progressBar->setValue(0);
  m_progressBar->setVisible(false);

  // User requested "green" bar
  m_progressBar->setStyleSheet(
      "QProgressBar { "
      "   border: 1px solid grey; "
      "   border-radius: 3px; "
      "   text-align: center; "
      "} "
      "QProgressBar::chunk { "
      "   background-color: #4CAF50; " // Material Green
      "   width: 10px; "
      "}");

  // Add to status bar (permanent widget so it stays on right or normal widget?)
  // Usually right side is better.
  statusBar()->addPermanentWidget(m_progressBar, 0);
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
// onNewFile moved to line 311

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
    if (m_assemblyTree) {
      m_assemblyTree->updateTree();
    }
    statusBar()->showMessage(
        QString("Undo: %1").arg(m_document->undoDescription()), 2000);

    // Update feature list (remove last item)
    if (m_featureList && m_featureList->count() > 0) {
      // TODO: improvements to feature list sync
      // delete m_featureList->takeItem(m_featureList->count() - 1);
      updateFeatureList(); // Safer to just refresh
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
    if (m_assemblyTree) {
      m_assemblyTree->updateTree();
    }
    statusBar()->showMessage(
        QString("Redo: %1").arg(m_document->redoDescription()), 2000);

    // Add feature back to list
    if (m_featureList) {
      updateFeatureList();
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

void MainWindow::onSectionView() {
  if (!m_actionSectionView || !m_viewport) return;
  bool isChecked = m_actionSectionView->isChecked();
  
  if (isChecked) {
    m_activePartTool = ActivePartTool::SectionView;
    if (m_toolSettingsDock && m_toolSettingsPanel) {
      m_toolSettingsDock->setFloating(false);
      addDockWidget(Qt::LeftDockWidgetArea, m_toolSettingsDock);
      m_toolSettingsDock->show();
      m_toolSettingsDock->raise();
      m_toolSettingsPanel->showSectionViewSettings();
    }
    updateSectionViewPreview();
  } else {
    m_activePartTool = ActivePartTool::None;
    m_viewport->setSectionView(false);
    if (m_toolSettingsDock) m_toolSettingsDock->hide();
  }
}

void MainWindow::updateSectionViewPreview() {
  if (!m_viewport || !m_toolSettingsPanel || m_activePartTool != ActivePartTool::SectionView) return;

  int planeType = m_toolSettingsPanel->sectionPlane();
  double offset = m_toolSettingsPanel->sectionOffset();
  bool flip = m_toolSettingsPanel->sectionFlip();

  gp_Dir normal;
  if (planeType == 0) { // XY (Top)
      normal = gp_Dir(0, 0, 1);
  } else if (planeType == 1) { // XZ (Front)
      normal = gp_Dir(0, 1, 0);
  } else { // YZ (Right)
      normal = gp_Dir(1, 0, 0);
  }

  if (flip) normal.Reverse();

  // Create plane passing through offset.
  // Origin is translated backwards along normal because offset pushes plane forward
  gp_Pnt origin = gp_Pnt(0, 0, 0).Translated(gp_Vec(normal).Multiplied(offset));
  gp_Pln sectionPlane(origin, normal);

  m_viewport->setSectionView(true, sectionPlane, true);
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
    // Remove original shapes, add result
    if (m_document->temporaryShapes().size() >= 2) {
      m_document->temporaryShapes().erase(
          m_document->temporaryShapes().begin(),
          m_document->temporaryShapes().begin() + 2);
      m_document->temporaryShapes().insert(
          m_document->temporaryShapes().begin(), result.occShape());
    }

    // Save state for undo AFTER modification
    saveUndoState("Boolean Fuse");

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
    // Remove original shapes, add result
    if (m_document->temporaryShapes().size() >= 2) {
      m_document->temporaryShapes().erase(
          m_document->temporaryShapes().begin(),
          m_document->temporaryShapes().begin() + 2);
      m_document->temporaryShapes().insert(
          m_document->temporaryShapes().begin(), result.occShape());
    }

    // Save state for undo AFTER modification
    saveUndoState("Boolean Cut");

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
    // Remove original shapes, add result
    if (m_document->temporaryShapes().size() >= 2) {
      m_document->temporaryShapes().erase(
          m_document->temporaryShapes().begin(),
          m_document->temporaryShapes().begin() + 2);
      m_document->temporaryShapes().insert(
          m_document->temporaryShapes().begin(), result.occShape());
    }

    // Save state for undo AFTER modification
    saveUndoState("Boolean Common");

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

  // Make sure sketch is visible when editing
  if (m_currentSketch) {
    m_currentSketch->setVisible(true);
  }

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
    m_pendingFaceOperation =
        PendingFaceOperation::SketchOnFace; // Enable sketch on face mode
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

    try {
      TopoDS_Shape baseShape;
      if (!m_document->temporaryShapes().empty()) {
        baseShape = m_document->temporaryShapes().back();
      } else {
        baseShape = m_document->getAllShapes().back();
      }

      switch (m_pendingFaceOperation) {
      case PendingFaceOperation::Dome: {
        part::DomeFeature dome;
        TopoDS_Shape result =
            dome.execute(baseShape, selectedFace, m_pendingDomeHeight);

        if (!result.IsNull()) {
          if (!m_document->temporaryShapes().empty()) {
            m_document->temporaryShapes().back() = result;
          } else {
            m_document->addTemporaryShape(result);
          }
          saveUndoState("Dome");
          displayAllShapes();
          m_featureList->addItem(
              QString("Dome (H=%1mm)").arg(m_pendingDomeHeight));
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
        part::ShellFeature shell;
        std::vector<TopoDS_Face> facesToRemove = {selectedFace};
        TopoDS_Shape result =
            shell.execute(baseShape, facesToRemove, m_pendingShellThickness);

        if (!result.IsNull()) {
          if (!m_document->temporaryShapes().empty()) {
            m_document->temporaryShapes().back() = result;
          } else {
            m_document->addTemporaryShape(result);
          }
          saveUndoState("Shell");
          displayAllShapes();
          m_featureList->addItem(
              QString("Shell (T=%1mm)").arg(m_pendingShellThickness));
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
        part::DraftFeature draft;
        std::vector<TopoDS_Face> draftFaces = {selectedFace};
        gp_Dir pullDir(0, 0, 1);
        part::DraftParams params;
        params.angle = m_pendingDraftAngle;

        TopoDS_Shape result =
            draft.execute(baseShape, draftFaces, pullDir, params);

        if (!result.IsNull()) {
          if (!m_document->temporaryShapes().empty()) {
            m_document->temporaryShapes().back() = result;
          } else {
            m_document->addTemporaryShape(result);
          }
          saveUndoState("Draft");
          displayAllShapes();
          m_featureList->addItem(
              QString("Draft (%1°)").arg(m_pendingDraftAngle));
          statusBar()->showMessage(
              QString("Draft applied: %1°").arg(m_pendingDraftAngle), 3000);
        } else {
          QMessageBox::warning(
              this, "Draft",
              "Draft failed: " + QString::fromStdString(draft.errorMessage()));
        }
        break;
      }

      case PendingFaceOperation::Thicken: {
        part::ThickenFeature thicken;
        TopoDS_Shape result =
            thicken.execute(selectedFace, m_pendingThickenValue);

        if (!result.IsNull()) {
          // Add the thickened solid to the document
          m_document->addTemporaryShape(result);
          saveUndoState("Thicken");

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
        part::OffsetSurfaceFeature offsetSurf;
        TopoDS_Shape result =
            offsetSurf.execute(selectedFace, m_pendingOffsetValue);

        if (!result.IsNull()) {
          // Add the offset surface to the document
          m_document->addTemporaryShape(result);
          saveUndoState("Offset Surface");

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

      case PendingFaceOperation::SketchOnFace: {
        // Sketch-on-face behavior with optional offset and angle
        gp_Pln plane;
        bool hasPlane = m_viewport->getSelectedFacePlane(plane);

        if (hasPlane) {
          if (m_pendingOffsetSketchDistance != 0.0) {
            gp_Vec offsetVec(plane.Axis().Direction());
            offsetVec *= m_pendingOffsetSketchDistance;
            plane.Translate(offsetVec);
          }
          if (m_pendingSketchAngle != 0.0) {
            double angleRad = m_pendingSketchAngle * M_PI / 180.0;
            // Rotate around its own local X axis
            plane.Rotate(plane.XAxis(), angleRad);
          }
          m_pendingOffsetSketchDistance = 0.0; // Reset for next use
          m_pendingSketchAngle = 0.0;

          // Create SketchPlane from the face's plane
          sketch::SketchPlane sketchPlane(plane);

          // Create new sketch with the face's plane
          m_currentSketch = std::make_shared<sketch::Sketch>(sketchPlane);
          m_currentSketch->setName("Sketch" +
                                   std::to_string(m_featureList->count() + 1));
          m_document->addSketch(m_currentSketch);

          m_sketchMode = true;
          updateSketchToolsEnabled(true);

          if (m_featureList) {
            m_featureList->addItem(
                "?? " + QString::fromStdString(m_currentSketch->name()));
          }

          showSketchEditor();
          // Switch to sketch view (normal to plane)
          if (m_viewport) {
            // Align camera to plane normal
            // This functionality might need to be exposed in Viewport3D
            // m_viewport->alignToPlane(plane);
          }

          if (m_sketchView) {
            m_sketchView->setTool(SketchToolType::Line);
          }

          statusBar()->showMessage("Sketch on face created");
          m_modified = true;
          updateWindowTitle();
        } else {
          QMessageBox::warning(
              this, "Non-Planar Face",
              "Selected face is not planar. Please select a flat face.");
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

    // Disable face selection mode AFTER the operation
    // (must be after getSelectedFacePlane() which reads m_selectedFace)
    m_viewport->enableFaceSelection(false);

    m_pendingFaceOperation = PendingFaceOperation::None;
    return;
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

void MainWindow::onSketchDimension() {
  // Auto-activate sketch mode if not already active
  if (!m_sketchMode) {
    m_sketchMode = true;
    updateSketchToolsEnabled(true);
  }
  if (m_sketchView) {
    m_sketchView->setTool(SketchToolType::Dimension);
    statusBar()->showMessage(
        "Smart Dimension: Click an entity to add a dimension constraint");
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

void MainWindow::onSketchProject() {
  if (!m_sketchMode || !m_currentSketch) {
    statusBar()->showMessage("Create or edit a sketch to use Project.");
    return;
  }

  // Check if there are already selected edges/faces
  if (m_viewport) {
    auto edges = m_viewport->getSelectedEdges();
    if (!edges.empty()) {
      // Confirmation Dialog
      auto reply =
          QMessageBox::question(this, "Convert Entities",
                                QString("Do you want to project %1 selected "
                                        "edges onto the sketch plane?")
                                    .arg(edges.size()),
                                QMessageBox::Yes | QMessageBox::No);

      if (reply == QMessageBox::No) {
        return;
      }

      int count = 0;
      for (const auto &edge : edges) {
        auto entities = m_currentSketch->addProjectedEntity(edge);
        if (!entities.empty()) {
          count += entities.size();
        }
      }

      if (count > 0) {
        statusBar()->showMessage(QString("Projected %1 entities").arg(count));
        // Refresh view
        if (m_sketchView) {
          m_sketchView->setSketch(m_currentSketch);
        }
        // Update 3D view
        TopoDS_Compound compound = m_currentSketch->buildCompound();
        if (!compound.IsNull()) {
          m_viewport->displaySketchWire(compound);
        }
        return; // Done using selection
      }
    }
  }

  m_activePartTool = ActivePartTool::Project;

  if (m_viewport) {
    // Enable edge and vertex selection
    m_viewport->enableEdgeSelection(true);
    statusBar()->showMessage(
        "Select an edge to project onto the sketch plane.");
  }
}

void MainWindow::onConvertEntities() { onSketchProject(); }

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
            m_currentSketch->saveCheckpoint("Horizontal Constraint");
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
            m_currentSketch->saveCheckpoint("Vertical Constraint");
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

  // Show Extrude settings panel
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
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

  // Detect all closed regions (faces) in sketch
  auto profileFaces = m_currentSketch->buildProfileFaces();
  qDebug() << "Detected profile regions:" << profileFaces.size();

  if (profileFaces.empty()) {
    QMessageBox::information(this, "Extrude",
                             "No closed regions found in sketch.\n"
                             "Ensure sketch curves form closed loops.");
    return;
  }

  // Set active tool
  m_pendingOperation = PendingOperation::Extrude;
  m_activePartTool = ActivePartTool::Extrude;

  // Update profile list in ProfileSelectionPanel
  if (m_profileSelectionPanel) {
    QStringList profileNames;
    for (size_t i = 0; i < profileFaces.size(); ++i) {
      profileNames << QString("Region %1").arg(i + 1);
    }
    m_profileSelectionPanel->updateProfileList(profileNames);
    m_profileSelectionPanel->setOperationTitle("Extrude");
    // Automatically select first profile so Apply works immediately
    if (!profileFaces.empty()) {
      m_profileSelectionPanel->setProfileIndex(0);
      m_selectedProfileIndex = 0;
      qDebug() << "Extrude: Auto-selected Region 1";
    }
    m_profileSelectionDock->show();
    m_profileSelectionDock->raise();
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
      QString("Click region to select (%1 "
              "available), adjust settings in panel, then click Apply")
          .arg(profileFaces.size()));
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

  // Use current sketch if available, otherwise try to find the last created
  // sketch
  std::shared_ptr<sketch::Sketch> sketchToUse = m_currentSketch;
  if (!sketchToUse && m_document && !m_document->sketches().empty()) {
    // Use the last sketch
    sketchToUse = m_document->sketches().back();
    m_currentSketch = sketchToUse; // Set it as current for this operation
  }

  if (!sketchToUse || sketchToUse->entities().empty()) {
    QMessageBox::information(this, "Revolve", "Create a sketch profile first.");
    return;
  }

  // Check for closed regions
  auto profileFaces = sketchToUse->buildProfileFaces();
  if (profileFaces.empty()) {
    QMessageBox::warning(
        this, "Revolve",
        "No closed regions found. Ensure sketch has closed loops.");
    return;
  }

  // Set active tool
  m_pendingOperation = PendingOperation::Revolve;
  m_activePartTool = ActivePartTool::Revolve;

  // Update profile list in ProfileSelectionPanel
  if (m_profileSelectionPanel) {
    QStringList profileNames;
    for (size_t i = 0; i < profileFaces.size(); ++i) {
      profileNames << QString("Region %1").arg(i + 1);
    }
    m_profileSelectionPanel->updateProfileList(profileNames);
    m_profileSelectionPanel->setOperationTitle("Revolve");
    // Auto-select first profile
    if (!profileFaces.empty()) {
      m_profileSelectionPanel->setProfileIndex(0);
      m_selectedProfileIndex = 0;
      qDebug() << "Revolve: Auto-selected Region 1";
    }
    m_profileSelectionDock->show();
    m_profileSelectionDock->raise();
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

  auto profileFaces = m_currentSketch->buildProfileFaces();
  if (profileFaces.empty()) {
    QMessageBox::warning(this, "Cut", "No closed regions found.");
    return;
  }

  m_pendingOperation = PendingOperation::Cut;
  m_activePartTool = ActivePartTool::Cut;

  if (m_profileSelectionPanel) {
    QStringList profileNames;
    for (size_t i = 0; i < profileFaces.size(); ++i) {
      profileNames << QString("Region %1").arg(i + 1);
    }
    m_profileSelectionPanel->updateProfileList(profileNames);
    m_profileSelectionPanel->setOperationTitle("Cut");
    // Automatically select first profile so Apply works immediately
    if (!profileFaces.empty()) {
      m_profileSelectionPanel->setProfileIndex(0);
      m_selectedProfileIndex = 0;
      qDebug() << "Cut: Auto-selected Region 1";
    }
    m_profileSelectionDock->show();
    m_profileSelectionDock->raise();
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
  // Show Sweep settings panel
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel) {
    m_toolSettingsPanel->showSweepSettings();
    m_toolSettingsPanel->setSweepPathText("Path: Not selected");

    // Populate the dropdown with sketches so the user can just select the full
    // sketch as a path
    QStringList sketchNames;
    if (m_document) {
      for (const auto &sketch : m_document->sketches()) {
        sketchNames << QString::fromStdString(sketch->name());
      }
    }
    m_toolSettingsPanel->populateSweepPathSketches(sketchNames);
  }

  // Reset selected profile
  m_selectedProfileIndex = -1;

  std::shared_ptr<sketch::Sketch> sketchToUse = m_currentSketch;
  if (!sketchToUse && m_document && !m_document->sketches().empty()) {
    sketchToUse = m_document->sketches().back();
    m_currentSketch = sketchToUse;
  }

  if (!sketchToUse || sketchToUse->entities().empty()) {
    QMessageBox::information(this, "Sweep", "Create a sketch profile first.");
    return;
  }

  auto profiles = sketchToUse->extractProfiles(true); // Allow open wires
  if (profiles.empty()) {
    QMessageBox::warning(this, "Sweep",
                         "No profiles found. Ensure sketch has geometry.");
    return;
  }

  m_pendingOperation = PendingOperation::Sweep;
  m_activePartTool = ActivePartTool::Sweep;

  // Setup ProfileSelectionPanel
  if (m_profileSelectionPanel) {
    QStringList profileNames;
    for (size_t i = 0; i < profiles.size(); ++i) {
      profileNames << QString("Profile %1").arg(i + 1);
    }
    m_profileSelectionPanel->updateProfileList(profileNames);
    m_profileSelectionPanel->setOperationTitle("Sweep");
    if (!profiles.empty()) {
      m_profileSelectionPanel->setProfileIndex(0);
      m_selectedProfileIndex = 0;
    }
    m_profileSelectionDock->show();
    m_profileSelectionDock->raise();
  }

  if (m_sketchView) {
    m_sketchView->enterProfileSelectMode(true); // Allow open wires
    m_sketchDock->show();
    m_sketchDock->raise();
  }

  // IMPORTANT: Raise Tool Settings Panel after showing sketch dock
  // so Apply button is visible
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }

  if (m_viewport) {
    m_viewport->enableEdgeSelection(true); // For path selection
  }

  statusBar()->showMessage("Sweep: Select Profile, click Path 3D edge or use "
                           "Sketch dropdown, then Apply");
}

void MainWindow::onLoft() {
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel) {
    m_toolSettingsPanel->showLoftSettings();

    QStringList sketchNames;
    if (m_document) {
      for (const auto &sketch : m_document->sketches()) {
        sketchNames << QString::fromStdString(sketch->name());
      }
    }
    m_toolSettingsPanel->populateLoftSketches(sketchNames);
  }

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
      if (m_document->temporaryShapes().empty()) {
        m_document->addTemporaryShape(result);
      } else {
        try {
          BRepAlgoAPI_Fuse fuseOp(m_document->temporaryShapes()[0], result);
          if (fuseOp.IsDone()) {
            m_document->temporaryShapes()[0] = fuseOp.Shape();
          } else {
            m_document->addTemporaryShape(result);
          }
        } catch (...) {
          m_document->addTemporaryShape(result);
        }
      }
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

void MainWindow::onViewportEdgeSelected() {
  if (m_activePartTool == ActivePartTool::Sweep) {
    int edgeCount = m_viewport ? m_viewport->getSelectedEdges().size() : 0;
    if (m_toolSettingsPanel) {
      if (edgeCount == 0) {
        m_toolSettingsPanel->setSweepPathText("Path: Not selected");
      } else {
        m_toolSettingsPanel->setSweepPathText(
            QString("Path: %1 edge(s) selected").arg(edgeCount));
      }
    }
  }
}

void MainWindow::onProfileSelected(int profileIndex) {
  if (!m_currentSketch || m_pendingOperation == PendingOperation::None)
    return;

  auto profileFaces =
      m_sketchView ? m_sketchView->getProfiles() : std::vector<TopoDS_Shape>();
  if (profileIndex < 0 ||
      profileIndex >= static_cast<int>(profileFaces.size())) {
    statusBar()->showMessage("Invalid region index", 2000);
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
      QString("Region %1 selected. Adjust settings in panel, then click Apply.")
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

void MainWindow::onMultiProfilesConfirmed(const std::vector<int> &selections) {
  if (selections.empty()) {
    m_selectedProfileIndex = -1;
  } else {
    m_selectedProfileIndex = selections[0]; // Use first for now
    qDebug() << "MainWindow: Multi-profile confirmed. Count="
             << selections.size() << "Using Index=" << m_selectedProfileIndex;
  }
}

void MainWindow::saveUndoState(const std::string &description) {
  // checkpoint() should be called AFTER making changes
  QStringList featureList;
  if (m_featureList) {
    for (int i = 0; i < m_featureList->count(); ++i) {
      featureList.append(m_featureList->item(i)->text());
    }
  }
  m_document->checkpoint(QString::fromStdString(description), featureList);
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
      TopoDS_Shape result = transformer.Shape();
      // Fix persistence: Update using temporaryShapes
      auto &temps = m_document->temporaryShapes();
      if (!temps.empty()) {
        temps.back() = result;
      } else {
        m_document->addTemporaryShape(result);
        // Hide feature shapes
        auto features = m_document->featureTree()->allFeatures();
        for (auto *feat : features) {
          feat->setVisible(false);
        }
      }
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

void MainWindow::clearHoleSelection() {
  m_holePoints.clear();
  if (m_sketchView) {
    m_sketchView->clearPreviewPoints();
  }
}

void MainWindow::onHoleWizard() {
  qDebug() << "HoleWizard: Action activated";
  if (m_document->getAllShapes().empty()) {
    QMessageBox::warning(this, "Hole Wizard",
                         "No shapes available. Create a solid first.");
    return;
  }

  if (m_profileSelectionDock) {
    m_profileSelectionDock->hide();
  }

  m_activePartTool = ActivePartTool::HoleWizard;
  clearHoleSelection();

  if (m_sketchMode && m_sketchView) {
    m_sketchView->setTool(SketchToolType::PointSelect);
    statusBar()->showMessage(
        "Hole Wizard: Select points from the active sketch, then click Apply",
        0);
  } else if (m_viewport) {
    m_viewport->enableVertexSelection(true);
    statusBar()->showMessage(
        "Hole Wizard: Select vertices to place the hole, then click Apply", 0);
  }

  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel) {
    qDebug() << "HoleWizard: Displaying Hole Settings Panel";
    m_toolSettingsPanel->showHoleSettings();
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
  // Show settings in tool panel
  if (m_toolSettingsDock) {
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }
  if (m_toolSettingsPanel)
    m_toolSettingsPanel->showRibSettings();

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

  m_activePartTool = ActivePartTool::Rib;
  statusBar()->showMessage("Rib: Configure settings in panel, then click Apply",
                           0);
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

void MainWindow::onGear() {
  // Show tool settings
  if (m_toolSettingsDock) {
    m_toolSettingsDock->setFloating(false);
    m_toolSettingsDock->show();
    m_toolSettingsDock->raise();
  }

  if (m_toolSettingsPanel) {
    m_toolSettingsPanel->showGearSettings();
  }

  m_activePartTool = ActivePartTool::Gear;
  statusBar()->showMessage("Adjust gear parameters and click Apply");
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
    m_currentSketch->saveCheckpoint("Mirror");
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
    m_currentSketch->saveCheckpoint("Trim");
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
    m_currentSketch->saveCheckpoint("Extend");
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

  // Switch sketch to Select mode so user can pick entities
  m_sketchView->setTool(SketchToolType::Select);

  // Set active tool — Apply will execute the pattern
  m_activePartTool = ActivePartTool::SketchLinearPattern;
  statusBar()->showMessage(
      "Select entities for linear pattern, then click Apply", 0);
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

  // Switch sketch to Select mode so user can pick entities
  m_sketchView->setTool(SketchToolType::Select);

  // Set active tool — Apply will execute the pattern
  m_activePartTool = ActivePartTool::SketchCircularPattern;
  statusBar()->showMessage(
      "Select entities for circular pattern, then click Apply", 0);
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
  m_currentSketch->saveCheckpoint("Tangent Constraint");
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
  m_currentSketch->saveCheckpoint("Equal Constraint");
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
  m_currentSketch->saveCheckpoint("Fix Constraint");
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
  m_currentSketch->saveCheckpoint("Concentric Constraint");
  m_sketchView->clearSelection();
  m_sketchView->update();
  m_featureList->addItem("? Concentric Constraint");
  statusBar()->showMessage("Concentric constraint applied", 3000);
}

void MainWindow::updateExtrudePreview() {
  if (m_activePartTool == ActivePartTool::SectionView) {
    updateSectionViewPreview();
    return;
  }

  if (m_pendingOperation == PendingOperation::None ||
      m_activePartTool != ActivePartTool::Extrude) {
    if (!m_previewShape.IsNull() && m_viewport) {
      m_viewport->context()->Remove(m_previewShape, Standard_True);
      m_previewShape.Nullify();
    }
    return;
  }

  if (!m_currentSketch || !m_sketchView || !m_viewport)
    return;

  int profileIndex = m_profileSelectionPanel
                         ? m_profileSelectionPanel->selectedProfile()
                         : m_selectedProfileIndex;

  if (profileIndex < 0)
    return;

  auto profileFaces = m_sketchView->getProfiles();
  if (profileIndex >= static_cast<int>(profileFaces.size()))
    return;

  TopoDS_Shape shape = profileFaces[profileIndex];
  if (shape.ShapeType() != TopAbs_FACE)
    return;
  TopoDS_Face profileFace = TopoDS::Face(shape);
  if (profileFace.IsNull())
    return;

  profileFace = enforceFaceHoles(profileFace);

  double depth =
      m_toolSettingsPanel ? m_toolSettingsPanel->extrudeDepth() : 20.0;
  bool symmetric =
      m_toolSettingsPanel ? m_toolSettingsPanel->extrudeSymmetric() : false;
  double draftAngle =
      m_toolSettingsPanel ? m_toolSettingsPanel->extrudeDraftAngle() : 0.0;

  gp_Dir dir = m_currentSketch->plane().normal();
  gp_Vec vec(dir);
  vec.Scale(depth);

  TopoDS_Shape previewShape;
  try {
    if (symmetric) {
      gp_Vec halfVec = vec;
      halfVec.Scale(0.5);
      gp_Vec backVec = halfVec.Reversed();
      gp_Trsf moveBack;
      moveBack.SetTranslation(backVec);
      BRepBuilderAPI_Transform transform(profileFace, moveBack, true);
      TopoDS_Face movedFace = TopoDS::Face(transform.Shape());
      BRepPrimAPI_MakePrism prism(movedFace, vec);
      if (prism.IsDone())
        previewShape = prism.Shape();
    } else {
      BRepPrimAPI_MakePrism prism(profileFace, vec);
      if (prism.IsDone())
        previewShape = prism.Shape();
    }

    if (!previewShape.IsNull() && std::abs(draftAngle) > 0.001) {
      double angleRad = draftAngle * M_PI / 180.0;
      gp_Dir draftDir = dir;
      gp_Pln neutralPlane = m_currentSketch->plane().plane();

      BRepOffsetAPI_DraftAngle draftOp(previewShape);
      TopExp_Explorer explorer(previewShape, TopAbs_FACE);
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
        previewShape = draftOp.Shape();
      }
    }
  } catch (...) {
  }

  if (!previewShape.IsNull()) {
    if (!m_previewShape.IsNull()) {
      m_viewport->context()->Remove(m_previewShape, Standard_False);
    }
    m_previewShape = new AIS_Shape(previewShape);
    m_previewShape->SetColor(Quantity_NOC_YELLOW);
    m_previewShape->SetTransparency(0.4);
    m_viewport->context()->Display(m_previewShape, Standard_False);
    m_viewport->update();
  }
}

void MainWindow::onToolApply() {
  qDebug() << "=== onToolApply() called ===";

  // Clear preview before apply
  if (!m_previewShape.IsNull() && m_viewport) {
    m_viewport->context()->Remove(m_previewShape, Standard_True);
    m_previewShape.Nullify();
  }
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

    auto profileFaces = m_sketchView ? m_sketchView->getProfiles()
                                     : std::vector<TopoDS_Shape>();
    if (profileIndex >= static_cast<int>(profileFaces.size())) {
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
    TopoDS_Face profileFace;
    if (profileIndex >= 0 &&
        profileIndex < static_cast<int>(profileFaces.size())) {
      TopoDS_Shape shape = profileFaces[profileIndex];
      if (shape.ShapeType() == TopAbs_FACE) {
        profileFace = TopoDS::Face(shape);
      }
    }

    // Verify face properties (Debug)
    if (!profileFace.IsNull()) {
      GProp_GProps props;
      BRepGProp::SurfaceProperties(profileFace, props);
      qDebug() << "Extrude: Selected Face Area:" << props.Mass();

      int wireCount = 0;
      TopExp_Explorer exp(profileFace, TopAbs_WIRE);
      while (exp.More()) {
        wireCount++;
        exp.Next();
      }
      qDebug() << "Extrude: Selected Face Wires:" << wireCount;

      try {
        FILE *f = fopen(
            "C:\\Projects\\opencadandsimulation\\opencad\\debug_topo.txt", "w");
        if (f) {
          fprintf(f, "AREA: %f\n", props.Mass());
          fprintf(f, "WIRES: %d\n", wireCount);
          TopExp_Explorer wexp(profileFace, TopAbs_WIRE);
          while (wexp.More()) {
            TopoDS_Wire w = TopoDS::Wire(wexp.Current());
            fprintf(f, "  WIRE ORIENTATION: %d\n", (int)w.Orientation());
            TopExp_Explorer eexp(w, TopAbs_EDGE);
            int edges = 0;
            while (eexp.More()) {
              edges++;
              eexp.Next();
            }
            fprintf(f, "  EDGES: %d\n", edges);
            wexp.Next();
          }
          fclose(f);
        }
      } catch (...) {
      }
    }

    profileFace = enforceFaceHoles(profileFace);

    try {
      if (!profileFace.IsNull()) {
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

        // Use sketch plane as neutral plane and extrusion direction as draft
        // direction
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

      // Store FeatureRecord for Edit Feature
      {
        core::FeatureRecord record;
        record.type = "Extrude";
        record.parameters["depth"] = depth;
        record.parameters["symmetric"] = symmetric;
        record.parameters["draftAngle"] = draftAngle;
        record.profileIndex = profileIndex;
        record.sketchIndex = findSketchIndex(m_currentSketch);
        // baseShape = shape BEFORE this extrude was applied
        // (we already saved it in undo, use the shape before fuse)
        record.baseShape = TopoDS_Shape(); // Will be populated from undo stack
        if (m_document->temporaryShapes().size() > 0) {
          // The base shape for next feature is the current result
          record.baseShape = m_document->temporaryShapes()[0];
        }
        // Insert feature at rollback position (or append if no rollback).
        // This makes new features appear BEFORE the rolled-back features,
        // matching SolidWorks behaviour.
        if (m_rollbackPosition >= 0 && m_rollbackPosition <= m_featureRecords.size()) {
          m_featureRecords.insert(m_rollbackPosition, record);
          ++m_rollbackPosition; // bar advances past the newly added feature
        } else {
          m_featureRecords.append(record);
        }
      }

      // Hide the consumed sketch
      if (m_currentSketch) {
        m_currentSketch->setVisible(false);
      }

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
      // If rollback is active, rebuild the full list from m_featureRecords
      // so the new feature appears at the correct position (before bar).
      // Otherwise just do a fast single-item insert.
      if (m_rollbackPosition >= 0) {
        replayFeaturesFrom(0);
      } else {
        addFeatureListItem(QString("\u2705 Extrude (%1)%2%3")
                               .arg(depth)
                               .arg(symInfo)
                               .arg(draftInfo));
      }
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

  case ActivePartTool::HoleWizard: {
    qDebug() << "HoleWizard: Entering case with" << m_holePoints.size()
             << "points";
    if (m_holePoints.empty()) {
      QMessageBox::warning(this, "Hole Wizard",
                           "Please select at least one point to place a hole.");
      return;
    }
    if (m_document->getAllShapes().empty()) {
      QMessageBox::warning(this, "Hole Wizard", "No solid to cut.");
      return;
    }

    double diameter =
        m_toolSettingsPanel ? m_toolSettingsPanel->holeDiameter() : 10.0;
    double depthInput =
        m_toolSettingsPanel ? m_toolSettingsPanel->holeDepth() : 20.0;
    bool flipDirection =
        m_toolSettingsPanel ? m_toolSettingsPanel->holeFlipDirection() : false;
    // int holeType = m_toolSettingsPanel ? m_toolSettingsPanel->holeType() : 0;
    // // Unused for now

    TopoDS_Shape currentSolid = m_document->getAllShapes().back();
    bool holeCreated = false;

    for (const gp_Pnt &p3d : m_holePoints) {
      gp_Dir normal(0, 0, -1);
      if (m_sketchMode && m_currentSketch) {
        normal = m_currentSketch->plane().normal().Reversed();
      }

      double depth = std::abs(depthInput);
      if (depthInput < 0.0 || flipDirection) {
        normal.Reverse();
      }

      try {
        qDebug() << "HoleWizard: Processing point at (" << p3d.X() << ","
                 << p3d.Y() << "," << p3d.Z() << ")";
        qDebug() << "HoleWizard: Normal vector: (" << normal.X() << ","
                 << normal.Y() << "," << normal.Z() << ")";
        qDebug() << "HoleWizard: Diameter:" << diameter << "Depth:" << depth;
        // Offset the point slightly backwards against the normal to avoid
        // coplanar face issues in Boolean cut
        gp_Vec offsetVec(normal);
        offsetVec.Reverse();
        offsetVec.Multiply(1.0); // Offset by 1mm
        gp_Pnt startPnt = p3d.Translated(offsetVec);

        qDebug() << "HoleWizard: offset startPnt at (" << startPnt.X() << ","
                 << startPnt.Y() << "," << startPnt.Z() << ")";

        gp_Ax2 axis(startPnt, normal);
        qDebug() << "HoleWizard: axis direction(" << axis.Direction().X() << ","
                 << axis.Direction().Y() << "," << axis.Direction().Z() << ")";

        BRepPrimAPI_MakeCylinder holeCyl(axis, diameter / 2.0, depth + 1.0);

        TopoDS_Shape cylShape = holeCyl.Shape();
        qDebug() << "HoleWizard: Cylinder tool built successfully";

        BRepAlgoAPI_Cut cutter(currentSolid, cylShape);
        if (cutter.IsDone()) {
          currentSolid = cutter.Shape();
          holeCreated = true;
          qDebug() << "HoleWizard: Boolean cut successful";
        } else {
          qDebug() << "HoleWizard: ERROR - Boolean cut failed";
        }
      } catch (const Standard_Failure &sf) {
        qDebug() << "HoleWizard: OCC Exception evaluating point:"
                 << sf.GetMessageString();
      } catch (const std::exception &e) {
        qDebug() << "HoleWizard: C++ Exception evaluating point:" << e.what();
      } catch (...) {
        qDebug() << "HoleWizard: Failed to cut hole at a point due to unknown "
                    "exception";
      }
    }

    if (holeCreated) {
      // Like Boolean Cut, remove the old solid and replace it with the new one
      // Since HoleWizard operates on m_document->getAllShapes().back() which is
      // likely in temporaryShapes
      m_document->clearTemporaryShapes();
      m_document->addTemporaryShape(currentSolid);

      // Hide all features just in case
      auto features = m_document->featureTree()->allFeatures();
      for (auto *feat : features) {
        feat->setVisible(false);
      }

      displayAllShapes();

      if (m_viewport) {
        m_viewport->update();
      }

      saveUndoState("Hole Wizard D" + std::to_string(diameter));
      m_featureList->addItem(QString("Hole Wizard (D%1 x %2) [%3]")
                                 .arg(diameter)
                                 .arg(depthInput)
                                 .arg(m_holePoints.size()));
      statusBar()->showMessage(
          QString("Created %1 hole(s)").arg(m_holePoints.size()), 3000);
      m_modified = true;
      updateWindowTitle();
    } else {
      QMessageBox::warning(this, "Hole Wizard", "Failed to create holes.");
    }

    // Reset state
    clearHoleSelection();
    if (m_sketchMode && m_sketchView) {
      m_sketchView->setTool(SketchToolType::Select);
      m_sketchView->clearSelection();
      onFinishSketch();
    } else if (m_viewport) {
      m_viewport->setSelectionMode(SelectionMode::Shape);
      m_viewport->clearSelection();
    }
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

  case ActivePartTool::Sweep: {
    std::shared_ptr<sketch::Sketch> sketchToUse = m_currentSketch;
    if (!sketchToUse && m_document && !m_document->sketches().empty()) {
      sketchToUse = m_document->sketches().back();
    }

    if (!sketchToUse) {
      QMessageBox::warning(this, "Sweep", "No sketch found for profile.");
      return;
    }

    auto profileFaces = sketchToUse->extractProfiles(true);
    int profileIndex = m_profileSelectionPanel
                           ? m_profileSelectionPanel->selectedProfile()
                           : m_selectedProfileIndex;

    if (profileIndex < 0 ||
        profileIndex >= static_cast<int>(profileFaces.size())) {
      QMessageBox::warning(this, "Sweep", "Please select a Profile region.");
      return;
    }

    TopoDS_Shape profileShape = profileFaces[profileIndex];
    TopoDS_Wire pathWire;

    // Check if user selected a Path sketch via ComboBox instead of 3D edge
    // clicks
    int pathIndex =
        m_toolSettingsPanel ? m_toolSettingsPanel->sweepPathSketchIndex() : -1;
    if (pathIndex >= 0 && m_document &&
        pathIndex < static_cast<int>(m_document->sketches().size())) {
      pathWire = m_document->sketches()[pathIndex]->buildWire();
    } else if (m_viewport) {
      // Check for manually selected 3D edges
      auto edges = m_viewport->getSelectedEdges();
      if (!edges.empty()) {
        BRepBuilderAPI_MakeWire wireBuilder;
        for (const auto &edge : edges) {
          wireBuilder.Add(edge);
        }
        if (wireBuilder.IsDone()) {
          pathWire = wireBuilder.Wire();
        }
      }
    }

    if (pathWire.IsNull()) {
      QMessageBox::warning(
          this, "Sweep",
          "You must select a Path for the Sweep operation.\n\n"
          "1. Go to the Tool Settings panel (usually on the right).\n"
          "2. Use the 'Path' dropdown to select the sketch containing your "
          "path.\n"
          "Note: The profile and path must be in different sketches.");
      return;
    }

    bool createSolid =
        m_toolSettingsPanel ? m_toolSettingsPanel->sweepSolid() : true;

    try {
      part::SweepFeature sweep;
      bool isClosed = pathWire.Closed();
      TopoDS_Shape sweptShape = sweep.execute(profileShape, pathWire, isClosed);

      if (sweptShape.IsNull()) {
        QMessageBox::warning(this, "Sweep Failed",
                             QString::fromStdString(sweep.errorMessage()));
        return;
      }

      saveUndoState("Sweep");

      if (m_document->temporaryShapes().empty()) {
        m_document->addTemporaryShape(sweptShape);
      } else {
        try {
          BRepAlgoAPI_Fuse fuseOp(m_document->temporaryShapes()[0], sweptShape);
          if (fuseOp.IsDone()) {
            m_document->temporaryShapes()[0] = fuseOp.Shape();
          } else {
            m_document->addTemporaryShape(sweptShape);
          }
        } catch (...) {
          m_document->addTemporaryShape(sweptShape);
        }
      }

      // Hide the consumed sketch
      if (m_currentSketch) {
        m_currentSketch->setVisible(false);
      }

      displayAllShapes();

      if (m_viewport) {
        m_viewport->enableEdgeSelection(false);
        m_viewport->clearSelectedEdges();
        m_viewport->update();
      }

      if (m_featureList) {
        m_featureList->addItem("✅ Sweep");
      }
      statusBar()->showMessage("Sweep completed", 3000);
      m_modified = true;
      updateWindowTitle();

    } catch (...) {
      QMessageBox::warning(this, "Sweep Failed", "Exception during sweep.");
      return;
    }

    // Reset state
    m_selectedProfileIndex = -1;
    m_pendingOperation = PendingOperation::None;
    m_activePartTool = ActivePartTool::None;
  } break;

  case ActivePartTool::Revolve: {
    std::vector<int> selectedIndices;
    if (m_sketchView && !m_sketchView->m_selectedProfiles.empty()) {
      selectedIndices = m_sketchView->m_selectedProfiles;
    } else {
      int profileIndex = m_profileSelectionPanel
                             ? m_profileSelectionPanel->selectedProfile()
                             : m_selectedProfileIndex;
      if (profileIndex >= 0) {
        selectedIndices.push_back(profileIndex);
      }
    }

    if (selectedIndices.empty() || !m_currentSketch) {
      QMessageBox::warning(
          this, "Revolve",
          "No profile selected. Select at least one profile first.");
      return;
    }

    auto profileFaces = m_sketchView ? m_sketchView->getProfiles()
                                     : std::vector<TopoDS_Shape>();

    // Get parameters from panel
    double angle =
        m_toolSettingsPanel ? m_toolSettingsPanel->revolveAngle() : 360.0;
    int axisIndex =
        m_toolSettingsPanel ? m_toolSettingsPanel->revolveAxis() : 1;

    qDebug() << "Revolve: Processing" << selectedIndices.size() << "profiles";
    qDebug() << "Revolve: Axis Index =" << axisIndex << ", Angle =" << angle;

    std::vector<TopoDS_Face> facesToRevolve;
    Bnd_Box m_bndBox;
    m_bndBox.SetGap(0.0);

    for (int idx : selectedIndices) {
      if (idx >= 0 && idx < static_cast<int>(profileFaces.size())) {
        TopoDS_Shape shape = profileFaces[idx];
        if (shape.ShapeType() == TopAbs_FACE) {
          TopoDS_Face face = TopoDS::Face(shape);
          face = enforceFaceHoles(face);
          if (!face.IsNull()) {
            facesToRevolve.push_back(face);
            BRepBndLib::Add(face, m_bndBox);
          }
        }
      }
    }

    if (facesToRevolve.empty()) {
      QMessageBox::warning(this, "Revolve", "Invalid profile selection.");
      return;
    }

    gp_Ax1 axis;
    QString axisName;
    qDebug() << "Revolve Debug: Axis Index =" << axisIndex;

    switch (axisIndex) {
    case 0:
      axis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
      axisName = "X";
      break;
    case 1:
      axis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0));
      axisName = "Y";
      break;
    case 2:
      axis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
      axisName = "Z";
      break;
    case 3: { // Selected Line
      // Try to get selected line from sketch view
      auto *selectedEntity =
          m_sketchView ? m_sketchView->selectedEntity() : nullptr;
      auto *lineEntity = dynamic_cast<sketch::SketchLine *>(selectedEntity);

      if (lineEntity && m_currentSketch) {
        // Convert 2D points to 3D
        gp_Pnt2d p1_2d = lineEntity->startPoint();
        gp_Pnt2d p2_2d = lineEntity->endPoint();
        gp_Pnt p1 = m_currentSketch->plane().to3D(p1_2d);
        gp_Pnt p2 = m_currentSketch->plane().to3D(p2_2d);

        if (p1.Distance(p2) > 1e-6) {
          gp_Vec dirVec(p1, p2);
          axis = gp_Ax1(p1, gp_Dir(dirVec));
          axisName = "Selected Line";

          qDebug() << "Revolve Debug: Custom Axis";
          qDebug() << "  P1 (3D):" << p1.X() << p1.Y() << p1.Z();
          qDebug() << "  P2 (3D):" << p2.X() << p2.Y() << p2.Z();
          qDebug() << "  Dir:" << axis.Direction().X() << axis.Direction().Y()
                   << axis.Direction().Z();
        } else {
          QMessageBox::warning(this, "Revolve", "Selected line is too short.");
          return;
        }
      } else {
        QMessageBox::warning(
            this, "Revolve",
            "Please select a Line entity in the sketch to use as axis.");
        return;
      }
      break;
    }
    }

    // Debug: Check Profile Bounding Box
    double xmin = 0, ymin = 0, zmin = 0, xmax = 0, ymax = 0, zmax = 0;
    if (!m_bndBox.IsVoid()) {
      m_bndBox.Get(xmin, ymin, zmin, xmax, ymax, zmax);
    }
    qDebug() << "Revolve Debug: Profiles Bounding Box";
    qDebug() << "  X: " << xmin << " to " << xmax;
    qDebug() << "  Y: " << ymin << " to " << ymax;
    qDebug() << "  Z: " << zmin << " to " << zmax;
    qDebug() << "  Axis Point: " << axis.Location().X() << axis.Location().Y()
             << axis.Location().Z();
    qDebug() << "  Axis Dir:   " << axis.Direction().X() << axis.Direction().Y()
             << axis.Direction().Z();

    // Check if axis intersects profile bounding box (debug info)
    bool axisIntersects = false;
    if (std::abs(axis.Direction().Y()) > 0.99) {
      double axisX = axis.Location().X();
      if (axisX > xmin && axisX < xmax)
        axisIntersects = true;
    } else if (std::abs(axis.Direction().Z()) > 0.99) {
      double axisX = axis.Location().X();
      double axisY = axis.Location().Y();
      if (axisX > xmin && axisX < xmax && axisY > ymin && axisY < ymax)
        axisIntersects = true;
    } else if (std::abs(axis.Direction().X()) > 0.99) {
      double axisY = axis.Location().Y();
      if (axisY > ymin && axisY < ymax)
        axisIntersects = true;
    }
    qDebug() << "Revolve: Axis intersects profiles bbox?" << axisIntersects;

    try {
      part::RevolveFeature revolve;
      TopoDS_Shape combinedRevolveShape;

      for (size_t i = 0; i < facesToRevolve.size(); ++i) {
        TopoDS_Shape revolvedShape =
            revolve.executeFace(facesToRevolve[i], axis, angle);

        if (revolvedShape.IsNull()) {
          QString errMsg =
              QString("Revolve failed (Profile %1, Axis %2): %3")
                  .arg(i + 1)
                  .arg(axisName)
                  .arg(QString::fromStdString(revolve.errorMessage()));
          qDebug() << errMsg;
          QMessageBox::warning(this, "Revolve", errMsg);
          return;
        }

        if (combinedRevolveShape.IsNull()) {
          combinedRevolveShape = revolvedShape;
        } else {
          try {
            BRepAlgoAPI_Fuse fuseOp(combinedRevolveShape, revolvedShape);
            if (fuseOp.IsDone()) {
              combinedRevolveShape = fuseOp.Shape();
            } else {
              qDebug() << "Revolve: Failed to fuse multiple revolved profiles!";
            }
          } catch (...) {
            qDebug() << "Revolve: Exception fusing revolved profiles!";
          }
        }
      }

      saveUndoState("Revolve");

      // Store FeatureRecord for Edit Feature
      {
        core::FeatureRecord record;
        record.type = "Revolve";
        record.parameters["revolveAngle"] = angle;
        record.parameters["axisIndex"] = axisIndex;
        record.parameters["axisName"] = axisName;
        record.profileIndex = m_selectedProfileIndex >= 0 ? m_selectedProfileIndex : 0;
        record.sketchIndex = findSketchIndex(m_currentSketch);
        if (m_document->temporaryShapes().size() > 0) {
          record.baseShape = m_document->temporaryShapes()[0];
        }
        if (m_rollbackPosition >= 0 && m_rollbackPosition <= m_featureRecords.size()) {
          m_featureRecords.insert(m_rollbackPosition, record);
          ++m_rollbackPosition;
        } else {
          m_featureRecords.append(record);
        }
      }

      if (m_document->temporaryShapes().empty()) {
        m_document->addTemporaryShape(combinedRevolveShape);
      } else {
        try {
          BRepAlgoAPI_Fuse fuseOp(m_document->temporaryShapes()[0],
                                  combinedRevolveShape);
          if (fuseOp.IsDone()) {
            m_document->temporaryShapes()[0] = fuseOp.Shape();
          } else {
            m_document->addTemporaryShape(combinedRevolveShape);
          }
        } catch (...) {
          m_document->addTemporaryShape(combinedRevolveShape);
        }
      }

      // Hide the consumed sketch
      if (m_currentSketch) {
        m_currentSketch->setVisible(false);
      }

      displayAllShapes();
      if (m_rollbackPosition >= 0) {
        replayFeaturesFrom(0);
      } else {
        addFeatureListItem(
            QString("\U0001F504 Revolve (%1\u00B0 around %2)").arg(angle).arg(axisName));
      }
      statusBar()->showMessage(QString("Revolved %1 Profiles around %2 axis")
                                   .arg(facesToRevolve.size())
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
    int profileIndex = m_profileSelectionPanel
                           ? m_profileSelectionPanel->selectedProfile()
                           : m_selectedProfileIndex;

    // Cut logic similar to Extrude but with cut direction
    if (profileIndex < 0 || !m_currentSketch) {
      QMessageBox::warning(this, "Cut", "No profile selected.");
      return;
    }

    if (m_document->getAllShapes().empty()) {
      QMessageBox::warning(this, "Cut", "No solid body to cut from.");
      return;
    }

    auto profileFaces = m_sketchView ? m_sketchView->getProfiles()
                                     : std::vector<TopoDS_Shape>();
    double depth = m_toolSettingsPanel ? m_toolSettingsPanel->cutDepth() : 20.0;

    if (profileIndex < 0 ||
        profileIndex >= static_cast<int>(profileFaces.size())) {
      QMessageBox::warning(this, "Cut", "Invalid profile selection.");
      return;
    }

    TopoDS_Face profileFace;
    TopoDS_Shape shape = profileFaces[profileIndex];
    if (shape.ShapeType() == TopAbs_FACE) {
      profileFace = TopoDS::Face(shape);
    }

    if (!profileFace.IsNull()) {
      profileFace = enforceFaceHoles(profileFace);
      qDebug() << "Cut: Using manually enforced profileFace.";
    }

    try {
      if (!profileFace.IsNull()) {
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

            // Store FeatureRecord for Edit Feature
            {
              core::FeatureRecord record;
              record.type = "Cut";
              record.parameters["depth"] = depth;
              record.profileIndex = profileIndex;
              record.sketchIndex = findSketchIndex(m_currentSketch);
              if (m_document->temporaryShapes().size() > 0) {
                record.baseShape = m_document->temporaryShapes()[0];
              }
              if (m_rollbackPosition >= 0 && m_rollbackPosition <= m_featureRecords.size()) {
                m_featureRecords.insert(m_rollbackPosition, record);
                ++m_rollbackPosition;
              } else {
                m_featureRecords.append(record);
              }
            }

            // Hide the consumed sketch
            if (m_currentSketch) {
              m_currentSketch->setVisible(false);
            }

            displayAllShapes();
            if (m_rollbackPosition >= 0) {
              replayFeaturesFrom(0);
            } else {
              addFeatureListItem(QString("\u2702\uFE0F Cut (%1)").arg(depth));
            }
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
    bool isLinear = m_toolSettingsPanel->patternIsLinear();

    try {
      saveUndoState("Pattern");
      part::PatternFeature patternOp;
      TopoDS_Shape result;

      TopoDS_Shape baseShape;
      if (!m_document->temporaryShapes().empty()) {
        baseShape = m_document->temporaryShapes().back();
      } else {
        baseShape = m_document->getAllShapes().back();
      }

      if (isLinear) {
        // Linear pattern along X direction
        gp_Dir dir(1, 0, 0);
        result = patternOp.linearPattern(baseShape, dir, count, spacing);
      } else {
        // Circular pattern around Z axis
        double angle = m_toolSettingsPanel->patternAngle();
        gp_Ax1 axis(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
        result = patternOp.circularPattern(baseShape, axis, count, angle, true);
      }

      if (!result.IsNull()) {
        if (!m_document->temporaryShapes().empty()) {
          m_document->temporaryShapes().back() = result;
        } else {
          m_document->addTemporaryShape(result);
        }
        displayAllShapes();
        if (isLinear) {
          m_featureList->addItem(
              QString("🔢 Linear Pattern (%1x, %2mm)").arg(count).arg(spacing));
        } else {
          m_featureList->addItem(
              QString("🔢 Circular Pattern (%1x)").arg(count));
        }
        statusBar()->showMessage("Pattern applied", 3000);
        m_modified = true;
        updateWindowTitle();
      } else {
        QMessageBox::warning(
            this, "Pattern",
            "Pattern failed: " +
                QString::fromStdString(patternOp.errorMessage()));
      }
    } catch (const std::exception &e) {
      QMessageBox::warning(this, "Pattern Failed",
                           QString("Exception: %1").arg(e.what()));
    } catch (...) {
      QMessageBox::warning(this, "Pattern Failed",
                           "Unknown error during pattern operation.");
    }

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
        m_document->addTemporaryShape(shape);
      }
      displayAllShapes();

      statusBar()->showMessage("Split completed", 3000);
      m_modified = true;
      updateWindowTitle();
    } catch (...) {
      QMessageBox::warning(this, "Split Failed", "Operation failed.");
    }

    m_activePartTool = ActivePartTool::None;
  } break;

  case ActivePartTool::Gear: {
    if (!m_toolSettingsPanel)
      return;

    part::GearParams params;
    params.module = m_toolSettingsPanel->gearModule();
    params.numTeeth = m_toolSettingsPanel->gearNumTeeth();
    params.pressureAngle = m_toolSettingsPanel->gearPressureAngle();
    params.thickness = m_toolSettingsPanel->gearThickness();

    part::GearFeature gearGen;
    TopoDS_Shape gearShape = gearGen.execute(params);

    if (!gearShape.IsNull()) {
      addShape(gearShape);

      QString info =
          QString("Gear (m=%1, z=%2)").arg(params.module).arg(params.numTeeth);
      m_featureList->addItem("⚙️ " + info);
      statusBar()->showMessage(info + " created", 3000);

      m_modified = true;
      updateWindowTitle();
    } else {
      QMessageBox::warning(this, "Gear Error",
                           QString::fromStdString(gearGen.errorMessage()));
    }

    // Keep tool active for creating more gears?
    // Or close it? Normally primitives close after value set?
    // Let's keep it active but maybe user wants to exit.
    // Standard behavior: One shot -> None.
    m_activePartTool = ActivePartTool::None;
  } break;

  case ActivePartTool::Mirror: {
    if (m_document->getAllShapes().empty() || !m_toolSettingsPanel) {
      QMessageBox::warning(this, "Mirror",
                           "No shapes selected or panel missing.");
      return;
    }

    TopoDS_Shape baseShape;
    if (!m_document->temporaryShapes().empty()) {
      baseShape = m_document->temporaryShapes().back();
    } else {
      baseShape = m_document->getAllShapes().back();
    }

    int axisIndex = m_toolSettingsPanel->mirrorAxis();
    gp_Ax2 mirrorPlane;
    QString planeName;

    // Default axes
    if (axisIndex == 0) { // XY
      mirrorPlane = gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));
      planeName = "XY";
    } else if (axisIndex == 1) { // XZ
      mirrorPlane = gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0));
      planeName = "XZ";
    } else { // YZ
      mirrorPlane = gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
      planeName = "YZ";
    }

    try {
      part::MirrorFeature mirrorOp;
      TopoDS_Shape result = mirrorOp.executeAxis(baseShape, mirrorPlane, true);

      if (!result.IsNull()) {
        saveUndoState("Mirror");
        if (!m_document->temporaryShapes().empty()) {
          m_document->temporaryShapes().back() = result;
        } else {
          m_document->addTemporaryShape(result);
        }
        displayAllShapes();
        m_featureList->addItem(QString("🪞 Mirror (%1)").arg(planeName));
        statusBar()->showMessage("Mirror applied", 3000);
        m_modified = true;
        updateWindowTitle();
      } else {
        QMessageBox::warning(
            this, "Mirror Failed",
            "Could not apply mirror: " +
                QString::fromStdString(mirrorOp.errorMessage()));
      }
    } catch (...) {
      QMessageBox::warning(this, "Mirror Failed",
                           "Exception during mirror operation.");
    }

    m_activePartTool = ActivePartTool::None;
  } break;

  case ActivePartTool::NewSketch: {
    int type = m_toolSettingsPanel->sketchPlaneType();
    double offset = m_toolSettingsPanel->sketchPlaneOffsetDistance();
    double angle = m_toolSettingsPanel->sketchPlaneAngle();

    if (type == 4) { // Face
      m_pendingOffsetSketchDistance = offset;
      m_pendingSketchAngle = angle;
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

    // Apply offset and angle to the base plane
    gp_Pln basePlane = sketchPlane.plane();
    if (offset != 0.0) {
      gp_Vec offsetVec(basePlane.Axis().Direction());
      offsetVec *= offset;
      basePlane.Translate(offsetVec);
    }
    if (angle != 0.0) {
      double angleRad = angle * M_PI / 180.0;
      basePlane.Rotate(basePlane.XAxis(), angleRad);
    }
    sketchPlane = sketch::SketchPlane(basePlane);

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

  case ActivePartTool::Loft: {
    if (!m_toolSettingsPanel || !m_document) {
      QMessageBox::warning(this, "Loft", "Invalid state for loft operation.");
      return;
    }

    std::vector<int> selectedIndices =
        m_toolSettingsPanel->loftSelectedSketches();
    if (selectedIndices.size() < 2) {
      QMessageBox::warning(
          this, "Loft",
          "Please select at least 2 sketches from the Profiles list.");
      return;
    }

    bool createSolid = m_toolSettingsPanel->loftSolid();
    bool ruledSurface = m_toolSettingsPanel->loftRuled();

    std::vector<TopoDS_Wire> loftWires;
    const auto &sketches = m_document->sketches();

    for (int idx : selectedIndices) {
      if (idx >= 0 && idx < static_cast<int>(sketches.size())) {
        TopoDS_Wire wire = sketches[idx]->buildWire();
        if (wire.IsNull()) {
          QMessageBox::warning(
              this, "Loft",
              QString("Sketch '%1' does not form a valid closed profile.")
                  .arg(QString::fromStdString(sketches[idx]->name())));
          return;
        }
        loftWires.push_back(wire);
      }
    }

    try {
      part::LoftFeature loft;
      TopoDS_Shape result = loft.execute(loftWires, createSolid, ruledSurface);

      if (!result.IsNull()) {
        saveUndoState("Loft");

        if (m_document->temporaryShapes().empty()) {
          m_document->addTemporaryShape(result);
        } else {
          try {
            BRepAlgoAPI_Fuse fuseOp(m_document->temporaryShapes()[0], result);
            if (fuseOp.IsDone()) {
              m_document->temporaryShapes()[0] = fuseOp.Shape();
            } else {
              m_document->addTemporaryShape(result);
            }
          } catch (...) {
            m_document->addTemporaryShape(result);
          }
        }

        displayAllShapes();
        if (m_featureList) {
          m_featureList->addItem("✅ Loft");
        }
        statusBar()->showMessage("Loft created successfully", 3000);
        m_modified = true;
        updateWindowTitle();
      } else {
        QMessageBox::warning(this, "Loft Failed",
                             QString::fromStdString(loft.errorMessage()));
      }
    } catch (const std::exception &e) {
      QMessageBox::critical(this, "Loft Error",
                            QString("Exception: %1").arg(e.what()));
    } catch (...) {
      QMessageBox::critical(this, "Loft Error", "Unknown error occurred.");
    }

    // In many CAD tools, the tool stays active, but we can reset it here
    m_activePartTool = ActivePartTool::None;
  } break;

  case ActivePartTool::SketchLinearPattern: {
    if (!m_sketchMode || !m_currentSketch || !m_sketchView) {
      QMessageBox::warning(this, "Linear Pattern", "No active sketch.");
      m_activePartTool = ActivePartTool::None;
      return;
    }

    const auto &linSelected = m_sketchView->m_selectedEntities;
    if (linSelected.empty()) {
      QMessageBox::information(
          this, "Linear Pattern",
          "Please select entities in the sketch first, then click Apply.");
      return;
    }

    int count = m_toolSettingsPanel ? m_toolSettingsPanel->patternCount() : 3;
    double spacing =
        m_toolSettingsPanel ? m_toolSettingsPanel->patternSpacing() : 20.0;

    std::vector<sketch::SketchEntity::Ptr> entities;
    for (auto *rawPtr : linSelected) {
      for (const auto &e : m_currentSketch->entities()) {
        if (e.get() == rawPtr) {
          entities.push_back(e);
          break;
        }
      }
    }

    sketch::SketchPattern patternTool;
    auto linResult = patternTool.linearPattern(*m_currentSketch, entities, 1.0,
                                               0.0, spacing, count);

    if (linResult.success) {
      m_currentSketch->saveCheckpoint("Linear Pattern");
      m_sketchView->clearSelection();
      m_sketchView->update();
      m_featureList->addItem(
          QString("\U0001f522 Linear Pattern (%1x)").arg(count));
      statusBar()->showMessage(QString("Created %1 copies at %2mm spacing")
                                   .arg(count - 1)
                                   .arg(spacing),
                               3000);
    } else {
      QMessageBox::warning(this, "Linear Pattern",
                           QString::fromStdString(linResult.error));
    }
    m_activePartTool = ActivePartTool::None;
  } break;

  case ActivePartTool::SketchCircularPattern: {
    if (!m_sketchMode || !m_currentSketch || !m_sketchView) {
      QMessageBox::warning(this, "Circular Pattern", "No active sketch.");
      m_activePartTool = ActivePartTool::None;
      return;
    }

    const auto &circSelected = m_sketchView->m_selectedEntities;
    if (circSelected.empty()) {
      QMessageBox::information(
          this, "Circular Pattern",
          "Please select entities in the sketch first, then click Apply.");
      return;
    }

    int circCount =
        m_toolSettingsPanel ? m_toolSettingsPanel->patternCount() : 6;
    double totalAngle =
        m_toolSettingsPanel ? m_toolSettingsPanel->patternAngle() : 360.0;

    std::vector<sketch::SketchEntity::Ptr> circEntities;
    for (auto *rawPtr : circSelected) {
      for (const auto &e : m_currentSketch->entities()) {
        if (e.get() == rawPtr) {
          circEntities.push_back(e);
          break;
        }
      }
    }

    sketch::SketchPattern circPatternTool;
    sketch::CircularPatternParams circParams;
    circParams.centerX = 0.0;
    circParams.centerY = 0.0;
    circParams.count = circCount;
    circParams.totalAngle = totalAngle;
    circParams.equalSpacing = true;
    auto circResult = circPatternTool.circularPattern(*m_currentSketch,
                                                      circEntities, circParams);

    if (circResult.success) {
      m_currentSketch->saveCheckpoint("Circular Pattern");
      m_sketchView->clearSelection();
      m_sketchView->update();
      m_featureList->addItem(
          QString("\U0001f504 Circular Pattern (%1x)").arg(circCount));
      statusBar()->showMessage(
          QString("Created %1 copies around origin").arg(circCount - 1), 3000);
    } else {
      QMessageBox::warning(this, "Circular Pattern",
                           QString::fromStdString(circResult.error));
    }
    m_activePartTool = ActivePartTool::None;
  } break;

  case ActivePartTool::Rib: {
    if (m_document->getAllShapes().empty()) {
      QMessageBox::warning(this, "Rib", "No solid body available.");
      return;
    }
    if (!m_currentSketch) {
      QMessageBox::warning(this, "Rib", "No active sketch.");
      return;
    }

    // Get parameters
    double thickness = m_toolSettingsPanel->ribThickness();
    int typeIdx = m_toolSettingsPanel->ribType();
    bool symmetric = m_toolSettingsPanel->ribSymmetric();
    double angle = m_toolSettingsPanel->ribAngle();
    bool flip = m_toolSettingsPanel->ribFlipDirection();
    double draft = m_toolSettingsPanel->ribDraftAngle();

    part::RibParams params;
    params.thickness = thickness;
    params.symmetric = symmetric;
    params.angle = angle;
    params.flipDirection = flip;
    params.draftAngle = draft;

    // Map type index to enum
    switch (typeIdx) {
    case 0:
      params.type = part::RibType::Parallel;
      break;
    case 1:
      params.type = part::RibType::Normal;
      break;
    case 2:
      params.type = part::RibType::AtAngle;
      break;
    default:
      params.type = part::RibType::Parallel;
      break;
    }

    try {
      saveUndoState("Rib");
      part::RibFeature rib;
      TopoDS_Shape result = rib.execute(
          *m_currentSketch, m_document->getAllShapes().back(), params);

      if (!result.IsNull()) {
        // Fix persistence
        auto &temps = m_document->temporaryShapes();
        if (!temps.empty()) {
          temps.back() = result;
        } else {
          m_document->addTemporaryShape(result);
          // Hide feature shapes to prevent overlap
          auto features = m_document->featureTree()->allFeatures();
          for (auto *feat : features) {
            feat->setVisible(false);
          }
        }
        displayAllShapes();
        m_featureList->addItem(QString("🦴 Rib (T=%1mm)").arg(thickness));
        statusBar()->showMessage(QString("Rib created: T=%1mm").arg(thickness),
                                 3000);
        m_activePartTool = ActivePartTool::None;
      } else {
        QMessageBox::warning(this, "Rib Failed",
                             QString::fromStdString(rib.errorMessage()));
      }
    } catch (...) {
      QMessageBox::warning(this, "Rib Error", "Rib operation failed.");
    }
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

  m_rollbackBar = nullptr; // will become dangling after clear()
  m_featureList->model()->blockSignals(true);
  m_featureList->clear();

  // ── Fixed origin items (non-draggable, non-drop) ──────────────────────
  auto addOriginItm = [this](const QString &text) {
    auto *itm = new QListWidgetItem(text);
    itm->setFlags(itm->flags() & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled);
    m_featureList->addItem(itm);
  };
  addOriginItm("\U0001F310 Origin");
  addOriginItm("  \u2514 XY Plane");
  addOriginItm("  \u2514 XZ Plane");
  addOriginItm("  \u2514 YZ Plane");

  // ── Legacy feature records (Extrude / Cut / Revolve via m_featureRecords) ─
  // These are separate from the new FeatureTree system.
  if (!m_featureRecords.isEmpty()) {
    const int barPos =
        (m_rollbackPosition >= 0 && m_rollbackPosition <= m_featureRecords.size())
            ? m_rollbackPosition
            : m_featureRecords.size();

    for (int ri = 0; ri < m_featureRecords.size(); ++ri) {
      // Insert rollback bar before the first rolled-back feature.
      if (ri == barPos) {
        m_rollbackBar = createRollbackBarItem();
        m_featureList->addItem(m_rollbackBar);
      }

      const bool rolledBack = (ri >= barPos);
      auto *itm = new QListWidgetItem(m_featureRecords[ri].displayString());
      itm->setData(Qt::UserRole + 1, ri);
      itm->setFlags(itm->flags() & ~Qt::ItemIsDragEnabled);

      if (rolledBack) {
        itm->setForeground(QColor(130, 130, 130));
        QFont f = itm->font();
        f.setItalic(true);
        itm->setFont(f);
      }
      m_featureList->addItem(itm);
    }

    // Bar at the very end (all features active).
    if (barPos >= m_featureRecords.size()) {
      m_rollbackBar = createRollbackBarItem();
      m_featureList->addItem(m_rollbackBar);
    }
  } else {
    // ── New FeatureTree system features ───────────────────────────────────
    if (m_document) {
      for (auto *feature : m_document->featureTree()->allFeatures()) {
        QString icon = feature->isSuppressed() ? "\u23F8\uFE0F" : "\u2705";
        QString name = QString("%1 %2").arg(icon).arg(feature->name());

        auto *item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, QVariant::fromValue((void *)feature));
        item->setFlags(item->flags() & ~Qt::ItemIsDragEnabled);

        if (feature->isSuppressed()) {
          item->setForeground(Qt::gray);
          QFont font = item->font();
          font.setItalic(true);
          item->setFont(font);
        }
        m_featureList->addItem(item);
      }
    }

    // Rollback bar at the bottom (no legacy features, so always at end).
    m_rollbackBar = createRollbackBarItem();
    m_featureList->addItem(m_rollbackBar);
  }

  m_featureList->model()->blockSignals(false);

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

void MainWindow::onFeatureReordered(int srcRow, int dstRow) {
  // -----------------------------------------------------------------------
  // SolidWorks-style rollback: when the user drags a feature up/down in the
  // feature tree, reorder m_featureRecords to match the new list order, then
  // replay all features from scratch so the 3D model reflects the new order.
  // -----------------------------------------------------------------------

  static const int kOriginItems = 4; // Origin, XY, XZ, YZ plane rows

  if (!m_featureList || m_featureRecords.isEmpty())
    return;

  // Translate list-widget row indices to m_featureRecords indices.
  // The origin items occupy rows 0..kOriginItems-1 and are non-draggable.
  int recSrc = srcRow - kOriginItems;

  // Qt's rowsMoved dstRow is the row BEFORE which the item is inserted.
  // If moving downward Qt passes dstRow one past the final position, so we
  // adjust: actual record destination = dstRow - kOriginItems, clamped.
  int recDst = dstRow - kOriginItems;
  if (dstRow > srcRow)
    recDst -= 1; // Qt inserts before dstRow; actual pos is one less when moving down

  // Validate both indices are within the feature records range.
  if (recSrc < 0 || recSrc >= m_featureRecords.size() ||
      recDst < 0 || recDst >= m_featureRecords.size() ||
      recSrc == recDst) {

    // Fallback: Strategy B — if tags exist use them; otherwise just refresh.
    QVector<core::FeatureRecord> reordered;
    bool allTagged = true;
    for (int i = kOriginItems; i < m_featureList->count(); ++i) {
      auto *item = m_featureList->item(i);
      if (!item) { allTagged = false; break; }
      QVariant roleData = item->data(Qt::UserRole + 1);
      if (!roleData.isValid()) { allTagged = false; break; }
      int origIdx = roleData.toInt();
      if (origIdx < 0 || origIdx >= m_featureRecords.size()) { allTagged = false; break; }
      reordered.append(m_featureRecords[origIdx]);
    }
    if (allTagged && reordered.size() == m_featureRecords.size()) {
      m_featureRecords = reordered;
    }
    replayFeaturesFrom(0);
    displayAllShapes();
    statusBar()->showMessage("Feature reordered – model updated", 3000);
    return;
  }

  // Move the record: remove from src, insert at dst.
  core::FeatureRecord moved = m_featureRecords.takeAt(recSrc);
  m_featureRecords.insert(recDst, moved);

  // Replay from scratch – this rebuilds the 3D model AND re-tags all items.
  saveUndoState("Reorder Feature");
  replayFeaturesFrom(0);
  displayAllShapes();
  statusBar()->showMessage(
      QString("Feature moved to position %1 – model updated").arg(recDst + 1), 3000);
}

void MainWindow::onNewAssembly() {
  createNewTab("Assembly 1");

  if (m_document) {
    m_document->setType(core::Document::Type::Assembly);
    // Force UI update since we just changed the type
    updateInterfaceMode();
  }

  statusBar()->showMessage("New Assembly created");
}

void MainWindow::onInsertComponent() {
  QString fileName = QFileDialog::getOpenFileName(
      this, "Insert Component", QString(),
      "STEP Files (*.stp *.step);;All Files (*.*)");

  if (fileName.isEmpty()) {
    return;
  }

  // Load backend (basic STEP import for now)
  // We can also allow selecting other opencad files later

  opencad::io::StepReader reader;
  if (!reader.read(fileName.toStdString())) {
    QMessageBox::critical(this, "Error",
                          QString::fromStdString(reader.errorMessage()));
    return;
  }

  auto shape = std::make_shared<core::Shape>(reader.getShape());
  auto component = std::make_shared<assembly::Component>(shape);

  // Add to assembly
  m_document->assembly()->addComponent(component);

  // Update Assembly Tree
  if (m_assemblyTree) {
    m_assemblyTree->updateTree();
  }

  displayAllShapes();
  statusBar()->showMessage("Component inserted");
}

void MainWindow::onMoveComponent() {
  m_currentAssemblyAction = AssemblyAction::Move;
  m_selectedComponents.clear();

  if (m_viewport) {
    m_viewport->enableComponentDragMode(true);
  }
  statusBar()->showMessage(
      "Move Component: Click and drag a component to move it");
}

void MainWindow::onAssemblyConstraint() {
  m_currentAssemblyAction = AssemblyAction::Constraint;
  m_selectedComponents.clear();

  // Initialize mate selection
  m_mateStep = MateStep::SelectFirst;
  m_mateShape1.Nullify();
  m_mateShape2.Nullify();
  m_mateComponent1.reset();

  if (m_viewport) {
    m_viewport->enableMateSelection(
        true); // Allow selecting faces/edges/vertices
  }
  statusBar()->showMessage("Constraint Mode: Select first face/edge/vertex");
}

void MainWindow::onMoveMultipleComponents() {
  if (!m_assemblyMode || !m_assemblyTree)
    return;

  auto selectedComps = m_assemblyTree->getSelectedComponents();
  if (selectedComps.size() < 2) {
    QMessageBox::warning(this, "Multi-Move",
                         "Please select at least 2 components.");
    return;
  }

  // Get translation vector from user
  bool ok;
  double dx = QInputDialog::getDouble(this, "Move Components", "X Offset:", 0,
                                      -10000, 10000, 2, &ok);
  if (!ok)
    return;
  double dy = QInputDialog::getDouble(this, "Move Components", "Y Offset:", 0,
                                      -10000, 10000, 2, &ok);
  if (!ok)
    return;
  double dz = QInputDialog::getDouble(this, "Move Components", "Z Offset:", 0,
                                      -10000, 10000, 2, &ok);
  if (!ok)
    return;

  // Apply translation to all selected components
  gp_Trsf translation;
  translation.SetTranslation(gp_Vec(dx, dy, dz));

  for (auto &comp : selectedComps) {
    gp_Trsf current = comp->getPlacement();
    current.Multiply(translation);
    comp->setPlacement(current);
  }

  displayAllShapes();
  statusBar()->showMessage(
      QString("Moved %1 components").arg(selectedComps.size()));
}

void MainWindow::onGroupSelectedComponents() {
  if (!m_assemblyMode || !m_assemblyTree || !m_document->assembly())
    return;

  auto selectedComps = m_assemblyTree->getSelectedComponents();
  if (selectedComps.size() < 2) {
    QMessageBox::warning(this, "Group Components",
                         "Please select at least 2 components to group.");
    return;
  }

  bool ok;
  QString groupName =
      QInputDialog::getText(this, "Group Components",
                            "Group Name:", QLineEdit::Normal, "New Group", &ok);
  if (!ok || groupName.isEmpty())
    return;

  // Create new group
  auto group =
      std::make_shared<assembly::ComponentGroup>(groupName.toStdString());

  // Move selected components into the group
  auto assembly = m_document->assembly();
  for (auto &comp : selectedComps) {
    // Remove from current parent
    auto parent = comp->getParent();
    if (parent && parent->isGroup()) {
      std::static_pointer_cast<assembly::ComponentGroup>(parent)->removeChild(
          comp);
    } else {
      assembly->removeNode(comp);
    }
    // Add to new group
    group->addChild(comp);
  }

  // Add group to assembly
  assembly->addNode(group);

  m_assemblyTree->updateTree();
  displayAllShapes();
  statusBar()->showMessage(QString("Grouped %1 components into '%2'")
                               .arg(selectedComps.size())
                               .arg(groupName));
}

void MainWindow::onSolveConstraints() {
  if (!m_assemblyMode || !m_document->assembly())
    return;

  bool success = m_document->assembly()->solve();
  if (success) {
    displayAllShapes();
    statusBar()->showMessage("Constraints solved successfully");
  } else {
    QMessageBox::warning(this, "Constraint Solver",
                         "Could not satisfy all constraints. Some components "
                         "may not be fully positioned.");
  }
}

void MainWindow::onParametricMove() {
  if (!m_assemblyMode || !m_assemblyTree)
    return;

  auto selectedComps = m_assemblyTree->getSelectedComponents();
  if (selectedComps.empty()) {
    QMessageBox::warning(this, "Parametric Move",
                         "Please select a component first.");
    return;
  }

  // Get X, Y, Z absolute position from user
  bool ok;
  double x = QInputDialog::getDouble(this, "Parametric Move", "X Position:", 0,
                                     -100000, 100000, 2, &ok);
  if (!ok)
    return;
  double y = QInputDialog::getDouble(this, "Parametric Move", "Y Position:", 0,
                                     -100000, 100000, 2, &ok);
  if (!ok)
    return;
  double z = QInputDialog::getDouble(this, "Parametric Move", "Z Position:", 0,
                                     -100000, 100000, 2, &ok);
  if (!ok)
    return;

  // Apply absolute position to all selected components
  for (auto &comp : selectedComps) {
    gp_Trsf newPlacement;
    newPlacement.SetTranslation(gp_Vec(x, y, z));
    comp->setPlacement(newPlacement);
  }

  displayAllShapes();
  statusBar()->showMessage(QString("Moved %1 component(s) to (%2, %3, %4)")
                               .arg(selectedComps.size())
                               .arg(x)
                               .arg(y)
                               .arg(z));
}

void MainWindow::onRotateComponent() {
  if (!m_assemblyMode || !m_assemblyTree)
    return;

  auto selectedComps = m_assemblyTree->getSelectedComponents();
  if (selectedComps.empty()) {
    QMessageBox::warning(this, "Rotate Component",
                         "Please select a component first.");
    return;
  }

  // Get rotation axis and angle
  QStringList axes = {"X", "Y", "Z"};
  bool ok;
  QString axis = QInputDialog::getItem(this, "Rotate Component",
                                       "Rotation Axis:", axes, 2, false, &ok);
  if (!ok)
    return;

  double angle = QInputDialog::getDouble(
      this, "Rotate Component", "Angle (degrees):", 0, -360, 360, 2, &ok);
  if (!ok)
    return;

  // Convert degrees to radians
  double angleRad = angle * M_PI / 180.0;

  // Create rotation around selected axis
  gp_Ax1 rotationAxis;
  if (axis == "X")
    rotationAxis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
  else if (axis == "Y")
    rotationAxis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0));
  else
    rotationAxis = gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1));

  for (auto &comp : selectedComps) {
    gp_Trsf current = comp->getPlacement();
    gp_Trsf rotation;
    rotation.SetRotation(rotationAxis, angleRad);
    current.Multiply(rotation);
    comp->setPlacement(current);
  }

  displayAllShapes();
  statusBar()->showMessage(
      QString("Rotated %1 component(s) by %2° around %3 axis")
          .arg(selectedComps.size())
          .arg(angle)
          .arg(axis));
}

void MainWindow::onCopyComponent() {
  if (!m_assemblyMode || !m_assemblyTree || !m_document->assembly())
    return;

  auto selectedComps = m_assemblyTree->getSelectedComponents();
  if (selectedComps.empty()) {
    QMessageBox::warning(this, "Copy Component",
                         "Please select a component to copy.");
    return;
  }

  // Get offset for the copy
  bool ok;
  double offsetX = QInputDialog::getDouble(
      this, "Copy Component", "X Offset for copy:", 50, -10000, 10000, 2, &ok);
  if (!ok)
    return;

  auto assembly = m_document->assembly();
  int copyCount = 0;

  for (auto &comp : selectedComps) {
    // Create a new component with the same shape
    auto newComp = std::make_shared<assembly::Component>(comp->getShape());
    newComp->setName(comp->getName() + "_copy");

    // Apply offset to placement
    gp_Trsf newPlacement = comp->getPlacement();
    gp_Trsf offset;
    offset.SetTranslation(gp_Vec(offsetX, 0, 0));
    newPlacement.Multiply(offset);
    newComp->setPlacement(newPlacement);

    assembly->addNode(newComp);
    copyCount++;
  }

  m_assemblyTree->updateTree();
  displayAllShapes();
  statusBar()->showMessage(QString("Created %1 copy(s)").arg(copyCount));
}

void MainWindow::onMoveToOrigin() {
  if (!m_assemblyMode || !m_assemblyTree)
    return;

  auto selectedComps = m_assemblyTree->getSelectedComponents();
  if (selectedComps.empty()) {
    QMessageBox::warning(this, "Move to Origin",
                         "Please select a component first.");
    return;
  }

  // Reset placement to identity (origin)
  for (auto &comp : selectedComps) {
    comp->setPlacement(gp_Trsf()); // Identity transform
  }

  displayAllShapes();
  statusBar()->showMessage(
      QString("Moved %1 component(s) to origin").arg(selectedComps.size()));
}

std::shared_ptr<assembly::Component>
MainWindow::findComponentFromShape(const TopoDS_Shape &shape) {
  if (!m_document->assembly())
    return nullptr;

  for (const auto &component : m_document->assembly()->getComponents()) {
    // In a real app, we need to map the display shape back to the component
    // For now, checks if IsEqual (this might need tracking of AIS_Shape to
    // Component)
    if (component->getTransformedShape().IsEqual(shape)) {
      return component;
    }
    // Fallback: since we might be selecting the original shape instance
    // displayed We might need a more robust mapping if transformations are
    // involved
    if (component->getShape()->occShape().IsEqual(shape)) {
      return component;
    }
  }
  return nullptr;
}

void MainWindow::onGeometrySelected(const QString &type) {
  if (!m_viewport)
    return;

  // Handle Hole Wizard generic message
  if (m_activePartTool == ActivePartTool::HoleWizard) {
    statusBar()->showMessage(
        QString("Selected: %1. Proceeding with hole wizard...").arg(type));
    // Implementation is in onShapeSelected
    return;
  }

  // Handle Project Tool Selection (Linked Geometry)
  if (m_activePartTool == ActivePartTool::Project && m_currentSketch) {
    TopoDS_Shape selectedShape;
    if (type.startsWith("Edge")) {
      auto edges = m_viewport->getSelectedEdges();
      if (!edges.empty()) {
        selectedShape = edges[0];
      }
    } else if (type == "Vertex") {
      // selectedShape = m_viewport->getSelectedVertex(); // Assuming
      // implemented
    }

    if (!selectedShape.IsNull()) {
      auto entities = m_currentSketch->addProjectedEntity(selectedShape);
      if (!entities.empty()) {
        statusBar()->showMessage(
            QString("Projected %1 entities").arg(entities.size()));
        // Refresh view
        if (m_sketchView) {
          m_sketchView->setSketch(m_currentSketch); // Trigger redraw
        }
        // Update 3D view
        TopoDS_Compound compound = m_currentSketch->buildCompound();
        if (!compound.IsNull()) {
          m_viewport->displaySketchWire(compound);
        }
      }
    }
    return;
  }

  if (type != "Shape")
    return;

  TopoDS_Shape selectedShape = m_viewport->getSelectedShape();
  if (selectedShape.IsNull())
    return;

  auto component = findComponentFromShape(selectedShape);
  if (!component) {
    statusBar()->showMessage("Selected object is not a component");
    return;
  }

  if (m_assemblyTree) {
    m_assemblyTree->selectComponent(component);
  }

  if (m_currentAssemblyAction == AssemblyAction::Move) {
    bool ok;
    double x = QInputDialog::getDouble(this, "Move X", "Delta X:", 0, -1000,
                                       1000, 2, &ok);
    if (ok) {
      double y = QInputDialog::getDouble(this, "Move Y", "Delta Y:", 0, -1000,
                                         1000, 2, &ok);
      if (ok) {
        double z = QInputDialog::getDouble(this, "Move Z", "Delta Z:", 0, -1000,
                                           1000, 2, &ok);
        if (ok) {
          gp_Trsf trsf = component->getPlacement();
          gp_Vec trans(x, y, z);
          trsf.SetTranslationPart(trsf.TranslationPart().Added(trans.XYZ()));
          component->setPlacement(trsf);

          displayAllShapes(); // Refresh view
          statusBar()->showMessage("Component moved");

          // Reset action
          m_currentAssemblyAction = AssemblyAction::None;
          m_viewport->enableShapeSelection(false);
        }
      }
    }
  } else if (m_currentAssemblyAction == AssemblyAction::Constraint) {
    m_selectedComponents.push_back(component);

    if (m_selectedComponents.size() == 1) {
      statusBar()->showMessage("Select the second component");
    } else if (m_selectedComponents.size() == 2) {
      // Use MateDialog with Preview
      opencad::ui::MateDialog dialog(m_selectedComponents, this);

      // Variables for Preview/Undo
      std::shared_ptr<assembly::AssemblyConstraint> previewConstraint = nullptr;
      std::vector<std::pair<std::shared_ptr<assembly::Component>, gp_Trsf>>
          initialTransforms;

      if (m_document && m_document->assembly()) {
        for (const auto &c : m_document->assembly()->getComponents()) {
          initialTransforms.push_back({c, c->getPlacement()});
        }
      }

      connect(&dialog, &opencad::ui::MateDialog::previewRequested, [&]() {
        auto c = dialog.getConstraint();
        if (!c || !m_document || !m_document->assembly())
          return;

        if (previewConstraint) {
          m_document->assembly()->removeConstraint(previewConstraint);
        }

        // Note: No sub-shapes in this path usually
        previewConstraint = c;
        m_document->assembly()->addConstraint(c);

        bool solved = m_document->assembly()->solve();
        if (solved)
          statusBar()->showMessage("Preview: Solved");
        else
          statusBar()->showMessage("Preview: Solver failed");

        updateAssemblyVisuals();
      });

      if (dialog.exec() == QDialog::Accepted) {
        if (previewConstraint && m_document && m_document->assembly()) {
          m_document->assembly()->removeConstraint(previewConstraint);
          previewConstraint = nullptr;
        }

        auto constraint = dialog.getConstraint();
        if (constraint) {
          m_document->assembly()->addConstraint(constraint);
          bool solved = m_document->assembly()->solve();
          if (solved) {
            statusBar()->showMessage("Constraint added and solved");
          } else {
            QMessageBox::warning(this, "Solver",
                                 "Constraint added but solver failed");
          }
          displayAllShapes();
        }
      } else {
        // Cancelled - undo preview
        if (previewConstraint && m_document && m_document->assembly()) {
          m_document->assembly()->removeConstraint(previewConstraint);
        }
        // Restore positions
        for (const auto &pair : initialTransforms) {
          pair.first->setPlacement(pair.second);
        }
        updateAssemblyVisuals();
        statusBar()->showMessage("Constraint cancelled");
      }

      // Reset
      m_selectedComponents.clear();
      m_currentAssemblyAction = AssemblyAction::None;
      m_viewport->enableShapeSelection(false);
    }
  }
}

void MainWindow::updateInterfaceMode() {
  if (m_assemblyMode) {
    if (m_featureToolbar)
      m_featureToolbar->setVisible(false);
    if (m_sketchToolbar)
      m_sketchToolbar->setVisible(false);
    if (m_constraintToolbar)
      m_constraintToolbar->setVisible(false);
    if (m_assemblyToolbar)
      m_assemblyToolbar->setVisible(true);
    if (m_featureTreeDock)
      m_featureTreeDock->setWindowTitle("Assembly Tree");
    if (m_treeStack)
      m_treeStack->setCurrentIndex(1);
  } else {
    // Part Mode
    if (m_featureToolbar)
      m_featureToolbar->setVisible(true);
    if (m_sketchToolbar)
      m_sketchToolbar->setVisible(true);
    if (m_constraintToolbar)
      m_constraintToolbar->setVisible(true);
    if (m_assemblyToolbar)
      m_assemblyToolbar->setVisible(false);
    if (m_featureTreeDock)
      m_featureTreeDock->setWindowTitle("Feature Tree");
    if (m_treeStack)
      m_treeStack->setCurrentIndex(0);
  }
}

void MainWindow::onAssemblyTreeSelection(
    std::shared_ptr<assembly::Component> component) {
  if (!m_viewport)
    return;

  if (component) {
    // Highlight the component's shape
    m_viewport->clearSelectedEdges();
    // m_viewport->clearSelectedFaces(); // Not implemented yet
    // Assuming viewport has a method to select a specific TopoDS_Shape
    // If not, we might need to rely on AIS context logic.
    // For now, we'll assume we can pass the shape.
    // m_viewport->selectShape(component->getTransformedShape());

    // Actually, Viewport3D typically uses AIS_InteractiveContext::SetSelected
    // We might need to expose a method in Viewport3D to select by
    // TopoDS_Shape For this MVP, we'll just log it to status bar if viewport
    // support is missing
    statusBar()->showMessage("Selected component: " +
                             QString::fromStdString(component->getName()));
  } else {
    m_viewport->clearSelectedEdges(); // Clear selection
  }
}

void MainWindow::onShapeSelected(const TopoDS_Shape &shape,
                                 Handle(AIS_InteractiveObject) object) {
  // Handle Hole Wizard point selection explicitly here where shape is provided
  if (m_activePartTool == ActivePartTool::HoleWizard) {
    if (shape.ShapeType() == TopAbs_VERTEX) {
      TopoDS_Vertex vertex = TopoDS::Vertex(shape);
      gp_Pnt pnt = BRep_Tool::Pnt(vertex);

      m_holePoints.push_back(pnt);
      statusBar()->showMessage(
          QString("Hole Wizard: %1 vertex selected. Click Apply when done.")
              .arg(m_holePoints.size()));
    } else {
      statusBar()->showMessage("Hole Wizard: Please select a Vertex (point)",
                               3000);
    }
    return;
  }

  auto it = m_visualMap.find(object);
  std::shared_ptr<assembly::Component> comp;

  if (it != m_visualMap.end()) {
    auto weakComp = it->second;
    comp = weakComp.lock();
    if (comp) {
      if (m_assemblyTree) {
        m_assemblyTree->selectComponent(comp);
        statusBar()->showMessage("Selected: " +
                                 QString::fromStdString(comp->getName()));
      }
    }
  } else {
    // Select background
    if (m_assemblyTree) {
      m_assemblyTree->selectComponent(nullptr);
    }
    return;
  }

  // Handle Mate Logic
  if (m_currentAssemblyAction == AssemblyAction::Constraint && comp) {
    if (m_mateStep == MateStep::SelectFirst) {
      m_mateComponent1 = comp;
      m_mateShape1 = shape;
      m_mateStep = MateStep::SelectSecond;
      statusBar()->showMessage("Selected first geometry on " +
                               QString::fromStdString(comp->getName()) +
                               ". Now select second geometry.");
    } else if (m_mateStep == MateStep::SelectSecond) {
      if (comp == m_mateComponent1) {
        statusBar()->showMessage("Error: Cannot mate a component to itself. "
                                 "Select a different component.");
        return;
      }

      m_mateShape2 = shape;

      std::vector<std::shared_ptr<assembly::Component>> selection;
      selection.push_back(m_mateComponent1);
      selection.push_back(comp);

      // Show MateDialog with Preview
      opencad::ui::MateDialog dialog(selection, this);

      // Variables for Preview/Undo
      std::shared_ptr<assembly::AssemblyConstraint> previewConstraint = nullptr;
      std::vector<std::pair<std::shared_ptr<assembly::Component>, gp_Trsf>>
          initialTransforms;

      // Capture initial state of all components (in case solver moves them)
      if (m_document && m_document->assembly()) {
        for (const auto &c : m_document->assembly()->getComponents()) {
          initialTransforms.push_back({c, c->getPlacement()});
        }
      }

      // Connect Preview Signal
      connect(&dialog, &opencad::ui::MateDialog::previewRequested, [&]() {
        auto c = dialog.getConstraint();
        if (!c || !m_document || !m_document->assembly())
          return;

        // Remove previous preview if any
        if (previewConstraint) {
          m_document->assembly()->removeConstraint(previewConstraint);
        }

        // Setup and Add
        c->setSubShapes(m_mateShape1, m_mateShape2);
        previewConstraint = c;
        m_document->assembly()->addConstraint(c);

        // Solve
        bool solved = m_document->assembly()->solve();
        if (solved) {
          statusBar()->showMessage("Preview: Solved");
        } else {
          statusBar()->showMessage("Preview: Solver failed");
        }

        updateAssemblyVisuals();
      });

      if (dialog.exec() == QDialog::Accepted) {
        // Clean up preview before adding final
        if (previewConstraint && m_document && m_document->assembly()) {
          m_document->assembly()->removeConstraint(previewConstraint);
          previewConstraint = nullptr;
        }

        auto constraint = dialog.getConstraint();
        if (constraint) {
          // Set sub-shapes
          constraint->setSubShapes(m_mateShape1, m_mateShape2);

          m_document->assembly()->addConstraint(constraint);

          // Trigger solver
          bool solved = m_document->assembly()->solve();

          if (solved) {
            statusBar()->showMessage("Constraint added and solved");
          } else {
            statusBar()->showMessage(
                "Constraint added but solver failed to fully converge");
          }

          updateAssemblyVisuals();
          if (m_assemblyTree) {
            m_assemblyTree->updateTree();
          }
        }
      } else {
        // Rejected (Cancel) - Undo Changes
        if (m_document && m_document->assembly()) {
          if (previewConstraint) {
            m_document->assembly()->removeConstraint(previewConstraint);
          }
          // Restore transforms
          for (const auto &pair : initialTransforms) {
            pair.first->setPlacement(pair.second);
          }
          updateAssemblyVisuals();
          statusBar()->showMessage("Constraint cancelled");
        }
      }

      // Reset
      m_mateStep = MateStep::SelectFirst;
      m_mateComponent1.reset();
      m_mateShape2.Nullify();
    }
  }
}

void MainWindow::onAiRun(const QString &prompt) {
  if (!m_cqClient) {
    statusBar()->showMessage("Error: CadQuery Client not initialized.");
    return;
  }

  qDebug() << "MainWindow: onAiRun called with prompt:" << prompt;

  // 1. Check for specific AI Commands (Shortcuts)
  if (prompt.contains("vida çiz", Qt::CaseInsensitive) ||
      prompt.contains("draw screw", Qt::CaseInsensitive)) {
    statusBar()->showMessage("Generating Screw (CadQuery)...");
    QApplication::setOverrideCursor(Qt::WaitCursor);

    // Predefined Screw Script
    QString script = R"(
import cadquery as cq
# Simple Bolt/Screw creation
head_radius = 5.0
head_height = 3.0
shaft_radius = 2.5
shaft_height = 15.0

# Create Head (Hexagon or Circle)
# Let's do a Hex head for 'bolt' look, or Circle for 'screw'
result = (
    cq.Workplane("XY")
    .polygon(6, head_radius * 2) # Hex head
    .extrude(head_height)
    .faces(">Z")
    .workplane()
    .circle(shaft_radius)
    .extrude(shaft_height)
)
)";
    QString tempDir =
        QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString outputPath = tempDir + "/cq_result.step";

    m_cqClient->runScript(script, outputPath);
    return;
  }

  // 2. Generic Box Command (Simplified)
  QString trimmedPrompt = prompt.trimmed().toLower();
  if (trimmedPrompt == "kutu" || trimmedPrompt == "box") {
    statusBar()->showMessage("Generating Box (CadQuery)...");
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString script = R"(
import cadquery as cq
result = cq.Workplane("XY").box(50, 50, 50)
)";
    QString tempDir =
        QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString outputPath = tempDir + "/cq_result.step";
    m_cqClient->runScript(script, outputPath);
    return;
  }

  // 3. Generic Cylinder Command (Simplified)
  if (trimmedPrompt == "silindir" || trimmedPrompt == "cylinder") {
    statusBar()->showMessage("Generating Cylinder (CadQuery)...");
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString script = R"(
import cadquery as cq
result = cq.Workplane("XY").circle(10).extrude(50)
)";
    QString tempDir =
        QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString outputPath = tempDir + "/cq_result.step";
    m_cqClient->runScript(script, outputPath);
    return;
  }

  // 4. Default: Treat as CadQuery Script or Natural Language Prompt
  // If it starts with cq or contains python-like syntax, try running it
  // directly
  bool isScript = prompt.startsWith("cq ") ||
                  prompt.contains("import cadquery") ||
                  prompt.contains("result =");
  qDebug() << "MainWindow: Checking if script. isScript =" << isScript;

  if (isScript) {
    qDebug() << "MainWindow: Detected as CadQuery script. Executing directly.";
    statusBar()->showMessage("Running CadQuery Script...");
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString tempDir =
        QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString outputPath = tempDir + "/cq_result.step";

    m_cqClient->runScript(prompt, outputPath);
    return;
  }

  // 5. Fallback: Send to LLM for Code Generation
  qDebug() << "MainWindow: Falling back to LLM generation.";
  statusBar()->showMessage(
      "Asking AI to generate code... (this may take a few seconds)");
  QApplication::setOverrideCursor(Qt::WaitCursor);

  if (m_llmClient) {
    qDebug() << "MainWindow: Calling LLMClient::generateCode";
    m_llmClient->generateCode(prompt);
  } else {
    QApplication::restoreOverrideCursor();
    QMessageBox::critical(this, "Error", "LLM Chat Client not initialized.");
  }
}

// ============================================================================
// Multi-Document Support
// ============================================================================

std::shared_ptr<core::Document> MainWindow::createNewTab(const QString &title) {

  // Create viewport

  auto viewport = new Viewport3D(this);

  // Create document

  auto doc = std::make_shared<core::Document>(this);
  doc->setName(title);
  doc->newDocument(); // Initialize

  // Map viewport to document
  m_documentMap[viewport] = doc;

  // Add tab

  int index = m_tabWidget->addTab(viewport, title);
  m_tabWidget->setCurrentIndex(index);

  // Connect viewport signals

  connect(viewport, &Viewport3D::faceSelected, this,
          &MainWindow::onFaceSelected);
  connect(viewport, &Viewport3D::geometrySelected, this,
          &MainWindow::onGeometrySelected);
  connect(viewport, &Viewport3D::shapeSelected, this,
          &MainWindow::onShapeSelected);
  connect(viewport, &Viewport3D::edgeSelected, this,
          &MainWindow::onViewportEdgeSelected);

  // Connect component drag signal
  connect(
      viewport, &Viewport3D::componentDragEnded, this,
      [this, viewport](Handle(AIS_InteractiveObject) aisObj, gp_Pnt dropPoint) {
        if (!m_assemblyMode || !m_document || !m_document->assembly())
          return;

        // Find the component from the AIS object
        auto it = m_visualMap.find(aisObj);
        if (it != m_visualMap.end()) {
          auto compWeak = it->second;
          if (auto comp = compWeak.lock()) {
            // Calculate translation from original position
            gp_Trsf currentPlacement = comp->getPlacement();
            gp_Trsf newPlacement;
            newPlacement.SetTranslation(gp_Vec(gp_Pnt(0, 0, 0), dropPoint));
            comp->setPlacement(newPlacement);

            displayAllShapes();
            statusBar()->showMessage(
                QString("Moved component '%1'")
                    .arg(QString::fromStdString(comp->getName())));
          }
        }

        // Disable drag mode after drop
        viewport->enableComponentDragMode(false);
        m_currentAssemblyAction = AssemblyAction::None;
      });

  // Connect document signals
  connect(doc.get(), &core::Document::featureAdded, this,
          [this](core::Feature *f) {
            updateFeatureList();
            displayAllShapes();
          });

  // Initial checkpoint (must be after valid document is active)
  doc->checkpoint("Initial");

  return doc;
}

void MainWindow::onTabChanged(int index) {
  if (index < 0) {
    // ── Save current document state before clearing ──────────────────────
    if (m_document)
      saveFeatureStateForDocument(m_document.get());

    m_document = nullptr;
    m_viewport = nullptr;
    return;
  }

  // ── 1. Save state of the document we are LEAVING ─────────────────────
  if (m_document)
    saveFeatureStateForDocument(m_document.get());

  // ── 2. Switch to the new document ────────────────────────────────────
  QWidget *widget = m_tabWidget->widget(index);
  m_viewport = qobject_cast<Viewport3D *>(widget);

  if (m_documentMap.count(widget)) {
    m_document = m_documentMap[widget];
    m_currentFile = m_document->filePath();
    m_modified = m_document->isModified();
  } else {
    m_document = nullptr;
  }

  // ── 3. Restore feature state for the document we are ENTERING ────────
  if (m_document)
    restoreFeatureStateForDocument(m_document.get());

  // Update UI components
  if (m_assemblyTree && m_document) {
    m_assemblyTree->setAssembly(m_document->assembly());
  }

  if (m_parameterEditor && m_document) {
    m_parameterEditor->setParameterManager(m_document->parameterManager());
  }

  // Connect document signals (disconnect outdated ones automatically handled
  // by object life but better to be safe if persistent) Actually, unique
  // connections or tracking connection might be needed. For now, simple
  // connect.
  if (m_document) {
    connect(m_document->featureTree(), &core::FeatureTree::featureAdded, this,
            &MainWindow::updateFeatureList, Qt::UniqueConnection);
    connect(m_document->featureTree(), &core::FeatureTree::featureRemoved, this,
            &MainWindow::updateFeatureList, Qt::UniqueConnection);
    connect(m_document->featureTree(), &core::FeatureTree::featureModified,
            this, &MainWindow::updateFeatureList, Qt::UniqueConnection);
    connect(m_document->featureTree(), &core::FeatureTree::treeStructureChanged,
            this, &MainWindow::updateFeatureList, Qt::UniqueConnection);
    connect(m_document.get(), &core::Document::featureListRestored, this,
            [this](const QStringList& items) {
                if (!m_featureList) return;
                m_rollbackBar = nullptr; // will be dangling after clear
                m_featureList->model()->blockSignals(true);
                m_featureList->clear();
                for (const auto& str : items) {
                    auto *itm = new QListWidgetItem(str);
                    itm->setFlags(itm->flags() & ~Qt::ItemIsDragEnabled);
                    m_featureList->addItem(itm);
                }
                // Re-add rollback bar at the bottom
                m_rollbackBar = createRollbackBarItem();
                m_featureList->addItem(m_rollbackBar);
                m_featureList->model()->blockSignals(false);
            }, Qt::UniqueConnection);
  }

  updateFeatureList();
  updateWindowTitle();

  // Determine mode from document type
  if (m_document) {
    m_assemblyMode = (m_document->type() == core::Document::Type::Assembly);
  } else {
    m_assemblyMode = false;
  }

  updateInterfaceMode();
}

void MainWindow::onCloseTab(int index) {
  QWidget *widget = m_tabWidget->widget(index);

  if (m_documentMap.count(widget) && m_documentMap[widget]->isModified()) {
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, "Application",
                               "The document has been modified.\n"
                               "Do you want to save your changes?",
                               QMessageBox::Save | QMessageBox::Discard |
                                   QMessageBox::Cancel);
    if (ret == QMessageBox::Save) {
      m_tabWidget->setCurrentIndex(index);
      onSaveFile();
    } else if (ret == QMessageBox::Cancel) {
      return;
    }
  }

  m_documentMap.erase(widget);
  m_tabWidget->removeTab(index);
  widget->deleteLater();

  if (m_tabWidget->count() == 0) {
    createNewTab("Untitled");
  }
}

void MainWindow::onNewFile() { createNewTab("Untitled"); }

void MainWindow::onOpenFile() {
  QString fileName = QFileDialog::getOpenFileName(
      this, "Open File", "",
      "All Supported Files (*.ocad *.step *.stp *.iges *.igs *.brep *.stl *.sldprt *.sldasm);;OpenCAD Files (*.ocad);;STEP Files (*.step *.stp);;IGES Files (*.iges *.igs);;BRep Files (*.brep);;STL Files (*.stl);;SolidWorks Files (*.sldprt *.sldasm);;All Files (*.*)");

  if (fileName.isEmpty())
    return;

  QFileInfo fileInfo(fileName);
  QString ext = fileInfo.suffix().toLower();
  createNewTab(fileInfo.fileName());

  if (ext == "ocad") {
    if (m_document) {
      if (m_document->load(fileName)) {
        statusBar()->showMessage("File loaded", 2000);
        displayAllShapes();
        onViewFit();
      } else {
        statusBar()->showMessage("Failed to load file");
        onCloseTab(m_tabWidget->currentIndex());
      }
    }
  } else if (ext == "step" || ext == "stp") {
    opencad::io::StepReader reader;
    if (reader.read(fileName.toStdString())) {
      addShape(reader.getShape().occShape());
      displayAllShapes();
      onViewFit();
      statusBar()->showMessage("STEP file imported", 2000);
    } else {
      statusBar()->showMessage("Failed to import STEP file: " + QString::fromStdString(reader.errorMessage()));
      onCloseTab(m_tabWidget->currentIndex());
    }
  } else if (ext == "iges" || ext == "igs") {
    opencad::io::IgesReader reader;
    if (reader.read(fileName.toStdString())) {
      addShape(reader.getShape().occShape());
      displayAllShapes();
      onViewFit();
      statusBar()->showMessage("IGES file imported", 2000);
    } else {
      statusBar()->showMessage("Failed to import IGES file: " + QString::fromStdString(reader.errorMessage()));
      onCloseTab(m_tabWidget->currentIndex());
    }
  } else if (ext == "brep") {
    opencad::core::Shape s = opencad::io::BRepReader::readFile(fileName.toStdString());
    if (!s.occShape().IsNull()) {
      addShape(s.occShape());
      displayAllShapes();
      onViewFit();
      statusBar()->showMessage("BRep file imported", 2000);
    } else {
      statusBar()->showMessage("Failed to import BRep file");
      onCloseTab(m_tabWidget->currentIndex());
    }
  } else if (ext == "stl") {
    TopoDS_Shape stlShape = opencad::io::StlReader::readFile(fileName);
    if (!stlShape.IsNull()) {
      addShape(stlShape);
      displayAllShapes();
      onViewFit();
      statusBar()->showMessage("STL file imported", 2000);
    } else {
      statusBar()->showMessage("Failed to import STL file");
      onCloseTab(m_tabWidget->currentIndex());
    }
  } else if (ext == "sldprt" || ext == "sldasm") {
    opencad::io::SolidWorksReader reader;
    if (reader.read(fileName.toStdString())) {
      auto shapes = reader.getAllShapes();
      if (!shapes.empty()) {
        for (const auto& s : shapes) {
           addShape(s.occShape());
        }
        displayAllShapes();
        onViewFit();
        statusBar()->showMessage("SolidWorks file imported", 2000);
      } else {
        statusBar()->showMessage("SolidWorks file imported but no shapes found");
        onCloseTab(m_tabWidget->currentIndex());
      }
    } else {
      statusBar()->showMessage("Failed to import SolidWorks file: " + QString::fromStdString(reader.errorMessage()));
      onCloseTab(m_tabWidget->currentIndex());
    }
  } else {
    // Fallback: try as ocad file
    if (m_document) {
      if (m_document->load(fileName)) {
        statusBar()->showMessage("File loaded", 2000);
        displayAllShapes();
        onViewFit();
      } else {
        statusBar()->showMessage("Failed to load file");
        onCloseTab(m_tabWidget->currentIndex());
      }
    }
  }
}

void MainWindow::onCopyGeometryToNewPart() {
  qDebug() << "Debug: onCopyGeometryToNewPart called";
  if (!m_viewport) {
    qDebug() << "Debug: No viewport!";
    return;
  }

  std::vector<TopoDS_Shape> shapesToCopy;

  // Check for selected faces (Multi-select)
  // Check for selected faces
  auto faces = m_viewport->getSelectedFaces();
  auto edges = m_viewport->getSelectedEdges();

  QMessageBox::information(this, "Debug Copy",
                           QString("Faces Selected: %1\nEdges Selected: %2")
                               .arg(faces.size())
                               .arg(edges.size()));

  qDebug() << "Debug: Viewport reports" << faces.size() << "faces selected.";

  // Also check single selected face just in case (compatibility)
  if (faces.empty()) {
    TopoDS_Face face = m_viewport->selectedFace();
    if (!face.IsNull()) {
      shapesToCopy.push_back(face);
      qDebug() << "Debug: Fallback to single face selected.";
    }
  } else {
    for (const auto &face : faces) {
      shapesToCopy.push_back(face);
    }
  }

  // Check for selected edges
  // auto edges = m_viewport->getSelectedEdges(); // Use existing variable
  qDebug() << "Debug: Viewport reports" << edges.size() << "edges selected.";
  for (const auto &edge : edges) {
    shapesToCopy.push_back(edge);
  }

  if (shapesToCopy.empty()) {
    QMessageBox::information(this, "Geometry Linker",
                             QString("Please select faces or edges to "
                                     "copy.\nDebug: Faces=%1, Edges=%2")
                                 .arg(faces.size())
                                 .arg(edges.size()));
    return;
  }

  // Create New Tab
  auto targetDoc = createNewTab("Copied Part");

  // Force update logic if onTabChanged didn't fire synchronously
  if (m_document != targetDoc) {
    qDebug() << "Debug: m_document was stale. Forcing update.";
    m_document = targetDoc;
    if (m_tabWidget) {
      m_viewport = qobject_cast<Viewport3D *>(m_tabWidget->currentWidget());
    }
  }

  // Add shapes to new document
  if (targetDoc) {
    for (const auto &shape : shapesToCopy) {
      targetDoc->addTemporaryShape(shape);
    }
    displayAllShapes();
    onViewFit();
    statusBar()->showMessage(
        QString("Copied %1 entities to new part").arg(shapesToCopy.size()));
  }
}

// ============================================================================
// Edit Feature (SolidWorks-style)
// ============================================================================

int MainWindow::findSketchIndex(
    const std::shared_ptr<sketch::Sketch> &sketch) const {
  if (!m_document || !sketch)
    return -1;
  const auto &sketches = m_document->sketches();
  for (size_t i = 0; i < sketches.size(); ++i) {
    if (sketches[i] == sketch)
      return static_cast<int>(i);
  }
  return -1;
}

void MainWindow::onFeatureListContextMenu(const QPoint &pos) {
  if (!m_featureList)
    return;

  QListWidgetItem *item = m_featureList->itemAt(pos);
  if (!item)
    return;

  int row = m_featureList->row(item);
  // Skip non-feature items (Origin, Planes are first 4 items)
  // Feature records start after the default items
  // The default items are: Origin, XY Plane, XZ Plane, YZ Plane (indices 0-3)
  int featureIndex = row - 4; // Offset by default items count

  if (featureIndex < 0 || featureIndex >= m_featureRecords.size())
    return;

  QMenu contextMenu(this);
  QAction *editAction = contextMenu.addAction(
      QIcon::fromTheme("document-edit"), "Edit Feature");
  QAction *deleteAction = contextMenu.addAction(
      QIcon::fromTheme("edit-delete"), "Delete Feature");

  QAction *selectedAction = contextMenu.exec(m_featureList->mapToGlobal(pos));

  if (selectedAction == editAction) {
    onEditFeature(featureIndex);
  } else if (selectedAction == deleteAction) {
    onDeleteFeature(featureIndex);
  }
}

void MainWindow::onEditFeature(int index) {
  if (index < 0 || index >= m_featureRecords.size())
    return;

  core::FeatureRecord &record = m_featureRecords[index];

  // Show a parameter edit dialog based on feature type
  if (record.type == "Extrude") {
    bool ok = false;
    double oldDepth = record.depth();
    bool oldSymmetric = record.symmetric();
    double oldDraft = record.draftAngle();

    // Use a simple QDialog with spinboxes
    QDialog dialog(this);
    dialog.setWindowTitle("Edit Extrude");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *depthLabel = new QLabel("Depth:");
    QDoubleSpinBox *depthSpin = new QDoubleSpinBox();
    depthSpin->setRange(0.01, 10000.0);
    depthSpin->setDecimals(2);
    depthSpin->setValue(oldDepth);
    layout->addWidget(depthLabel);
    layout->addWidget(depthSpin);

    QCheckBox *symmetricCheck = new QCheckBox("Symmetric (Mid-Plane)");
    symmetricCheck->setChecked(oldSymmetric);
    layout->addWidget(symmetricCheck);

    QLabel *draftLabel = new QLabel("Draft Angle (°):");
    QDoubleSpinBox *draftSpin = new QDoubleSpinBox();
    draftSpin->setRange(-45.0, 45.0);
    draftSpin->setDecimals(1);
    draftSpin->setValue(oldDraft);
    layout->addWidget(draftLabel);
    layout->addWidget(draftSpin);

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted)
      return;

    // Update record with new values
    record.parameters["depth"] = depthSpin->value();
    record.parameters["symmetric"] = symmetricCheck->isChecked();
    record.parameters["draftAngle"] = draftSpin->value();

  } else if (record.type == "Cut") {
    QDialog dialog(this);
    dialog.setWindowTitle("Edit Cut");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *depthLabel = new QLabel("Depth:");
    QDoubleSpinBox *depthSpin = new QDoubleSpinBox();
    depthSpin->setRange(0.01, 10000.0);
    depthSpin->setDecimals(2);
    depthSpin->setValue(record.depth());
    layout->addWidget(depthLabel);
    layout->addWidget(depthSpin);

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted)
      return;

    record.parameters["depth"] = depthSpin->value();

  } else if (record.type == "Revolve") {
    QDialog dialog(this);
    dialog.setWindowTitle("Edit Revolve");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLabel *angleLabel = new QLabel("Angle (°):");
    QDoubleSpinBox *angleSpin = new QDoubleSpinBox();
    angleSpin->setRange(0.1, 360.0);
    angleSpin->setDecimals(1);
    angleSpin->setValue(record.revolveAngle());
    layout->addWidget(angleLabel);
    layout->addWidget(angleSpin);

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);

    if (dialog.exec() != QDialog::Accepted)
      return;

    record.parameters["revolveAngle"] = angleSpin->value();

  } else {
    QMessageBox::information(
        this, "Edit Feature",
        QString("Edit is not yet supported for '%1' features.").arg(record.type));
    return;
  }

  // Save undo state before replaying
  saveUndoState("Edit Feature");

  // Replay all features from the beginning to rebuild geometry
  replayFeaturesFrom(0);

  displayAllShapes();
  statusBar()->showMessage(
      QString("Edited %1 feature").arg(record.type), 3000);
}

void MainWindow::onDeleteFeature(int index) {
  if (index < 0 || index >= m_featureRecords.size())
    return;

  QMessageBox::StandardButton ret = QMessageBox::question(
      this, "Delete Feature",
      QString("Are you sure you want to delete '%1'?")
          .arg(m_featureRecords[index].displayString()),
      QMessageBox::Yes | QMessageBox::No);

  if (ret != QMessageBox::Yes)
    return;

  saveUndoState("Delete Feature");

  // Remove the record
  m_featureRecords.removeAt(index);

  // Replay all features from scratch
  replayFeaturesFrom(0);

  displayAllShapes();
  statusBar()->showMessage("Feature deleted", 3000);
}

TopoDS_Shape MainWindow::replayFeature(const core::FeatureRecord &record,
                                       const TopoDS_Shape &baseShape) {
  if (!m_document)
    return baseShape;

  // Get the sketch for this feature
  std::shared_ptr<sketch::Sketch> sketch;
  if (record.sketchIndex >= 0 &&
      record.sketchIndex < static_cast<int>(m_document->sketches().size())) {
    sketch = m_document->sketches()[record.sketchIndex];
  }

  if (!sketch) {
    qDebug() << "replayFeature: No sketch found for index" << record.sketchIndex;
    return baseShape;
  }

  // Build profile faces from sketch
  auto profileFaces = sketch->buildProfileFaces();
  if (record.profileIndex < 0 ||
      record.profileIndex >= static_cast<int>(profileFaces.size())) {
    qDebug() << "replayFeature: Invalid profile index" << record.profileIndex;
    return baseShape;
  }

  TopoDS_Shape profileShape = profileFaces[record.profileIndex];
  if (profileShape.ShapeType() != TopAbs_FACE) {
    qDebug() << "replayFeature: Profile is not a face";
    return baseShape;
  }
  TopoDS_Face profileFace = TopoDS::Face(profileShape);
  profileFace = enforceFaceHoles(profileFace);

  if (record.type == "Extrude") {
    double depth = record.depth();
    bool symmetric = record.symmetric();
    double draftAngle = record.draftAngle();

    gp_Dir dir = sketch->plane().normal();
    gp_Vec vec(dir);
    vec.Scale(depth);

    TopoDS_Shape extrudedShape;
    try {
      if (symmetric) {
        gp_Vec halfVec = vec;
        halfVec.Scale(0.5);
        gp_Vec backVec = halfVec.Reversed();
        gp_Trsf moveBack;
        moveBack.SetTranslation(backVec);
        BRepBuilderAPI_Transform transform(profileFace, moveBack, true);
        TopoDS_Face movedFace = TopoDS::Face(transform.Shape());
        BRepPrimAPI_MakePrism prism(movedFace, vec);
        if (prism.IsDone())
          extrudedShape = prism.Shape();
      } else {
        BRepPrimAPI_MakePrism prism(profileFace, vec);
        if (prism.IsDone())
          extrudedShape = prism.Shape();
      }

      // Apply draft angle
      if (!extrudedShape.IsNull() && std::abs(draftAngle) > 0.001) {
        double angleRad = draftAngle * M_PI / 180.0;
        gp_Dir draftDir = dir;
        gp_Pln neutralPlane = sketch->plane().plane();
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
        }
      }
    } catch (...) {
      return baseShape;
    }

    if (extrudedShape.IsNull())
      return baseShape;

    // Fuse or set
    if (baseShape.IsNull()) {
      return extrudedShape;
    } else {
      try {
        BRepAlgoAPI_Fuse fuseOp(baseShape, extrudedShape);
        if (fuseOp.IsDone())
          return fuseOp.Shape();
      } catch (...) {
      }
      return baseShape;
    }

  } else if (record.type == "Cut") {
    if (baseShape.IsNull())
      return baseShape;

    double depth = record.depth();
    gp_Dir cutDir = sketch->plane().normal();
    gp_Vec cutVec(cutDir);
    cutVec.Scale(-depth);

    try {
      BRepPrimAPI_MakePrism prism(profileFace, cutVec);
      if (prism.IsDone()) {
        TopoDS_Shape cutTool = prism.Shape();
        BRepAlgoAPI_Cut cutOp(baseShape, cutTool);
        if (cutOp.IsDone())
          return cutOp.Shape();
      }
    } catch (...) {
    }
    return baseShape;

  } else if (record.type == "Revolve") {
    double angle = record.revolveAngle();
    int axisIndex = record.parameters.value("axisIndex", 1).toInt();

    gp_Ax1 axis;
    gp_Pnt origin(0, 0, 0);
    switch (axisIndex) {
    case 0:
      axis = gp_Ax1(origin, gp_Dir(1, 0, 0));
      break;
    case 1:
      axis = gp_Ax1(origin, gp_Dir(0, 1, 0));
      break;
    case 2:
      axis = gp_Ax1(origin, gp_Dir(0, 0, 1));
      break;
    default:
      axis = gp_Ax1(origin, gp_Dir(0, 1, 0));
      break;
    }

    try {
      part::RevolveFeature revolve;
      TopoDS_Shape revolvedShape = revolve.executeFace(profileFace, axis, angle);
      if (revolvedShape.IsNull())
        return baseShape;

      if (baseShape.IsNull()) {
        return revolvedShape;
      } else {
        try {
          BRepAlgoAPI_Fuse fuseOp(baseShape, revolvedShape);
          if (fuseOp.IsDone())
            return fuseOp.Shape();
        } catch (...) {
        }
        return baseShape;
      }
    } catch (...) {
      return baseShape;
    }
  }

  return baseShape;
}


void MainWindow::replayFeaturesFrom(int startIndex) {
  if (!m_document || m_featureRecords.isEmpty())
    return;

  TopoDS_Shape currentShape;
  m_document->temporaryShapes().clear();

  // Determine replay limit: respect m_rollbackPosition.
  // -1 means "show all", otherwise replay only up to that many features.
  int endIndex = m_featureRecords.size();
  if (m_rollbackPosition >= 0 && m_rollbackPosition < endIndex)
    endIndex = m_rollbackPosition;

  for (int i = startIndex; i < endIndex; ++i) {
    currentShape = replayFeature(m_featureRecords[i], currentShape);
  }

  if (!currentShape.IsNull()) {
    m_document->addTemporaryShape(currentShape);
  }

  // ── Rebuild the feature list UI ──────────────────────────────────────────
  if (m_featureList) {
    m_rollbackBar = nullptr; // null BEFORE clear() so any signal during clear can't crash
    m_featureList->model()->blockSignals(true);
    m_featureList->clear(); // deletes all items including old rollback bar

    // Fixed origin items (non-draggable)
    auto addOriginItem = [this](const QString &text) {
      auto *itm = new QListWidgetItem(text);
      itm->setFlags(itm->flags() & ~Qt::ItemIsDragEnabled & ~Qt::ItemIsDropEnabled);
      m_featureList->addItem(itm);
    };
    addOriginItem(QString::fromUtf8("\U0001F310 Origin"));
    addOriginItem(QString::fromUtf8("  \u2514 XY Plane"));
    addOriginItem(QString::fromUtf8("  \u2514 XZ Plane"));
    addOriginItem(QString::fromUtf8("  \u2514 YZ Plane"));

    // The bar sits AFTER the first `barPos` feature records.
    // barPos == -1 or >= size  →  bar at the very bottom (all features active).
    const int barPos = (m_rollbackPosition >= 0 && m_rollbackPosition <= m_featureRecords.size())
                           ? m_rollbackPosition
                           : m_featureRecords.size();

    for (int ri = 0; ri < m_featureRecords.size(); ++ri) {
      // Insert rollback bar BEFORE recording ri if that's where barPos is.
      if (ri == barPos) {
        m_rollbackBar = createRollbackBarItem();
        m_featureList->addItem(m_rollbackBar);
      }

      bool rolledBack = (ri >= barPos);
      auto *itm = new QListWidgetItem(m_featureRecords[ri].displayString());
      itm->setData(Qt::UserRole + 1, ri);
      itm->setFlags(itm->flags() & ~Qt::ItemIsDragEnabled); // not draggable

      if (rolledBack) {
        itm->setForeground(QColor(130, 130, 130));
        QFont f = itm->font();
        f.setItalic(true);
        itm->setFont(f);
      }
      m_featureList->addItem(itm);
    }

    // If bar belongs at the very end (all features active), add it now.
    if (barPos >= m_featureRecords.size()) {
      m_rollbackBar = createRollbackBarItem();
      m_featureList->addItem(m_rollbackBar);
    }

    m_featureList->model()->blockSignals(false);
  }

  m_modified = true;
  updateWindowTitle();
  // Always keep the per-document map in sync so tab switching
  // restores the correct feature list and rollback position.
  if (m_document)
    saveFeatureStateForDocument(m_document.get());
}

// ── Rollback Bar helpers ──────────────────────────────────────────────────

QListWidgetItem *MainWindow::createRollbackBarItem() {
  auto *bar = new QListWidgetItem(
      QString::fromUtf8("\u23EC \u2500\u2500\u2500\u2500\u2500 Rollback Bar \u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500\u2500"));
  QFont f = bar->font();
  f.setBold(true);
  bar->setFont(f);
  bar->setBackground(QColor(190, 140, 0)); // amber / gold
  bar->setForeground(Qt::white);
  bar->setData(Qt::UserRole, QString("ROLLBACK_BAR")); // marker
  // Make draggable but not editable. Keep ItemIsSelectable so Qt's
  // InternalMove drag-drop does NOT crash (non-selectable + draggable = crash).
  bar->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
  return bar;
}

void MainWindow::addFeatureListItem(const QString &text) {
  if (!m_featureList) return;

  auto *itm = new QListWidgetItem(text);
  itm->setFlags(itm->flags() & ~Qt::ItemIsDragEnabled); // not draggable

  // Always SCAN for the rollback bar — never trust the stored pointer which
  // may have been invalidated by a m_featureList->clear() elsewhere.
  int barRow = m_featureList->count(); // default: end of list
  if (QListWidgetItem *bar = findRollbackBar()) {
    int found = m_featureList->row(bar);
    if (found >= 0) barRow = found;
  }

  m_featureList->model()->blockSignals(true);
  m_featureList->insertItem(barRow, itm);
  m_featureList->model()->blockSignals(false);

  // Sync per-document map immediately (don't wait for tab switch).
  if (m_document)
    saveFeatureStateForDocument(m_document.get());
}

void MainWindow::ensureRollbackBarAtBottom() {
  if (!m_featureList) return;
  // Use scan to avoid dangling pointer.
  QListWidgetItem *bar = findRollbackBar();
  if (!bar) return;
  m_featureList->model()->blockSignals(true);
  int row = m_featureList->row(bar);
  if (row >= 0 && row != m_featureList->count() - 1) {
    m_featureList->takeItem(row);
    m_featureList->addItem(bar);
  }
  m_featureList->model()->blockSignals(false);
}

QListWidgetItem *MainWindow::findRollbackBar() const {
  if (!m_featureList) return nullptr;
  for (int i = 0; i < m_featureList->count(); ++i) {
    auto *itm = m_featureList->item(i);
    if (itm && itm->data(Qt::UserRole).toString() == QLatin1String("ROLLBACK_BAR"))
      return itm;
  }
  return nullptr;
}

void MainWindow::onRollbackBarMoved(int srcRow, int dstRow) {
  // -----------------------------------------------------------------------
  // IMPORTANT: This is called from inside a rowsMoved signal, while Qt's
  // drag-drop system is still running. We must NOT clear or rebuild the
  // QListWidget synchronously here — doing so causes a crash because Qt
  // holds internal pointers to list items.
  //
  // Solution: calculate the desired rollback position, store it, then
  // defer the actual model/UI rebuild to the next event-loop tick via
  // QTimer::singleShot(0, ...).
  // -----------------------------------------------------------------------

  static const int kOriginItems = 4; // Origin + 3 planes

  if (!m_featureList) return;

  // Qt rowsMoved semantics:
  //   srcRow  = row the item was at BEFORE the move
  //   dstRow  = row BEFORE which the item was inserted AFTER the move
  //   When moving downward dstRow > srcRow, actual final row = dstRow - 1.
  int actualDst = (dstRow > srcRow) ? dstRow - 1 : dstRow;
  actualDst = qMax(kOriginItems, actualDst); // bar can't go above origin rows

  // How many feature records are above the bar?
  int featuresAboveBar = actualDst - kOriginItems;
  featuresAboveBar = qMax(0, qMin(featuresAboveBar, m_featureRecords.size()));

  const bool allActive = (featuresAboveBar >= m_featureRecords.size());
  m_rollbackPosition = allActive ? -1 : featuresAboveBar;

  // Defer the rebuild to avoid crashing inside the rowsMoved handler.
  QTimer::singleShot(0, this, [this, featuresAboveBar, allActive]() {
    // By now Qt's drag-drop internals have finished — safe to clear & rebuild.
    if (!m_featureRecords.isEmpty()) {
      replayFeaturesFrom(0);
    } else {
      // No feature records: just update the bar visuals (nothing to rebuild).
      if (m_rollbackBar && m_featureList) {
        ensureRollbackBarAtBottom();
      }
    }
    displayAllShapes();

    if (allActive) {
      statusBar()->showMessage("Rollback bar at end \u2013 all features active", 3000);
    } else {
      statusBar()->showMessage(
          QString("Rollback: showing %1 of %2 features")
              .arg(featuresAboveBar)
              .arg(m_featureRecords.size()),
          3000);
    }
  });
}

// ── Per-document feature state save / restore ─────────────────────────────

void MainWindow::saveFeatureStateForDocument(core::Document *doc) {
  if (!doc) return;
  DocumentFeatureState state;
  state.featureRecords  = m_featureRecords;
  state.rollbackPosition = m_rollbackPosition;
  m_documentFeatureState[doc] = state;
}

void MainWindow::restoreFeatureStateForDocument(core::Document *doc) {
  if (!doc) return;

  auto it = m_documentFeatureState.find(doc);
  if (it != m_documentFeatureState.end()) {
    // Restore saved state for this document
    m_featureRecords   = it->featureRecords;
    m_rollbackPosition = it->rollbackPosition;
  } else {
    // First time we visit this document — start clean
    m_featureRecords.clear();
    m_rollbackPosition = -1;
  }
  // m_rollbackBar will be re-created by the updateFeatureList() / replayFeaturesFrom()
  // call that follows in onTabChanged.
  m_rollbackBar = nullptr;
}

} // namespace ui
} // namespace opencad

