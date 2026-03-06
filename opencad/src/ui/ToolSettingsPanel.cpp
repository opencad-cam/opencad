/**
 * @file ToolSettingsPanel.cpp
 * @brief Professional tool settings panel implementation
 */

#include "ToolSettingsPanel.h"
#include "sketch/SketchView2D.h"

namespace opencad {
namespace ui {

ToolSettingsPanel::ToolSettingsPanel(QWidget *parent) : QWidget(parent) {
  setupUI();
}

void ToolSettingsPanel::setupUI() {
  auto *mainLayout = new QVBoxLayout(this);
  mainLayout->setContentsMargins(8, 8, 8, 8);
  mainLayout->setSpacing(8);

  // === HEADER ===
  m_toolLabel = new QLabel("No Tool Selected", this);
  m_toolLabel->setStyleSheet(
      "font-weight: bold; font-size: 14pt; color: #2196F3; padding: 5px;");
  mainLayout->addWidget(m_toolLabel);

  m_descriptionLabel = new QLabel("", this);
  m_descriptionLabel->setStyleSheet(
      "color: #666; font-size: 9pt; padding: 0 5px;");
  m_descriptionLabel->setWordWrap(true);
  mainLayout->addWidget(m_descriptionLabel);

  // === SKETCH SETTINGS ===

  // General settings (Auto-Constraint)
  m_generalGroup = new QGroupBox("⚙️ Auto-Constraint", this);
  m_generalGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *generalLayout = new QFormLayout(m_generalGroup);
  m_autoConstraintCheck = new QCheckBox("Enable", this);
  m_autoConstraintCheck->setChecked(true);
  generalLayout->addRow(m_autoConstraintCheck);
  m_toleranceSpin = new QDoubleSpinBox(this);
  m_toleranceSpin->setRange(0.5, 15.0);
  m_toleranceSpin->setValue(5.0);
  m_toleranceSpin->setSuffix("°");
  generalLayout->addRow("Tolerance:", m_toleranceSpin);
  mainLayout->addWidget(m_generalGroup);
  m_generalGroup->hide();

  // Polygon settings
  m_polygonGroup = new QGroupBox("⬡ Polygon", this);
  m_polygonGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *polygonLayout = new QFormLayout(m_polygonGroup);
  m_polygonSidesSpin = new QSpinBox(this);
  m_polygonSidesSpin->setRange(3, 32);
  m_polygonSidesSpin->setValue(6);
  polygonLayout->addRow("Sides:", m_polygonSidesSpin);
  m_polygonInscribedCheck = new QCheckBox("Inscribed", this);
  m_polygonInscribedCheck->setChecked(true);
  polygonLayout->addRow(m_polygonInscribedCheck);
  mainLayout->addWidget(m_polygonGroup);
  m_polygonGroup->hide();

  // Slot settings
  m_slotGroup = new QGroupBox("⊂⊃ Slot", this);
  m_slotGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *slotLayout = new QFormLayout(m_slotGroup);
  m_slotWidthSpin = new QDoubleSpinBox(this);
  m_slotWidthSpin->setRange(0.5, 500.0);
  m_slotWidthSpin->setValue(10.0);
  m_slotWidthSpin->setSuffix(" mm");
  slotLayout->addRow("Width:", m_slotWidthSpin);
  mainLayout->addWidget(m_slotGroup);
  m_slotGroup->hide();

  // === PART FEATURE SETTINGS ===

  // Extrude settings
  m_extrudeGroup = new QGroupBox("📦 Extrude", this);
  m_extrudeGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *extrudeLayout = new QFormLayout(m_extrudeGroup);
  m_extrudeDepthSpin = new QDoubleSpinBox(this);
  m_extrudeDepthSpin->setRange(0.1, 10000.0);
  m_extrudeDepthSpin->setValue(10.0);
  m_extrudeDepthSpin->setSuffix(" mm");
  extrudeLayout->addRow("Depth:", m_extrudeDepthSpin);
  m_extrudeDirectionCombo = new QComboBox(this);
  m_extrudeDirectionCombo->addItems(
      {"One Direction", "Symmetric", "Two Directions"});
  extrudeLayout->addRow("Direction:", m_extrudeDirectionCombo);
  m_extrudeSymmetricCheck = new QCheckBox("Mid-plane", this);
  extrudeLayout->addRow(m_extrudeSymmetricCheck);
  m_extrudeDraftSpin = new QDoubleSpinBox(this);
  m_extrudeDraftSpin->setRange(0.0, 45.0);
  m_extrudeDraftSpin->setValue(0.0);
  m_extrudeDraftSpin->setSuffix("°");
  extrudeLayout->addRow("Draft:", m_extrudeDraftSpin);
  mainLayout->addWidget(m_extrudeGroup);
  m_extrudeGroup->hide();

  // Cut settings
  m_cutGroup = new QGroupBox("🔪 Cut", this);
  m_cutGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *cutLayout = new QFormLayout(m_cutGroup);
  m_cutDepthSpin = new QDoubleSpinBox(this);
  m_cutDepthSpin->setRange(0.1, 10000.0);
  m_cutDepthSpin->setValue(10.0);
  m_cutDepthSpin->setSuffix(" mm");
  cutLayout->addRow("Depth:", m_cutDepthSpin);
  m_cutThroughAllCheck = new QCheckBox("Through All", this);
  cutLayout->addRow(m_cutThroughAllCheck);
  mainLayout->addWidget(m_cutGroup);
  m_cutGroup->hide();

  // Fillet settings
  m_filletGroup = new QGroupBox("⭕ Fillet", this);
  m_filletGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *filletLayout = new QFormLayout(m_filletGroup);
  m_filletRadiusSpin = new QDoubleSpinBox(this);
  m_filletRadiusSpin->setRange(0.1, 1000.0);
  m_filletRadiusSpin->setValue(5.0);
  m_filletRadiusSpin->setSuffix(" mm");
  filletLayout->addRow("Radius:", m_filletRadiusSpin);
  m_filletTypeCombo = new QComboBox(this);
  m_filletTypeCombo->addItems({"Constant", "Variable", "Full Round"});
  filletLayout->addRow("Type:", m_filletTypeCombo);
  mainLayout->addWidget(m_filletGroup);
  m_filletGroup->hide();

  // Chamfer settings
  m_chamferGroup = new QGroupBox("◢ Chamfer", this);
  m_chamferGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *chamferLayout = new QFormLayout(m_chamferGroup);
  m_chamferSizeSpin = new QDoubleSpinBox(this);
  m_chamferSizeSpin->setRange(0.1, 1000.0);
  m_chamferSizeSpin->setValue(2.0);
  m_chamferSizeSpin->setSuffix(" mm");
  chamferLayout->addRow("Size:", m_chamferSizeSpin);
  m_chamferAngleSpin = new QDoubleSpinBox(this);
  m_chamferAngleSpin->setRange(1.0, 89.0);
  m_chamferAngleSpin->setValue(45.0);
  m_chamferAngleSpin->setSuffix("°");
  chamferLayout->addRow("Angle:", m_chamferAngleSpin);
  mainLayout->addWidget(m_chamferGroup);
  m_chamferGroup->hide();

  // Pattern settings
  m_patternGroup = new QGroupBox("🔢 Pattern", this);
  m_patternGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *patternLayout = new QFormLayout(m_patternGroup);
  m_patternTypeCombo = new QComboBox(this);
  m_patternTypeCombo->addItems({"Linear", "Circular"});
  patternLayout->addRow("Type:", m_patternTypeCombo);
  m_patternCountSpin = new QSpinBox(this);
  m_patternCountSpin->setRange(2, 100);
  m_patternCountSpin->setValue(3);
  patternLayout->addRow("Count:", m_patternCountSpin);
  m_patternSpacingSpin = new QDoubleSpinBox(this);
  m_patternSpacingSpin->setRange(0.1, 1000.0);
  m_patternSpacingSpin->setValue(20.0);
  m_patternSpacingSpin->setSuffix(" mm");
  patternLayout->addRow("Spacing:", m_patternSpacingSpin);
  m_patternAngleSpin = new QDoubleSpinBox(this);
  m_patternAngleSpin->setRange(1.0, 360.0);
  m_patternAngleSpin->setValue(360.0);
  m_patternAngleSpin->setSuffix("°");
  patternLayout->addRow("Total Angle:", m_patternAngleSpin);
  mainLayout->addWidget(m_patternGroup);
  m_patternGroup->hide();

  // Mirror settings
  m_mirrorGroup = new QGroupBox("🪞 Mirror", this);
  m_mirrorGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *mirrorLayout = new QFormLayout(m_mirrorGroup);
  m_mirrorAxisCombo = new QComboBox(this);
  m_mirrorAxisCombo->addItems({"X Axis", "Y Axis", "Z Axis", "Custom Plane"});
  mirrorLayout->addRow("Axis:", m_mirrorAxisCombo);
  mainLayout->addWidget(m_mirrorGroup);
  m_mirrorGroup->hide();

  // Shell settings
  m_shellGroup = new QGroupBox("🥚 Shell", this);
  m_shellGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *shellLayout = new QFormLayout(m_shellGroup);
  m_shellThicknessSpin = new QDoubleSpinBox(this);
  m_shellThicknessSpin->setRange(0.1, 100.0);
  m_shellThicknessSpin->setValue(2.0);
  m_shellThicknessSpin->setSuffix(" mm");
  shellLayout->addRow("Thickness:", m_shellThicknessSpin);
  mainLayout->addWidget(m_shellGroup);
  m_shellGroup->hide();

  // Dome settings
  m_domeGroup = new QGroupBox("🔶 Dome", this);
  m_domeGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *domeLayout = new QFormLayout(m_domeGroup);
  m_domeHeightSpin = new QDoubleSpinBox(this);
  m_domeHeightSpin->setRange(0.1, 1000.0);
  m_domeHeightSpin->setValue(10.0);
  m_domeHeightSpin->setSuffix(" mm");
  domeLayout->addRow("Height:", m_domeHeightSpin);
  m_domeRatioSpin = new QDoubleSpinBox(this);
  m_domeRatioSpin->setRange(0.1, 5.0);
  m_domeRatioSpin->setValue(1.0);
  domeLayout->addRow("Ratio:", m_domeRatioSpin);
  mainLayout->addWidget(m_domeGroup);
  m_domeGroup->hide();

  // Draft settings
  m_draftGroup = new QGroupBox("📐 Draft", this);
  m_draftGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *draftLayout = new QFormLayout(m_draftGroup);
  m_draftAngleSpin = new QDoubleSpinBox(this);
  m_draftAngleSpin->setRange(0.1, 45.0);
  m_draftAngleSpin->setValue(3.0);
  m_draftAngleSpin->setSuffix("°");
  draftLayout->addRow("Angle:", m_draftAngleSpin);
  m_draftOutwardCheck = new QCheckBox("Outward", this);
  draftLayout->addRow(m_draftOutwardCheck);
  mainLayout->addWidget(m_draftGroup);
  m_draftGroup->hide();

  // Thicken settings
  m_thickenGroup = new QGroupBox("📏 Thicken", this);
  m_thickenGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *thickenLayout = new QFormLayout(m_thickenGroup);
  m_thickenValueSpin = new QDoubleSpinBox(this);
  m_thickenValueSpin->setRange(0.1, 100.0);
  m_thickenValueSpin->setValue(5.0);
  m_thickenValueSpin->setSuffix(" mm");
  thickenLayout->addRow("Thickness:", m_thickenValueSpin);
  mainLayout->addWidget(m_thickenGroup);
  m_thickenGroup->hide();

  // OffsetSurface settings
  m_offsetSurfaceGroup = new QGroupBox("📐 Offset Surface", this);
  m_offsetSurfaceGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *offsetLayout = new QFormLayout(m_offsetSurfaceGroup);
  m_offsetSurfaceValueSpin = new QDoubleSpinBox(this);
  m_offsetSurfaceValueSpin->setRange(-100.0, 100.0);
  m_offsetSurfaceValueSpin->setValue(5.0);
  m_offsetSurfaceValueSpin->setSuffix(" mm");
  offsetLayout->addRow("Offset:", m_offsetSurfaceValueSpin);
  mainLayout->addWidget(m_offsetSurfaceGroup);
  m_offsetSurfaceGroup->hide();

  // New Sketch Plane settings
  m_sketchPlaneGroup = new QGroupBox("📐 Sketch Plane", this);
  m_sketchPlaneGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *sketchPlaneLayout = new QVBoxLayout(m_sketchPlaneGroup);

  // Radio buttons for plane selection
  m_sketchPlaneXY = new QRadioButton("XY Plane (Top)", this);
  m_sketchPlaneXZ = new QRadioButton("XZ Plane (Front)", this);
  m_sketchPlaneYZ = new QRadioButton("YZ Plane (Right)", this);
  m_sketchPlaneFace = new QRadioButton("Select Face", this);
  m_sketchPlaneXY->setChecked(true);

  sketchPlaneLayout->addWidget(m_sketchPlaneXY);
  sketchPlaneLayout->addWidget(m_sketchPlaneXZ);
  sketchPlaneLayout->addWidget(m_sketchPlaneYZ);
  sketchPlaneLayout->addWidget(m_sketchPlaneFace);

  // Offset & Angle Inputs
  auto *sketchOffsetLayout = new QFormLayout();
  m_sketchPlaneOffsetSpin = new QDoubleSpinBox(this);
  m_sketchPlaneOffsetSpin->setRange(-10000.0, 10000.0);
  m_sketchPlaneOffsetSpin->setValue(0.0);
  m_sketchPlaneOffsetSpin->setSuffix(" mm");
  sketchOffsetLayout->addRow("Offset:", m_sketchPlaneOffsetSpin);

  m_sketchPlaneAngleSpin = new QDoubleSpinBox(this);
  m_sketchPlaneAngleSpin->setRange(-360.0, 360.0);
  m_sketchPlaneAngleSpin->setValue(0.0);
  m_sketchPlaneAngleSpin->setSuffix("°");
  sketchOffsetLayout->addRow("Angle:", m_sketchPlaneAngleSpin);

  sketchPlaneLayout->addLayout(sketchOffsetLayout);

  mainLayout->addWidget(m_sketchPlaneGroup);
  m_sketchPlaneGroup->hide();

  // Constraint info
  m_constraintGroup = new QGroupBox("📏 Constraint", this);
  m_constraintGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *constraintLayout = new QVBoxLayout(m_constraintGroup);
  m_constraintTypeLabel = new QLabel("Type: -", this);
  m_constraintInfoLabel =
      new QLabel("Select entities to apply constraint", this);
  m_constraintInfoLabel->setWordWrap(true);
  constraintLayout->addWidget(m_constraintTypeLabel);
  constraintLayout->addWidget(m_constraintInfoLabel);
  mainLayout->addWidget(m_constraintGroup);
  m_constraintGroup->hide();

  // Revolve settings
  m_revolveGroup = new QGroupBox("🔄 Revolve", this);
  m_revolveGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *revolveLayout = new QFormLayout(m_revolveGroup);
  m_revolveAngleSpin = new QDoubleSpinBox(this);
  m_revolveAngleSpin->setRange(1.0, 360.0);
  m_revolveAngleSpin->setValue(360.0);
  m_revolveAngleSpin->setSuffix("°");
  revolveLayout->addRow("Angle:", m_revolveAngleSpin);
  m_revolveAxisCombo = new QComboBox(this);
  m_revolveAxisCombo->addItems({"X Axis", "Y Axis", "Z Axis"});
  m_revolveAxisCombo->setCurrentIndex(1); // Y as default
  revolveLayout->addRow("Axis:", m_revolveAxisCombo);
  mainLayout->addWidget(m_revolveGroup);
  m_revolveGroup->hide();

  // Sweep settings
  m_sweepGroup = new QGroupBox("🔀 Sweep", this);
  m_sweepGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *sweepLayout = new QFormLayout(m_sweepGroup);
  m_sweepSolidCheck = new QCheckBox("Create Solid", this);
  m_sweepSolidCheck->setChecked(true);
  sweepLayout->addRow(m_sweepSolidCheck);
  m_sweepPathLabel = new QLabel("Path: Not selected", this);
  sweepLayout->addRow(m_sweepPathLabel);
  mainLayout->addWidget(m_sweepGroup);
  m_sweepGroup->hide();

  // Loft settings
  m_loftGroup = new QGroupBox("📐 Loft", this);
  m_loftGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *loftLayout = new QFormLayout(m_loftGroup);
  m_loftSolidCheck = new QCheckBox("Create Solid", this);
  m_loftSolidCheck->setChecked(true);
  loftLayout->addRow(m_loftSolidCheck);
  m_loftRuledCheck = new QCheckBox("Ruled Surface", this);
  loftLayout->addRow(m_loftRuledCheck);
  m_loftProfilesList = new QListWidget(this);
  m_loftProfilesList->setSelectionMode(QAbstractItemView::MultiSelection);
  loftLayout->addRow(m_loftProfilesList);
  m_loftProfilesLabel = new QLabel("Profiles: 0 selected", this);
  loftLayout->addRow(m_loftProfilesLabel);

  connect(m_loftProfilesList, &QListWidget::itemSelectionChanged, this,
          [this]() {
            int count = m_loftProfilesList->selectedItems().count();
            m_loftProfilesLabel->setText(
                QString("Profiles: %1 selected").arg(count));
          });
  mainLayout->addWidget(m_loftGroup);
  m_loftGroup->hide();

  // Boolean settings
  m_booleanGroup = new QGroupBox("⊕ Boolean", this);
  m_booleanGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *booleanLayout = new QFormLayout(m_booleanGroup);
  m_booleanOpCombo = new QComboBox(this);
  m_booleanOpCombo->addItems({"Union (Add)", "Subtract (Cut)", "Intersect"});
  booleanLayout->addRow("Operation:", m_booleanOpCombo);
  mainLayout->addWidget(m_booleanGroup);
  m_booleanGroup->hide();

  // Reference Plane settings
  m_refPlaneGroup = new QGroupBox("📐 Reference Plane", this);
  m_refPlaneGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *refPlaneLayout = new QFormLayout(m_refPlaneGroup);
  m_refPlaneTypeCombo = new QComboBox(this);
  m_refPlaneTypeCombo->addItems({"XY Plane (Top)", "XZ Plane (Front)",
                                 "YZ Plane (Right)", "Offset from XY",
                                 "Offset from XZ", "Offset from YZ"});
  refPlaneLayout->addRow("Type:", m_refPlaneTypeCombo);
  m_refPlaneOffsetSpin = new QDoubleSpinBox(this);
  m_refPlaneOffsetSpin->setRange(-10000.0, 10000.0);
  m_refPlaneOffsetSpin->setValue(50.0);
  m_refPlaneOffsetSpin->setSuffix(" mm");
  refPlaneLayout->addRow("Offset:", m_refPlaneOffsetSpin);
  mainLayout->addWidget(m_refPlaneGroup);
  m_refPlaneGroup->hide();

  // Split settings
  m_splitGroup = new QGroupBox("✂ Split", this);
  m_splitGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *splitLayout = new QFormLayout(m_splitGroup);
  m_splitPlaneCombo = new QComboBox(this);
  m_splitPlaneCombo->addItems({"XY Plane", "XZ Plane", "YZ Plane"});
  splitLayout->addRow("Plane:", m_splitPlaneCombo);
  m_splitOffsetSpin = new QDoubleSpinBox(this);
  m_splitOffsetSpin->setRange(-10000.0, 10000.0);
  m_splitOffsetSpin->setValue(0.0);
  m_splitOffsetSpin->setSuffix(" mm");
  splitLayout->addRow("Offset:", m_splitOffsetSpin);
  m_splitKeepPartCombo = new QComboBox(this);
  m_splitKeepPartCombo->addItems({"Keep Both", "Keep Above", "Keep Below"});
  splitLayout->addRow("Keep:", m_splitKeepPartCombo);
  mainLayout->addWidget(m_splitGroup);
  mainLayout->addWidget(m_splitGroup);
  m_splitGroup->hide();

  // Gear Settings
  m_gearGroup = new QGroupBox("⚙️ Gear", this);
  m_gearGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *gearLayout = new QFormLayout(m_gearGroup);

  m_gearModuleSpin = new QDoubleSpinBox(this);
  m_gearModuleSpin->setRange(0.1, 100.0);
  m_gearModuleSpin->setValue(1.0);
  m_gearModuleSpin->setSingleStep(0.1);
  gearLayout->addRow("Module:", m_gearModuleSpin);

  m_gearTeethSpin = new QSpinBox(this);
  m_gearTeethSpin->setRange(3, 500);
  m_gearTeethSpin->setValue(20);
  gearLayout->addRow("Teeth:", m_gearTeethSpin);

  m_gearPressureAngleSpin = new QDoubleSpinBox(this);
  m_gearPressureAngleSpin->setRange(1.0, 45.0);
  m_gearPressureAngleSpin->setValue(20.0);
  m_gearPressureAngleSpin->setSuffix("°");
  gearLayout->addRow("Pressure Angle:", m_gearPressureAngleSpin);

  m_gearThicknessSpin = new QDoubleSpinBox(this);
  m_gearThicknessSpin->setRange(0.1, 1000.0);
  m_gearThicknessSpin->setValue(5.0);
  m_gearThicknessSpin->setSuffix(" mm");
  gearLayout->addRow("Thickness:", m_gearThicknessSpin);

  mainLayout->addWidget(m_gearGroup);
  m_gearGroup->hide();

  // Hole settings
  m_holeGroup = new QGroupBox("🕳️ Hole Settings", this);
  m_holeGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
  auto *holeLayout = new QFormLayout(m_holeGroup);

  // Hole type
  m_holeTypeCombo = new QComboBox(this);
  m_holeTypeCombo->addItems({"Simple", "Counterbore", "Countersink", "Tapped"});
  holeLayout->addRow("Hole Type:", m_holeTypeCombo);

  // Diameter
  m_holeDiameterSpin = new QDoubleSpinBox(this);
  m_holeDiameterSpin->setRange(0.1, 1000.0);
  m_holeDiameterSpin->setValue(10.0);
  m_holeDiameterSpin->setSingleStep(0.5);
  m_holeDiameterSpin->setSuffix(" mm");
  holeLayout->addRow("Diameter:", m_holeDiameterSpin);

  // Depth
  m_holeDepthSpin = new QDoubleSpinBox(this);
  m_holeDepthSpin->setRange(0.1, 1000.0);
  m_holeDepthSpin->setValue(20.0);
  m_holeDepthSpin->setSingleStep(1.0);
  m_holeDepthSpin->setSuffix(" mm");
  holeLayout->addRow("Depth:", m_holeDepthSpin);

  // Counterbore diameter
  m_cboreDiameterSpin = new QDoubleSpinBox(this);
  m_cboreDiameterSpin->setRange(0.1, 1000.0);
  m_cboreDiameterSpin->setValue(18.0);
  m_cboreDiameterSpin->setSingleStep(0.5);
  m_cboreDiameterSpin->setSuffix(" mm");
  holeLayout->addRow("Counterbore Dia:", m_cboreDiameterSpin);

  // Counterbore depth
  m_cboreDepthSpin = new QDoubleSpinBox(this);
  m_cboreDepthSpin->setRange(0.1, 1000.0);
  m_cboreDepthSpin->setValue(5.0);
  m_cboreDepthSpin->setSingleStep(0.5);
  m_cboreDepthSpin->setSuffix(" mm");
  holeLayout->addRow("Counterbore Depth:", m_cboreDepthSpin);

  // Countersink diameter
  m_csinkDiameterSpin = new QDoubleSpinBox(this);
  m_csinkDiameterSpin->setRange(0.1, 1000.0);
  m_csinkDiameterSpin->setValue(18.0);
  m_csinkDiameterSpin->setSingleStep(0.5);
  m_csinkDiameterSpin->setSuffix(" mm");
  holeLayout->addRow("Countersink Dia:", m_csinkDiameterSpin);

  // Countersink angle
  m_csinkAngleSpin = new QDoubleSpinBox(this);
  m_csinkAngleSpin->setRange(1.0, 180.0);
  m_csinkAngleSpin->setValue(82.0);
  m_csinkAngleSpin->setSingleStep(1.0);
  m_csinkAngleSpin->setSuffix(" °");
  holeLayout->addRow("Countersink Angle:", m_csinkAngleSpin);

  // Flip Direction
  m_holeFlipDirectionCheck = new QCheckBox("Flip Direction", this);
  m_holeFlipDirectionCheck->setChecked(false);
  holeLayout->addRow(m_holeFlipDirectionCheck);

  mainLayout->addWidget(m_holeGroup);
  m_holeGroup->hide();

  // Apply button
  m_applyButton = new QPushButton("Apply (Enter)", this);
  m_applyButton->setStyleSheet(
      "QPushButton { background-color: #4CAF50; color: white; padding: 8px; "
      "font-weight: bold; border-radius: 4px; } "
      "QPushButton:hover { background-color: #45a049; }");
  m_applyButton->setShortcut(
      QKeySequence(Qt::Key_Return)); // Enter key shortcut
  m_applyButton->setDefault(true);   // Make it the default button
  mainLayout->addWidget(m_applyButton);
  m_applyButton->hide();

  // Stretch at bottom
  mainLayout->addStretch();

  // Connect signals
  connect(m_autoConstraintCheck, &QCheckBox::toggled, this,
          &ToolSettingsPanel::onAutoConstraintToggled);
  connect(m_toleranceSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &ToolSettingsPanel::onToleranceChanged);
  connect(m_polygonSidesSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &ToolSettingsPanel::onPolygonSidesChanged);
  connect(m_slotWidthSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &ToolSettingsPanel::onSlotWidthChanged);
  connect(m_polygonInscribedCheck, &QCheckBox::toggled, this,
          &ToolSettingsPanel::onPolygonInscribedToggled);
  connect(m_applyButton, &QPushButton::clicked, this, [this]() {
    qDebug() << "=== Apply button CLICKED ===";
    emit applyClicked();
  });

  // Connect all spins to settingsChanged
  connect(m_extrudeDepthSpin,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          &ToolSettingsPanel::onSettingsChanged);
  connect(m_filletRadiusSpin,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          &ToolSettingsPanel::onSettingsChanged);
  connect(m_chamferSizeSpin,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          &ToolSettingsPanel::onSettingsChanged);
  connect(m_shellThicknessSpin,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          &ToolSettingsPanel::onSettingsChanged);
  connect(m_domeHeightSpin,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          &ToolSettingsPanel::onSettingsChanged);
  connect(m_draftAngleSpin,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          &ToolSettingsPanel::onSettingsChanged);
  connect(m_patternCountSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &ToolSettingsPanel::onSettingsChanged);
  connect(m_patternSpacingSpin,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          &ToolSettingsPanel::onSettingsChanged);

  connect(m_revolveAngleSpin,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          &ToolSettingsPanel::onSettingsChanged);

  // Gear signals
  connect(m_gearModuleSpin,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          &ToolSettingsPanel::onSettingsChanged);
  connect(m_gearTeethSpin, QOverload<int>::of(&QSpinBox::valueChanged), this,
          &ToolSettingsPanel::onSettingsChanged);
  connect(m_gearPressureAngleSpin,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          &ToolSettingsPanel::onSettingsChanged);
  connect(m_gearThicknessSpin,
          QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
          &ToolSettingsPanel::onSettingsChanged);
}

void ToolSettingsPanel::setSketchView(SketchView2D *view) {
  m_sketchView = view;
  if (m_sketchView) {
    m_autoConstraintCheck->setChecked(m_sketchView->autoConstraint());
    m_polygonSidesSpin->setValue(m_sketchView->polygonSides());
    m_slotWidthSpin->setValue(m_sketchView->slotWidth());
  }
}

void ToolSettingsPanel::updateForTool(SketchToolType tool) {
  hideAllSettings();

  switch (tool) {
  case SketchToolType::None:
  case SketchToolType::Select:
    m_toolLabel->setText("🔍 Selection Mode");
    m_descriptionLabel->setText(
        "Click to select entities. Shift+Click for multi-select.");
    m_generalGroup->show();
    break;
  case SketchToolType::Line:
    m_toolLabel->setText("🖊️ Line Tool");
    m_descriptionLabel->setText(
        "Click to set start point, click again to set end point.");
    m_generalGroup->show();
    break;
  case SketchToolType::Rectangle:
    m_toolLabel->setText("⬜ Rectangle Tool");
    m_descriptionLabel->setText("Click and drag to create rectangle.");
    m_generalGroup->show();
    break;
  case SketchToolType::Circle:
    m_toolLabel->setText("⭕ Circle Tool");
    m_descriptionLabel->setText("Click for center, drag for radius.");
    m_generalGroup->show();
    break;
  case SketchToolType::Arc:
    m_toolLabel->setText("◠ Arc Tool");
    m_descriptionLabel->setText("Click 3 points: start, through, end.");
    m_generalGroup->show();
    break;
  case SketchToolType::Point:
    m_toolLabel->setText("• Point Tool");
    m_descriptionLabel->setText("Click to place construction points.");
    m_generalGroup->show();
    break;
  case SketchToolType::Spline:
    m_toolLabel->setText("〰️ Spline Tool");
    m_descriptionLabel->setText(
        "Click to add control points. Enter to finish.");
    m_generalGroup->show();
    break;
  case SketchToolType::Ellipse:
    m_toolLabel->setText("⬭ Ellipse Tool");
    m_descriptionLabel->setText("Click center, drag for radii.");
    m_generalGroup->show();
    break;
  case SketchToolType::Polygon:
    m_toolLabel->setText("⬡ Polygon Tool");
    m_descriptionLabel->setText("Set sides below, then click center and drag.");
    showPolygonSettings();
    m_generalGroup->show();
    break;
  case SketchToolType::Slot:
    m_toolLabel->setText("⊂⊃ Slot Tool");
    m_descriptionLabel->setText("Set width below, click start and end points.");
    showSlotSettings();
    m_generalGroup->show();
    break;
  case SketchToolType::Offset:
    m_toolLabel->setText("⟦ Offset Tool");
    m_descriptionLabel->setText("Select entity and specify offset distance.");
    m_generalGroup->show();
    break;
  case SketchToolType::ProfileSelect:
    m_toolLabel->setText("📐 Profile Selection");
    m_descriptionLabel->setText(
        "Click on closed profiles to select for extrude/cut.");
    break;
  default:
    m_toolLabel->setText("No Tool Selected");
    m_descriptionLabel->setText("");
    m_generalGroup->show();
    break;
  }
}

void ToolSettingsPanel::hideAllSettings() {
  m_generalGroup->hide();
  m_polygonGroup->hide();
  m_slotGroup->hide();
  m_extrudeGroup->hide();
  m_cutGroup->hide();
  m_filletGroup->hide();
  m_chamferGroup->hide();
  m_patternGroup->hide();
  m_mirrorGroup->hide();
  m_shellGroup->hide();
  m_domeGroup->hide();
  m_draftGroup->hide();
  m_thickenGroup->hide();
  m_offsetSurfaceGroup->hide();
  m_constraintGroup->hide();
  m_revolveGroup->hide();
  m_sweepGroup->hide();
  m_loftGroup->hide();
  m_booleanGroup->hide();
  m_refPlaneGroup->hide();
  m_splitGroup->hide();
  m_gearGroup->hide();
  if (m_holeGroup)
    m_holeGroup->hide();
  m_sketchPlaneGroup->hide();
  m_constraintGroup->hide();
  m_applyButton->hide();
}

void ToolSettingsPanel::showPolygonSettings() { m_polygonGroup->show(); }
void ToolSettingsPanel::showSlotSettings() { m_slotGroup->show(); }
void ToolSettingsPanel::showLineSettings() {}
void ToolSettingsPanel::showGeneralSettings() { m_generalGroup->show(); }

void ToolSettingsPanel::showExtrudeSettings() {
  hideAllSettings();
  m_toolLabel->setText("📦 Extrude");
  m_descriptionLabel->setText("Select profile and set extrusion parameters.");
  m_extrudeGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showCutSettings() {
  hideAllSettings();
  m_toolLabel->setText("🔪 Cut");
  m_descriptionLabel->setText("Select profile to cut material from solid.");
  m_cutGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showFilletSettings() {
  hideAllSettings();
  m_toolLabel->setText("⭕ Fillet");
  m_descriptionLabel->setText("Round edges with specified radius.");
  m_filletGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showChamferSettings() {
  hideAllSettings();
  m_toolLabel->setText("◢ Chamfer");
  m_descriptionLabel->setText("Create angled cut on edges.");
  m_chamferGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showPatternSettings() {
  hideAllSettings();
  m_toolLabel->setText("🔢 Pattern");
  m_descriptionLabel->setText(
      "Create copies in linear or circular arrangement.");
  m_patternGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showMirrorSettings() {
  hideAllSettings();
  m_toolLabel->setText("🪞 Mirror");
  m_descriptionLabel->setText("Mirror selected entities about an axis.");
  m_mirrorGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showShellSettings() {
  hideAllSettings();
  m_toolLabel->setText("🥚 Shell");
  m_descriptionLabel->setText(
      "Hollow out solid with specified wall thickness.");
  m_shellGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showDomeSettings() {
  hideAllSettings();
  m_toolLabel->setText("🔶 Dome");
  m_descriptionLabel->setText("Add dome/dish to selected face.");
  m_domeGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showDraftSettings() {
  hideAllSettings();
  m_toolLabel->setText("📐 Draft");
  m_descriptionLabel->setText("Add taper angle to selected faces.");
  m_draftGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showThickenSettings() {
  hideAllSettings();
  m_toolLabel->setText("📏 Thicken");
  m_descriptionLabel->setText("Offset surfaces to create solid from shell.");
  m_thickenGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showOffsetSurfaceSettings() {
  hideAllSettings();
  m_toolLabel->setText("📐 Offset Surface");
  m_descriptionLabel->setText("Create offset copy of selected surface.");
  m_offsetSurfaceGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showConstraintSettings() {
  hideAllSettings();
  m_toolLabel->setText("📏 Constraint");
  m_descriptionLabel->setText(
      "Apply geometric constraint to selected entities.");
  m_constraintGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showNoToolSettings() {
  hideAllSettings();
  m_toolLabel->setText("OpenCAD");
  m_descriptionLabel->setText("Select a tool from the toolbar or menu.");
}

void ToolSettingsPanel::showRevolveSettings() {
  hideAllSettings();
  m_toolLabel->setText("🔄 Revolve");
  m_descriptionLabel->setText("Create solid by revolving profile around axis.");
  m_revolveGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showSweepSettings() {
  hideAllSettings();
  m_toolLabel->setText("🔀 Sweep");
  m_descriptionLabel->setText("Create solid by sweeping profile along path.");
  m_sweepGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showLoftSettings() {
  hideAllSettings();
  m_toolLabel->setText("📐 Loft");
  m_descriptionLabel->setText("Create solid by blending between profiles.");
  m_loftGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showBooleanSettings() {
  hideAllSettings();
  m_toolLabel->setText("⊕ Boolean");
  m_descriptionLabel->setText("Combine or subtract solid bodies.");
  m_booleanGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showReferencePlaneSettings() {
  hideAllSettings();
  m_toolLabel->setText("📐 Reference Plane");
  m_descriptionLabel->setText("Create a new reference plane for sketching.");
  m_refPlaneGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showSplitSettings() {
  hideAllSettings();
  m_toolLabel->setText("Split");
  m_descriptionLabel->setText("Split solid into multiple parts");
  m_splitGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showSketchPlaneSettings() {
  hideAllSettings();
  m_toolLabel->setText("New Sketch");
  m_descriptionLabel->setText("Select a plane for the new sketch");
  m_sketchPlaneGroup->show();
  m_applyButton->show();
}

int ToolSettingsPanel::splitPlane() const {
  return m_splitPlaneCombo->currentIndex();
}

double ToolSettingsPanel::splitOffset() const {
  return m_splitOffsetSpin->value();
}

int ToolSettingsPanel::splitKeepPart() const {
  return m_splitKeepPartCombo->currentIndex();
}

// Gear getters
double ToolSettingsPanel::gearModule() const {
  return m_gearModuleSpin->value();
}
int ToolSettingsPanel::gearNumTeeth() const { return m_gearTeethSpin->value(); }
double ToolSettingsPanel::gearPressureAngle() const {
  return m_gearPressureAngleSpin->value();
}
double ToolSettingsPanel::gearThickness() const {
  return m_gearThicknessSpin->value();
}

// Getters
double ToolSettingsPanel::extrudeDepth() const {
  return m_extrudeDepthSpin->value();
}
bool ToolSettingsPanel::extrudeSymmetric() const {
  return m_extrudeSymmetricCheck->isChecked();
}
double ToolSettingsPanel::extrudeDraftAngle() const {
  return m_extrudeDraftSpin->value();
}
double ToolSettingsPanel::filletRadius() const {
  return m_filletRadiusSpin->value();
}
double ToolSettingsPanel::chamferSize() const {
  return m_chamferSizeSpin->value();
}
double ToolSettingsPanel::chamferAngle() const {
  return m_chamferAngleSpin->value();
}
int ToolSettingsPanel::patternCount() const {
  return m_patternCountSpin->value();
}
double ToolSettingsPanel::patternSpacing() const {
  return m_patternSpacingSpin->value();
}
double ToolSettingsPanel::shellThickness() const {
  return m_shellThicknessSpin->value();
}
double ToolSettingsPanel::domeHeight() const {
  return m_domeHeightSpin->value();
}
double ToolSettingsPanel::draftAngle() const {
  return m_draftAngleSpin->value();
}
double ToolSettingsPanel::thickenValue() const {
  return m_thickenValueSpin ? m_thickenValueSpin->value() : 5.0;
}
double ToolSettingsPanel::offsetSurfaceValue() const {
  return m_offsetSurfaceValueSpin ? m_offsetSurfaceValueSpin->value() : 5.0;
}

// Hole Wizard getters
int ToolSettingsPanel::holeType() const {
  return m_holeTypeCombo ? m_holeTypeCombo->currentIndex() : 0;
}
double ToolSettingsPanel::holeDiameter() const {
  return m_holeDiameterSpin ? m_holeDiameterSpin->value() : 10.0;
}
double ToolSettingsPanel::holeDepth() const {
  return m_holeDepthSpin ? m_holeDepthSpin->value() : 20.0;
}
double ToolSettingsPanel::holeCounterboreDiameter() const {
  return m_cboreDiameterSpin ? m_cboreDiameterSpin->value() : 18.0;
}
double ToolSettingsPanel::holeCounterboreDepth() const {
  return m_cboreDepthSpin ? m_cboreDepthSpin->value() : 5.0;
}
double ToolSettingsPanel::holeCountersinkDiameter() const {
  return m_csinkDiameterSpin ? m_csinkDiameterSpin->value() : 18.0;
}
double ToolSettingsPanel::holeCountersinkAngle() const {
  return m_csinkAngleSpin ? m_csinkAngleSpin->value() : 82.0;
}

// Slots
void ToolSettingsPanel::onPolygonSidesChanged(int value) {
  if (m_sketchView)
    m_sketchView->setPolygonSides(value);
  emit polygonSidesChanged(value);
}

void ToolSettingsPanel::onSlotWidthChanged(double value) {
  if (m_sketchView)
    m_sketchView->setSlotWidth(value);
  emit slotWidthChanged(value);
}

void ToolSettingsPanel::onAutoConstraintToggled(bool checked) {
  if (m_sketchView)
    m_sketchView->setAutoConstraint(checked);
  emit autoConstraintEnabledChanged(checked);
}

void ToolSettingsPanel::onToleranceChanged(double value) {
  if (m_sketchView)
    m_sketchView->setAutoConstraintTolerance(value);
  emit autoConstraintToleranceChanged(value);
}

void ToolSettingsPanel::onPolygonInscribedToggled(bool checked) {
  if (m_sketchView)
    m_sketchView->setPolygonInscribed(checked);
  emit polygonInscribedChanged(checked);
}

void ToolSettingsPanel::onSettingsChanged() { emit settingsChanged(); }

// Additional getters
double ToolSettingsPanel::patternAngle() const {
  return m_patternAngleSpin->value();
}

double ToolSettingsPanel::domeRatio() const { return m_domeRatioSpin->value(); }

bool ToolSettingsPanel::draftOutward() const {
  return m_draftOutwardCheck->isChecked();
}

double ToolSettingsPanel::cutDepth() const { return m_cutDepthSpin->value(); }

bool ToolSettingsPanel::cutThroughAll() const {
  return m_cutThroughAllCheck->isChecked();
}

double ToolSettingsPanel::revolveAngle() const {
  return m_revolveAngleSpin->value();
}

int ToolSettingsPanel::revolveAxis() const {
  return m_revolveAxisCombo->currentIndex();
}

bool ToolSettingsPanel::sweepSolid() const {
  return m_sweepSolidCheck->isChecked();
}

bool ToolSettingsPanel::loftSolid() const {
  return m_loftSolidCheck->isChecked();
}

bool ToolSettingsPanel::loftRuled() const {
  return m_loftRuledCheck->isChecked();
}

bool ToolSettingsPanel::holeFlipDirection() const {
  return m_holeFlipDirectionCheck ? m_holeFlipDirectionCheck->isChecked()
                                  : false;
}

int ToolSettingsPanel::booleanOperation() const {
  return m_booleanOpCombo->currentIndex();
}

int ToolSettingsPanel::mirrorAxis() const {
  return m_mirrorAxisCombo->currentIndex();
}

int ToolSettingsPanel::refPlaneType() const {
  return m_refPlaneTypeCombo->currentIndex();
}

double ToolSettingsPanel::refPlaneOffset() const {
  return m_refPlaneOffsetSpin->value();
}

int ToolSettingsPanel::sketchPlaneType() const {
  if (m_sketchPlaneXY->isChecked())
    return 1; // XY
  if (m_sketchPlaneXZ->isChecked())
    return 2; // XZ
  if (m_sketchPlaneYZ->isChecked())
    return 3; // YZ
  if (m_sketchPlaneFace->isChecked())
    return 4; // Face
  return 0;   // None
}

void ToolSettingsPanel::showScaleSettings() {
  hideAllSettings();
  m_toolLabel->setText("⚖️ Scale Feature");
  m_descriptionLabel->setText("Scale the part uniformly or non-uniformly");

  if (!m_scaleGroup) {
    m_scaleGroup = new QGroupBox("⚖️ Scale Settings", this);
    QVBoxLayout *layout = new QVBoxLayout(m_scaleGroup);

    // Scale type
    layout->addWidget(new QLabel("Scale Type:"));
    m_scaleTypeCombo = new QComboBox();
    m_scaleTypeCombo->addItems({"Uniform", "Non-Uniform"});
    layout->addWidget(m_scaleTypeCombo);

    // Uniform scale factor
    layout->addWidget(new QLabel("Scale Factor:"));
    m_scaleFactorSpin = new QDoubleSpinBox();
    m_scaleFactorSpin->setRange(0.01, 100.0);
    m_scaleFactorSpin->setValue(2.0);
    m_scaleFactorSpin->setSingleStep(0.1);
    layout->addWidget(m_scaleFactorSpin);

    // Non-uniform scales
    layout->addWidget(new QLabel("Scale X:"));
    m_scaleXSpin = new QDoubleSpinBox();
    m_scaleXSpin->setRange(0.01, 100.0);
    m_scaleXSpin->setValue(1.0);
    m_scaleXSpin->setSingleStep(0.1);
    layout->addWidget(m_scaleXSpin);

    layout->addWidget(new QLabel("Scale Y:"));
    m_scaleYSpin = new QDoubleSpinBox();
    m_scaleYSpin->setRange(0.01, 100.0);
    m_scaleYSpin->setValue(1.0);
    m_scaleYSpin->setSingleStep(0.1);
    layout->addWidget(m_scaleYSpin);

    layout->addWidget(new QLabel("Scale Z:"));
    m_scaleZSpin = new QDoubleSpinBox();
    m_scaleZSpin->setRange(0.01, 100.0);
    m_scaleZSpin->setValue(1.0);
    m_scaleZSpin->setSingleStep(0.1);
    layout->addWidget(m_scaleZSpin);

    // About centroid
    m_scaleCentroidCheck = new QCheckBox("Scale about centroid");
    m_scaleCentroidCheck->setChecked(true);
    layout->addWidget(m_scaleCentroidCheck);
  }

  m_scaleGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showHoleSettings() {
  hideAllSettings();
  m_toolLabel->setText("🕳️ Hole Wizard");
  m_descriptionLabel->setText("Create holes with various configurations");

  if (m_holeGroup)
    m_holeGroup->show();
  m_applyButton->show();
}

void ToolSettingsPanel::showGearSettings() {
  hideAllSettings();
  m_toolLabel->setText("⚙️ Gear");
  m_descriptionLabel->setText("Create a spur gear with specified parameters.");
  m_gearGroup->show();
  m_applyButton->show();
}

bool ToolSettingsPanel::patternIsLinear() const {
  return m_patternTypeCombo && m_patternTypeCombo->currentIndex() == 0;
}

double ToolSettingsPanel::ribThickness() const {
  return m_ribThicknessSpin ? m_ribThicknessSpin->value() : 0.0;
}

int ToolSettingsPanel::ribType() const {
  return m_ribTypeCombo ? m_ribTypeCombo->currentIndex() : 0;
}

bool ToolSettingsPanel::ribSymmetric() const {
  return m_ribSymmetricCheck ? m_ribSymmetricCheck->isChecked() : false;
}

double ToolSettingsPanel::ribAngle() const {
  return m_ribAngleSpin ? m_ribAngleSpin->value() : 0.0;
}

bool ToolSettingsPanel::ribFlipDirection() const {
  return m_ribFlipCheck ? m_ribFlipCheck->isChecked() : false;
}

double ToolSettingsPanel::ribDraftAngle() const {
  return m_ribDraftSpin ? m_ribDraftSpin->value() : 0.0;
}

void ToolSettingsPanel::setSweepProfileText(const QString &text) {
  if (m_sweepProfileLabel)
    m_sweepProfileLabel->setText(text);
}

void ToolSettingsPanel::setSweepPathText(const QString &text) {
  if (m_sweepPathLabel)
    m_sweepPathLabel->setText(text);
}

void ToolSettingsPanel::populateSweepPathSketches(const QStringList &names) {
  if (m_sweepPathCombo) {
    m_sweepPathCombo->clear();
    m_sweepPathCombo->addItems(names);
  }
}

int ToolSettingsPanel::sweepPathSketchIndex() const {
  return m_sweepPathCombo ? m_sweepPathCombo->currentIndex() : -1;
}

void ToolSettingsPanel::populateLoftSketches(const QStringList &sketches) {
  if (m_loftProfilesList) {
    m_loftProfilesList->clear();
    m_loftProfilesList->addItems(sketches);
  }
}

std::vector<int> ToolSettingsPanel::loftSelectedSketches() const {
  std::vector<int> selected;
  if (!m_loftProfilesList)
    return selected;
  for (int i = 0; i < m_loftProfilesList->count(); ++i) {
    if (m_loftProfilesList->item(i)->isSelected()) {
      selected.push_back(i);
    }
  }
  return selected;
}
void ToolSettingsPanel::setRevolveAxis(int index) {
  if (m_revolveAxisCombo)
    m_revolveAxisCombo->setCurrentIndex(index);
}

void ToolSettingsPanel::showRibSettings() {
  hideAllSettings();
  m_toolLabel->setText("Rib");
  m_descriptionLabel->setText("Create a rib from a sketch profile.");
  if (m_ribGroup)
    m_ribGroup->show();
  m_applyButton->show();
}

double ToolSettingsPanel::sketchPlaneOffsetDistance() const {
  return m_sketchPlaneOffsetSpin ? m_sketchPlaneOffsetSpin->value() : 0.0;
}

double ToolSettingsPanel::sketchPlaneAngle() const {
  return m_sketchPlaneAngleSpin ? m_sketchPlaneAngleSpin->value() : 0.0;
}

} // namespace ui
} // namespace opencad
