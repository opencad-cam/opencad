/**
 * @file PropertiesPanel.cpp
 * @brief Implementation of PropertiesPanel
 */

#include "PropertiesPanel.h"
#include "sketch/Sketch.h"

#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepGProp.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <QFont>
#include <ShapeAnalysis.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <TopExp_Explorer.hxx>

namespace opencad {
namespace ui {

PropertiesPanel::PropertiesPanel(QWidget *parent) : QWidget(parent) {
  setupUI();
}

void PropertiesPanel::setupUI() {
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(6);
  layout->setContentsMargins(8, 8, 8, 8);

  // Title
  auto *titleLabel = new QLabel("<b>📊 Properties</b>");
  titleLabel->setStyleSheet("color: #00bcd4; font-size: 14px;");
  layout->addWidget(titleLabel);

  // ===== Shape Properties Group =====
  m_shapeGroup = new QGroupBox("3D Shape");
  auto *shapeLayout = new QVBoxLayout(m_shapeGroup);
  shapeLayout->setSpacing(4);

  m_shapeTypeLabel = new QLabel("Type: -");
  m_faceCountLabel = new QLabel("Faces: -");
  m_edgeCountLabel = new QLabel("Edges: -");
  m_volumeLabel = new QLabel("Volume: -");
  m_areaLabel = new QLabel("Area: -");
  m_comLabel = new QLabel("CoM: -");
  m_bboxLabel = new QLabel("Size: -");

  m_shapeTypeLabel->setStyleSheet("color: #e0e0e0;");
  m_faceCountLabel->setStyleSheet("color: #b0b0b0;");
  m_edgeCountLabel->setStyleSheet("color: #b0b0b0;");
  m_volumeLabel->setStyleSheet("color: #4caf50;");
  m_areaLabel->setStyleSheet("color: #ff9800;");
  m_comLabel->setStyleSheet("color: #2196f3;");
  m_bboxLabel->setStyleSheet("color: #9c27b0;");

  shapeLayout->addWidget(m_shapeTypeLabel);
  shapeLayout->addWidget(m_faceCountLabel);
  shapeLayout->addWidget(m_edgeCountLabel);
  shapeLayout->addWidget(m_volumeLabel);
  shapeLayout->addWidget(m_areaLabel);
  shapeLayout->addWidget(m_comLabel);
  shapeLayout->addWidget(m_bboxLabel);
  layout->addWidget(m_shapeGroup);

  // ===== Sketch Properties Group =====
  m_sketchGroup = new QGroupBox("Sketch (2D)");
  auto *sketchLayout = new QVBoxLayout(m_sketchGroup);
  sketchLayout->setSpacing(4);

  m_sketchEntityCountLabel = new QLabel("Entities: -");
  m_sketchPerimeterLabel = new QLabel("Perimeter: -");
  m_sketchAreaLabel = new QLabel("Enclosed Area: -");

  m_sketchEntityCountLabel->setStyleSheet("color: #e0e0e0;");
  m_sketchPerimeterLabel->setStyleSheet("color: #03a9f4;");
  m_sketchAreaLabel->setStyleSheet("color: #8bc34a;");

  sketchLayout->addWidget(m_sketchEntityCountLabel);
  sketchLayout->addWidget(m_sketchPerimeterLabel);
  sketchLayout->addWidget(m_sketchAreaLabel);

  // Entity list with lengths
  auto *entityListLabel = new QLabel("Entity Lengths:");
  entityListLabel->setStyleSheet("color: #888; font-size: 11px;");
  sketchLayout->addWidget(entityListLabel);

  m_entityList = new QListWidget();
  m_entityList->setMaximumHeight(120);
  m_entityList->setStyleSheet(R"(
    QListWidget {
      background-color: #2a2a2a;
      border: 1px solid #444;
      border-radius: 4px;
      color: #ccc;
      font-size: 11px;
    }
    QListWidget::item {
      padding: 2px;
    }
  )");
  sketchLayout->addWidget(m_entityList);
  layout->addWidget(m_sketchGroup);

  layout->addStretch();

  // Dark theme styling
  setStyleSheet(R"(
    QGroupBox {
      font-weight: bold;
      border: 1px solid #444;
      border-radius: 4px;
      margin-top: 10px;
      padding-top: 10px;
      color: #888;
    }
    QGroupBox::title {
      subcontrol-origin: margin;
      left: 8px;
      padding: 0 4px;
    }
  )");

  // Initially hide both groups
  m_shapeGroup->hide();
  m_sketchGroup->hide();
}

void PropertiesPanel::setShape(const TopoDS_Shape &shape) {
  m_shape = shape;
  m_sketch = nullptr;
  updateShapeDisplay();
  m_shapeGroup->show();
  m_sketchGroup->hide();
}

void PropertiesPanel::setSketch(std::shared_ptr<sketch::Sketch> sketch) {
  m_sketch = sketch;
  updateSketchDisplay();
  m_sketchGroup->show();
  // Keep shape group visible if we have a shape
}

void PropertiesPanel::clear() {
  m_shape = TopoDS_Shape();
  m_sketch = nullptr;

  m_shapeTypeLabel->setText("Type: -");
  m_volumeLabel->setText("Volume: -");
  m_areaLabel->setText("Area: -");
  m_comLabel->setText("CoM: -");
  m_bboxLabel->setText("Size: -");
  m_faceCountLabel->setText("Faces: -");
  m_edgeCountLabel->setText("Edges: -");

  m_sketchEntityCountLabel->setText("Entities: -");
  m_sketchPerimeterLabel->setText("Perimeter: -");
  m_sketchAreaLabel->setText("Enclosed Area: -");
  m_entityList->clear();

  m_shapeGroup->hide();
  m_sketchGroup->hide();
}

void PropertiesPanel::updateShapeDisplay() {
  if (m_shape.IsNull()) {
    return;
  }

  // Shape Type
  QString typeStr;
  switch (m_shape.ShapeType()) {
  case TopAbs_SOLID:
    typeStr = "Solid";
    break;
  case TopAbs_SHELL:
    typeStr = "Shell";
    break;
  case TopAbs_FACE:
    typeStr = "Face";
    break;
  case TopAbs_WIRE:
    typeStr = "Wire";
    break;
  case TopAbs_EDGE:
    typeStr = "Edge";
    break;
  case TopAbs_COMPOUND:
    typeStr = "Compound";
    break;
  default:
    typeStr = "Shape";
    break;
  }
  m_shapeTypeLabel->setText("Type: " + typeStr);

  // Count topology
  int faceCount = 0, edgeCount = 0;
  for (TopExp_Explorer exp(m_shape, TopAbs_FACE); exp.More(); exp.Next())
    faceCount++;
  for (TopExp_Explorer exp(m_shape, TopAbs_EDGE); exp.More(); exp.Next())
    edgeCount++;
  m_faceCountLabel->setText(QString("Faces: %1").arg(faceCount));
  m_edgeCountLabel->setText(QString("Edges: %1").arg(edgeCount));

  // Volume
  GProp_GProps volumeProps;
  BRepGProp::VolumeProperties(m_shape, volumeProps);
  double volume = volumeProps.Mass();
  m_volumeLabel->setText(QString("Volume: %1 mm³").arg(volume, 0, 'f', 2));

  // Surface Area
  GProp_GProps surfProps;
  BRepGProp::SurfaceProperties(m_shape, surfProps);
  double area = surfProps.Mass();
  m_areaLabel->setText(QString("Area: %1 mm²").arg(area, 0, 'f', 2));

  // Center of Mass
  gp_Pnt com = volumeProps.CentreOfMass();
  m_comLabel->setText(QString("CoM: (%1, %2, %3)")
                          .arg(com.X(), 0, 'f', 1)
                          .arg(com.Y(), 0, 'f', 1)
                          .arg(com.Z(), 0, 'f', 1));

  // Bounding Box
  Bnd_Box box;
  BRepBndLib::Add(m_shape, box);
  if (!box.IsVoid()) {
    double xMin, yMin, zMin, xMax, yMax, zMax;
    box.Get(xMin, yMin, zMin, xMax, yMax, zMax);
    double dx = xMax - xMin;
    double dy = yMax - yMin;
    double dz = zMax - zMin;
    m_bboxLabel->setText(QString("Size: %1 x %2 x %3")
                             .arg(dx, 0, 'f', 1)
                             .arg(dy, 0, 'f', 1)
                             .arg(dz, 0, 'f', 1));
  }
}

void PropertiesPanel::updateSketchDisplay() {
  if (!m_sketch) {
    return;
  }

  const auto &entities = m_sketch->entities();
  int entityCount = static_cast<int>(entities.size());
  m_sketchEntityCountLabel->setText(QString("Entities: %1").arg(entityCount));

  // Calculate total perimeter (sum of all entity lengths)
  double totalLength = 0.0;
  m_entityList->clear();

  for (size_t i = 0; i < entities.size(); ++i) {
    const auto &entity = entities[i];
    double length = entity->length();
    totalLength += length;

    // Add to list
    QString itemText = QString("%1. %2: %3 mm")
                           .arg(i + 1)
                           .arg(QString::fromStdString(entity->typeName()))
                           .arg(length, 0, 'f', 2);
    m_entityList->addItem(itemText);
  }

  m_sketchPerimeterLabel->setText(
      QString("Perimeter: %1 mm").arg(totalLength, 0, 'f', 2));

  // Calculate enclosed area if sketch is closed
  double enclosedArea = 0.0;
  TopoDS_Wire wire = m_sketch->buildWire();
  if (!wire.IsNull() && wire.Closed()) {
    // Use ShapeAnalysis to get enclosed area
    GProp_GProps props;
    BRepGProp::SurfaceProperties(BRepBuilderAPI_MakeFace(wire).Face(), props);
    enclosedArea = props.Mass();
  }

  if (enclosedArea > 0) {
    m_sketchAreaLabel->setText(
        QString("Enclosed Area: %1 mm²").arg(enclosedArea, 0, 'f', 2));
  } else {
    m_sketchAreaLabel->setText("Enclosed Area: (open sketch)");
  }
}

} // namespace ui
} // namespace opencad
