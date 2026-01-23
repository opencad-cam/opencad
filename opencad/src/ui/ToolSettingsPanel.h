/**
 * @file ToolSettingsPanel.h
 * @brief Professional tool settings panel for all CAD operations
 */

#pragma once

#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

namespace opencad {
namespace ui {

// Forward declaration
class SketchView2D;
enum class SketchToolType;

/**
 * @brief Professional settings panel for all CAD tools
 */
class ToolSettingsPanel : public QWidget {
  Q_OBJECT

public:
  explicit ToolSettingsPanel(QWidget *parent = nullptr);
  ~ToolSettingsPanel() override = default;

  void setSketchView(SketchView2D *view);
  void updateForTool(SketchToolType tool);

  // Show specific settings panels
  void showExtrudeSettings();
  void showCutSettings();
  void showFilletSettings();
  void showChamferSettings();
  void showPatternSettings();
  void showMirrorSettings();
  void showShellSettings();
  void showDomeSettings();
  void showDraftSettings();
  void showThickenSettings();
  void showOffsetSurfaceSettings();
  void showConstraintSettings();
  void showNoToolSettings();
  void showRevolveSettings();
  void showSweepSettings();
  void showLoftSettings();
  void showBooleanSettings();
  void showReferencePlaneSettings();
  void showSplitSettings();
  void showSketchPlaneSettings();
  void showScaleSettings();
  void showHoleSettings();

  // Get current values
  double extrudeDepth() const;
  bool extrudeSymmetric() const;
  double extrudeDraftAngle() const;
  double filletRadius() const;
  double chamferSize() const;
  double chamferAngle() const;
  int patternCount() const;
  double patternSpacing() const;
  double patternAngle() const;
  double shellThickness() const;
  double domeHeight() const;
  double domeRatio() const;
  double draftAngle() const;
  bool draftOutward() const;
  double thickenValue() const;
  double offsetSurfaceValue() const;
  double cutDepth() const;
  bool cutThroughAll() const;
  // Reference Plane settings
  int refPlaneType() const;
  double refPlaneOffset() const;
  // Revolve settings
  double revolveAngle() const;
  int revolveAxis() const;
  // Sweep settings
  bool sweepSolid() const;
  // Loft settings
  bool loftSolid() const;
  bool loftRuled() const;
  // Boolean settings
  int booleanOperation() const;
  // Mirror settings
  int mirrorAxis() const;
  // Split settings
  int splitPlane() const;
  double splitOffset() const;
  int splitKeepPart() const;
  int sketchPlaneType() const;

signals:
  void polygonSidesChanged(int sides);
  void slotWidthChanged(double width);
  void autoConstraintEnabledChanged(bool enabled);
  void autoConstraintToleranceChanged(double degrees);
  void polygonInscribedChanged(bool inscribed);
  void settingsChanged();
  void applyClicked();

private slots:
  void onPolygonSidesChanged(int value);
  void onSlotWidthChanged(double value);
  void onAutoConstraintToggled(bool checked);
  void onToleranceChanged(double value);
  void onPolygonInscribedToggled(bool checked);
  void onSettingsChanged();

private:
  void setupUI();
  void showPolygonSettings();
  void showSlotSettings();
  void showLineSettings();
  void showGeneralSettings();
  void hideAllSettings();

  SketchView2D *m_sketchView = nullptr;

  // Current tool label
  QLabel *m_toolLabel;
  QLabel *m_descriptionLabel;

  // General settings group (Auto-constraint)
  QGroupBox *m_generalGroup;
  QCheckBox *m_autoConstraintCheck;
  QDoubleSpinBox *m_toleranceSpin;

  // Polygon settings
  QGroupBox *m_polygonGroup;
  QSpinBox *m_polygonSidesSpin;
  QCheckBox *m_polygonInscribedCheck;

  // Slot settings
  QGroupBox *m_slotGroup;
  QDoubleSpinBox *m_slotWidthSpin;

  // === PART FEATURE SETTINGS ===

  // Extrude settings
  QGroupBox *m_extrudeGroup;
  QDoubleSpinBox *m_extrudeDepthSpin;
  QCheckBox *m_extrudeSymmetricCheck;
  QDoubleSpinBox *m_extrudeDraftSpin;
  QComboBox *m_extrudeDirectionCombo;

  // Cut settings
  QGroupBox *m_cutGroup;
  QDoubleSpinBox *m_cutDepthSpin;
  QCheckBox *m_cutThroughAllCheck;

  // Fillet settings
  QGroupBox *m_filletGroup;
  QDoubleSpinBox *m_filletRadiusSpin;
  QComboBox *m_filletTypeCombo;

  // Chamfer settings
  QGroupBox *m_chamferGroup;
  QDoubleSpinBox *m_chamferSizeSpin;
  QDoubleSpinBox *m_chamferAngleSpin;

  // Pattern settings (Linear & Circular)
  QGroupBox *m_patternGroup;
  QSpinBox *m_patternCountSpin;
  QDoubleSpinBox *m_patternSpacingSpin;
  QComboBox *m_patternTypeCombo;
  QDoubleSpinBox *m_patternAngleSpin;

  // Mirror settings
  QGroupBox *m_mirrorGroup;
  QComboBox *m_mirrorAxisCombo;

  // Shell settings
  QGroupBox *m_shellGroup;
  QDoubleSpinBox *m_shellThicknessSpin;

  // Dome settings
  QGroupBox *m_domeGroup;
  QDoubleSpinBox *m_domeHeightSpin;
  QDoubleSpinBox *m_domeRatioSpin;

  // Draft settings
  QGroupBox *m_draftGroup;
  QDoubleSpinBox *m_draftAngleSpin;
  QCheckBox *m_draftOutwardCheck;

  // Thicken settings
  QGroupBox *m_thickenGroup;
  QDoubleSpinBox *m_thickenValueSpin;

  // OffsetSurface settings
  QGroupBox *m_offsetSurfaceGroup;
  QDoubleSpinBox *m_offsetSurfaceValueSpin;

  // Revolve settings
  QGroupBox *m_revolveGroup;
  QDoubleSpinBox *m_revolveAngleSpin;
  QComboBox *m_revolveAxisCombo;

  // Sweep settings
  QGroupBox *m_sweepGroup;
  QCheckBox *m_sweepSolidCheck;
  QLabel *m_sweepPathLabel;

  // Loft settings
  QGroupBox *m_loftGroup;
  QCheckBox *m_loftSolidCheck;
  QCheckBox *m_loftRuledCheck;
  QLabel *m_loftProfilesLabel;

  // Boolean settings
  QGroupBox *m_booleanGroup;
  QComboBox *m_booleanOpCombo;

  // Reference Plane settings
  QGroupBox *m_refPlaneGroup;
  QComboBox *m_refPlaneTypeCombo;
  QDoubleSpinBox *m_refPlaneOffsetSpin;

  // Split settings
  QGroupBox *m_splitGroup;
  QComboBox *m_splitPlaneCombo;
  QDoubleSpinBox *m_splitOffsetSpin;
  QComboBox *m_splitKeepPartCombo;

  // New Sketch Plane settings
  QGroupBox *m_sketchPlaneGroup;
  QRadioButton *m_sketchPlaneXY;
  QRadioButton *m_sketchPlaneXZ;
  QRadioButton *m_sketchPlaneYZ;
  QRadioButton *m_sketchPlaneFace;

  // Constraint info
  QGroupBox *m_constraintGroup;
  QLabel *m_constraintInfoLabel;
  QLabel *m_constraintTypeLabel;

  // Scale settings
  QGroupBox *m_scaleGroup;
  QComboBox *m_scaleTypeCombo;
  QDoubleSpinBox *m_scaleFactorSpin;
  QDoubleSpinBox *m_scaleXSpin;
  QDoubleSpinBox *m_scaleYSpin;
  QDoubleSpinBox *m_scaleZSpin;
  QCheckBox *m_scaleCentroidCheck;

  // Hole settings
  QGroupBox *m_holeGroup;
  QComboBox *m_holeTypeCombo;
  QDoubleSpinBox *m_holeDiameterSpin;
  QDoubleSpinBox *m_holeDepthSpin;
  QDoubleSpinBox *m_cboreDiameterSpin;
  QDoubleSpinBox *m_cboreDepthSpin;
  QDoubleSpinBox *m_csinkDiameterSpin;
  QDoubleSpinBox *m_csinkAngleSpin;

  // Apply button
  QPushButton *m_applyButton;
};

} // namespace ui
} // namespace opencad
