#pragma once
/**
 * @file MainWindow.h
 * @brief Main application window
 *
 * OpenCAD - Modular CAD/CAE Platform
 * UI Module
 */

#include "../core/geometry/Shape.h"
#include <QDockWidget>
#include <QListWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QProgressBar> // Added
#include <QStatusBar>
#include <QToolBar>
#include <memory>
#include <vector>

class QStackedWidget; // Forward declaration

// Forward declarations
namespace opencad {
namespace sketch {
class Sketch;
}
namespace core {
class Document;
class Shape; // Added
} // namespace core

namespace assembly {
class Component;
}

} // namespace opencad

#include <AIS_InteractiveObject.hxx>
#include <map>

#include "ai/CadQueryClient.h"
#include "ai/LLMClient.h"
#include "dialogs/CadQueryEditorDialog.h"

namespace opencad {
namespace ui {

class Viewport3D;
class AssemblyTreeWidget;
class AIChatPanel; // Forward declaration

/**
 * @class MainWindow
 * @brief Main application window with viewport and tools
 */
class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

  // Shape management
  void addShape(const TopoDS_Shape &shape);
  void clearShapes();
  void displayAllShapes();

  // AI Integration
  void onAiRun(const QString &prompt);

protected:
  void closeEvent(QCloseEvent *event) override;
  bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
  // File menu
  void onNewFile();
  void onOpenFile();
  void onSaveFile();
  void onSaveAs();
  void onExportSTL();
  void onExportSTEP();
  void onExit();

  // Edit menu
  void onUndo();
  void onRedo();
  void onDelete();

  // View menu
  void onViewFit();
  void onViewFront();
  void onViewBack();
  void onViewTop();
  void onViewBottom();
  void onViewLeft();
  void onViewRight();
  void onViewIsometric();

  // Create menu (primitives)
  void onCreateBox();
  void onCreateCylinder();
  void onCreateSphere();
  void onCreateCone();

  // Boolean menu
  void onBooleanFuse();
  void onBooleanCut();
  void onBooleanCommon();

  // Sketch menu
  void onNewSketch();
  void onSketchOnFace(); // Select face then sketch
  void onEditSketch();
  void onFinishSketch();
  void onFaceSelected(); // Handle face selection

  // Sketch tools
  void onSketchLine();
  void onSketchRectangle();
  void onSketchCircle();
  void onSketchArc();
  void onSketchPoint();
  void onSketchSpline();
  void onSketchEllipse();
  void onSketchProject(); // Added

  // Sketch constraints
  void onConstraintHorizontal();
  void onConstraintVertical();
  void onConstraintCoincident();
  void onConstraintDistance();
  void onConstraintRadius();
  void onConstraintAngle();
  void onConstraintParallel();
  void onConstraintPerpendicular();

  // Part features
  void onReferencePlane();
  void onExtrude();
  void onCut();
  void onRevolve();
  void onFillet();
  void onChamfer();
  void onShell();
  void onSweep();
  void onLoft();
  void onLoftSurface();
  void onPattern();
  void onMirror();

  // New Part features
  void onScale();
  void onHoleWizard();
  void onDraft();
  void onRib();
  void onThicken();
  void onOffsetSurface();
  void onSplit();

  void onDome();
  void onGear(); // Added

  // Sketch advanced tools
  void onSketchSlot();
  void onSketchPolygon();
  void onConvertEntities();
  void onIntersectionCurve();
  void onSketchMirror();
  void onSketchTrim();
  void onSketchExtend();
  void onSketchLinearPattern();
  void onSketchCircularPattern();

  // Additional constraints
  void onConstraintTangent();
  void onConstraintEqual();
  void onConstraintFix();
  void onConstraintConcentric();

  // Profile selection (visual Extrude/Cut)
  void onProfileSelected(int profileIndex);
  void onRingSelected(int outerProfileIndex, int innerProfileIndex);
  void
  onMultiProfilesConfirmed(const std::vector<std::pair<int, int>> &selections);
  void onProfileSelectionCancelled();

  // Selection modes
  void onSelectShape();
  void onSelectFace();
  void onSelectEdge();
  void onSelectVertex();

  // Help menu
  void onAbout();

  // Tool settings apply
  void onToolApply();

  // Feature tree interaction
  void onFeatureSelected(QListWidgetItem *item);
  void onFeatureContextMenu(const QPoint &pos);
  void onToggleFeatureSuppression();
  void onFeatureReordered();

  // Assembly tree interaction
  void onAssemblyTreeSelection(std::shared_ptr<assembly::Component> component);

private:
  void setupMenus();
  void setupToolbars();
  void setupStatusBar();
  void setupDockWidgets();
  void updateFeatureList();
  void updateWindowTitle();
  void updateInterfaceMode();
  void updateSketchToolsEnabled(bool enabled);
  void showSketchEditor();

  std::unique_ptr<Viewport3D> m_viewport;
  QString m_currentFile;
  bool m_modified = false;

  // Central document (contains FeatureTree, UndoRedoManager, ParameterManager)
  std::unique_ptr<core::Document> m_document;

  // Sketch
  std::shared_ptr<sketch::Sketch> m_currentSketch;
  bool m_sketchMode = false;

  // Dock widgets
  QDockWidget *m_featureTreeDock = nullptr;
  QStackedWidget *m_treeStack = nullptr; // Added
  QListWidget *m_featureList = nullptr;
  QDockWidget *m_sketchDock = nullptr;
  class SketchView2D *m_sketchView = nullptr;
  QDockWidget *m_propertiesDock = nullptr;
  class PropertiesPanel *m_propertiesPanel = nullptr;
  QDockWidget *m_parameterDock = nullptr;
  class ParameterEditor *m_parameterEditor = nullptr;
  QDockWidget *m_toolSettingsDock = nullptr;
  class ToolSettingsPanel *m_toolSettingsPanel = nullptr;
  QDockWidget *m_profileSelectionDock = nullptr;
  class ProfileSelectionPanel *m_profileSelectionPanel = nullptr;
  AssemblyTreeWidget *m_assemblyTree = nullptr; // Fixed type

  // Toolbars
  QToolBar *m_sketchToolbar = nullptr;
  QToolBar *m_constraintToolbar = nullptr;
  QToolBar *m_featureToolbar = nullptr;
  QToolBar *m_assemblyToolbar = nullptr;

  // Profile selection state (for visual Extrude/Cut)
  enum class PendingOperation { None, Extrude, Cut };
  PendingOperation m_pendingOperation = PendingOperation::None;

  // Face-based operation state
  enum class PendingFaceOperation {
    None,
    Dome,
    Shell,
    Draft,
    Thicken,
    OffsetSurface
  };
  PendingFaceOperation m_pendingFaceOperation = PendingFaceOperation::None;

  // Pending parameters for face operations
  double m_pendingDomeHeight = 10.0;
  bool m_pendingDomeReversed = false;
  double m_pendingShellThickness = 2.0;
  bool m_pendingShellOutward = false;
  double m_pendingDraftAngle = 3.0;
  double m_pendingThickenValue = 5.0;
  double m_pendingOffsetValue = 5.0;

  // Selected profile index for Extrude/Cut (after visual selection)
  int m_selectedProfileIndex = -1;

  // Helper to save state before operations
  void saveUndoState(const std::string &description);

  /**
   * @brief Sketch plane selection result
   */
  enum class SketchPlaneType {
    None,
    XY,  // Top plane (Z=0)
    XZ,  // Front plane (Y=0)
    YZ,  // Right plane (X=0)
    Face // Select from 3D model
  };

  // Active Part tool tracking for panel-based operations
  enum class ActivePartTool {
    None,
    Extrude,
    Cut,
    Revolve,
    Sweep,
    Loft,
    Fillet,
    Chamfer,
    Shell,
    Dome,
    Draft,
    Pattern,
    Mirror,
    Boolean,
    ReferencePlane,
    Split,
    NewSketch,

    Project, // Added
    Gear     // Added
  };
  ActivePartTool m_activePartTool = ActivePartTool::None;

  // Assembly
  bool m_assemblyMode = false;

  enum class AssemblyAction { None, Move, Constraint };
  AssemblyAction m_currentAssemblyAction = AssemblyAction::None;

  // Track selected components for assembly operations
  std::vector<std::shared_ptr<assembly::Component>> m_selectedComponents;

  // Mate selection state
  enum class MateStep { None, SelectFirst, SelectSecond };
  MateStep m_mateStep = MateStep::None;
  TopoDS_Shape m_mateShape1; // First selected sub-shape
  TopoDS_Shape m_mateShape2; // Second selected sub-shape
  std::shared_ptr<assembly::Component> m_mateComponent1; // First component

  // Helper find component from shape
  std::shared_ptr<assembly::Component>
  findComponentFromShape(const TopoDS_Shape &shape);

  // Handle generic geometry selection (including Shapes for assembly)
  void onGeometrySelected(const QString &type);

  // Optimized visual update
  void updateAssemblyVisuals();

  // Assembly slots
  void onNewAssembly();
  void onInsertComponent();
  void onAssemblyConstraint();
  void onMoveComponent();
  void onMoveMultipleComponents();  // Batch move multiple selected components
  void onGroupSelectedComponents(); // Group selected components into a group
  void onSolveConstraints();        // Manually trigger constraint solving

  // Parametric assembly operations
  void onParametricMove();  // Move with precise X/Y/Z input
  void onRotateComponent(); // Rotate component around an axis
  void onCopyComponent();   // Copy selected component
  void onMoveToOrigin();    // Reset component position to origin

  // Selection Sync
  void onShapeSelected(const TopoDS_Shape &shape,
                       Handle(AIS_InteractiveObject) object);

private:
  // Visual Map for Selection Sync: AIS Object -> Component
  std::map<Handle(AIS_InteractiveObject), std::weak_ptr<assembly::Component>>
      m_visualMap;

  QProgressBar *m_progressBar = nullptr; // Added

  // AI Clients
  std::unique_ptr<ai::CadQueryClient> m_cqClient;
  std::unique_ptr<ai::LLMClient> m_llmClient;
  CadQueryEditorDialog *m_cqEditor = nullptr;

  // UI Panels
  AIChatPanel *m_aiChatPanel = nullptr;
};

} // namespace ui
} // namespace opencad
