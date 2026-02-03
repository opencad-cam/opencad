/**
 * @file SketchEntity.h
 * @brief Base class for all 2D sketch entities
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <Geom2d_Curve.hxx>
#include <gp_Pnt2d.hxx>

namespace opencad {
namespace sketch {

// Forward declarations
class Constraint;
class Sketch;

/**
 * @brief Entity type enumeration
 */
enum class EntityType {
  Point,
  Line,
  Arc,
  Circle,
  Rectangle,
  Spline,
  Ellipse,
  Polygon,
  Slot
};

/**
 * @brief Base class for all sketch entities
 */
class SketchEntity {
public:
  using Ptr = std::shared_ptr<SketchEntity>;

  SketchEntity();
  virtual ~SketchEntity() = default;

  // Identification
  uint64_t id() const { return m_id; }
  void setId(uint64_t id) { m_id = id; }

  // Type
  virtual EntityType type() const = 0;
  virtual std::string typeName() const = 0;

  // Geometry
  virtual Handle(Geom2d_Curve) curve() const = 0;
  virtual gp_Pnt2d startPoint() const = 0;
  virtual gp_Pnt2d endPoint() const = 0;
  virtual gp_Pnt2d midPoint() const = 0;
  virtual double length() const = 0;

  // Construction mode
  bool isConstruction() const { return m_isConstruction; }
  void setConstruction(bool construction) { m_isConstruction = construction; }

  // Selection state
  bool isSelected() const { return m_isSelected; }
  void setSelected(bool selected) { m_isSelected = selected; }

  // Constraint tracking
  const std::vector<std::weak_ptr<Constraint>> &constraints() const {
    return m_constraints;
  }
  void addConstraint(std::shared_ptr<Constraint> constraint);
  void removeConstraint(std::shared_ptr<Constraint> constraint);

  // Degrees of freedom (before constraints)
  virtual int baseDOF() const = 0;

  // Parameter access for solver
  virtual int parameterCount() const = 0;
  virtual double getParameter(int index) const = 0;
  virtual void setParameter(int index, double value) = 0;

  // Clone
  virtual Ptr clone() const = 0;

  // Validation
  virtual bool isValid() const = 0;

protected:
  uint64_t m_id;
  bool m_isConstruction;
  bool m_isSelected;
  std::vector<std::weak_ptr<Constraint>> m_constraints;
  std::string m_sourceReference; // Added for Linked Geometry

public:
  void setSourceReference(const std::string &ref) { m_sourceReference = ref; }
  std::string sourceReference() const { return m_sourceReference; }

  static uint64_t s_nextId;
};

} // namespace sketch
} // namespace opencad
