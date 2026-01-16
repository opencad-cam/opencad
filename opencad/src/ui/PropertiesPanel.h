/**
 * @file PropertiesPanel.h
 * @brief Properties panel showing geometry measurements (NX-style Associative)
 */

#pragma once

#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <TopoDS_Shape.hxx>


// Forward declarations
namespace opencad {
namespace sketch {
class Sketch;
}
} // namespace opencad

namespace opencad {
namespace ui {

/**
 * @brief Panel showing mass properties of selected geometry
 *
 * NX-style associative properties that update automatically when geometry
 * changes. Shows: Volume, Surface Area, Center of Mass, Bounding Box dimensions
 * Also shows sketch properties: perimeter, area, entity lengths
 */
class PropertiesPanel : public QWidget {
  Q_OBJECT

public:
  explicit PropertiesPanel(QWidget *parent = nullptr);
  ~PropertiesPanel() override = default;

  // Update properties for a shape
  void setShape(const TopoDS_Shape &shape);

  // Update properties for a sketch (associative)
  void setSketch(std::shared_ptr<sketch::Sketch> sketch);

  // Clear all properties
  void clear();

private:
  void setupUI();
  void updateShapeDisplay();
  void updateSketchDisplay();

  // Shape data
  TopoDS_Shape m_shape;
  std::shared_ptr<sketch::Sketch> m_sketch;

  // UI Elements - Shape
  QGroupBox *m_shapeGroup;
  QLabel *m_shapeTypeLabel;
  QLabel *m_volumeLabel;
  QLabel *m_areaLabel;
  QLabel *m_comLabel;  // Center of Mass
  QLabel *m_bboxLabel; // Bounding Box
  QLabel *m_edgeCountLabel;
  QLabel *m_faceCountLabel;

  // UI Elements - Sketch
  QGroupBox *m_sketchGroup;
  QLabel *m_sketchEntityCountLabel;
  QLabel *m_sketchPerimeterLabel;
  QLabel *m_sketchAreaLabel;
  QListWidget *m_entityList;
};

} // namespace ui
} // namespace opencad
