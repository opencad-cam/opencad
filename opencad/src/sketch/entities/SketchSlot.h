/**
 * @file SketchSlot.h
 * @brief Slot entity for sketch (elongated hole)
 *
 * OpenCAD - Modular CAD/CAE Platform
 */
#pragma once

#include "SketchEntity.h"
#include <Geom2d_Curve.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Pnt2d.hxx>
#include <memory>

namespace opencad {
namespace sketch {

/**
 * @enum SlotType
 * @brief Type of slot
 */
enum class SlotType {
  Straight,      // Straight slot with semicircular ends
  Centerpoint,   // Slot defined by center, length, and width
  ThreePointArc, // Arc slot following 3 points
  CenterpointArc // Arc slot with center point
};

/**
 * @class SketchSlot
 * @brief Represents a slot (elongated hole) in a sketch
 *
 * A slot consists of two parallel lines connected by semicircular ends.
 */
class SketchSlot : public SketchEntity {
public:
  using Ptr = std::shared_ptr<SketchSlot>;

  /**
   * @brief Create straight slot from two center points and width
   */
  SketchSlot(const gp_Pnt2d &center1, const gp_Pnt2d &center2, double width);

  /**
   * @brief Create slot from center, length, width, and angle
   */
  SketchSlot(const gp_Pnt2d &center, double length, double width,
             double angle = 0.0);

  // SketchEntity interface
  EntityType type() const override { return EntityType::Slot; }
  std::string typeName() const override { return "Slot"; }
  int baseDOF() const override {
    return 5;
  } // center1 (2) + center2 (2) + width (1)

  Handle(Geom2d_Curve) curve() const override;
  gp_Pnt2d startPoint() const override { return m_center1; }
  gp_Pnt2d endPoint() const override { return m_center2; }
  gp_Pnt2d midPoint() const override { return center(); }
  double length() const override;

  int parameterCount() const override { return 5; }
  double getParameter(int index) const override;
  void setParameter(int index, double value) override;

  SketchEntity::Ptr clone() const override;
  bool isValid() const override { return m_width > 0; }

  // Slot-specific methods
  gp_Pnt2d center1() const { return m_center1; }
  gp_Pnt2d center2() const { return m_center2; }
  double width() const { return m_width; }
  double slotLength() const;
  double angle() const;
  gp_Pnt2d center() const;

  void setCenter1(const gp_Pnt2d &center1) { m_center1 = center1; }
  void setCenter2(const gp_Pnt2d &center2) { m_center2 = center2; }
  void setWidth(double width) { m_width = width; }

  SlotType slotType() const { return m_slotType; }
  void setSlotType(SlotType type) { m_slotType = type; }

  // Get corner points (outer boundary)
  std::vector<gp_Pnt2d> cornerPoints() const;

  // Get the outline wire
  TopoDS_Wire buildWire() const;

  // Get as shape
  TopoDS_Shape toShape() const;

private:
  gp_Pnt2d m_center1; // Center of first semicircle
  gp_Pnt2d m_center2; // Center of second semicircle
  double m_width;     // Width (diameter of semicircles)
  SlotType m_slotType;
};

} // namespace sketch
} // namespace opencad
