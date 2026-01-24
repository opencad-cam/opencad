/**
 * @file Sketch.h
 * @brief Main Sketch class - 2D paramteric sketch on a plane
 */

#pragma once

#include "SketchPlane.h"
#include "constraints/Constraint.h"
#include "entities/SketchArc.h"
#include "entities/SketchCircle.h"
#include "entities/SketchEllipse.h"
#include "entities/SketchEntity.h"
#include "entities/SketchLine.h"
#include "entities/SketchPoint.h"
#include "entities/SketchPolygon.h"
#include "entities/SketchRectangle.h"
#include "entities/SketchSlot.h"
#include "entities/SketchSpline.h"
#include "solver/ConstraintSolver.h"

#include <TopoDS_Compound.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace opencad {
namespace sketch {

/**
 * @brief Sketch status
 */
enum class SketchStatus {
  Open,   // Sketch is being edited
  Closed, // Sketch is complete
  Invalid // Sketch has errors
};

/**
 * @brief Main Sketch class
 *
 * A sketch is a 2D parametric drawing on a plane that can be used
 * for extrude, revolve, sweep, loft operations.
 */
class Sketch {
public:
  using Ptr = std::shared_ptr<Sketch>;

  Sketch();
  explicit Sketch(const SketchPlane &plane);

  // Name
  std::string name() const { return m_name; }
  void setName(const std::string &name) { m_name = name; }

  // Plane
  const SketchPlane &plane() const { return m_plane; }
  void setPlane(const SketchPlane &plane) { m_plane = plane; }

  // === Entity Management ===

  // Add entities
  SketchPoint::Ptr addPoint(double x, double y);
  SketchLine::Ptr addLine(double x1, double y1, double x2, double y2);
  SketchLine::Ptr addLine(const gp_Pnt2d &start, const gp_Pnt2d &end);
  SketchArc::Ptr addArc(const gp_Pnt2d &center, double radius,
                        double startAngle, double endAngle);
  SketchArc::Ptr addThreePointArc(const gp_Pnt2d &start, const gp_Pnt2d &end,
                                  const gp_Pnt2d &mid);
  SketchCircle::Ptr addCircle(double cx, double cy, double radius);
  SketchCircle::Ptr addCircle(const gp_Pnt2d &center, double radius);
  SketchRectangle::Ptr addRectangle(double x, double y, double width,
                                    double height);
  SketchSpline::Ptr addSpline(const std::vector<gp_Pnt2d> &points,
                              bool closed = false);
  SketchEllipse::Ptr addEllipse(const gp_Pnt2d &center, double majorR,
                                double minorR, double angle = 0);
  SketchPolygon::Ptr addPolygon(const gp_Pnt2d &center, const gp_Pnt2d &vertex,
                                int sides);

  // === Advanced Sketch Features (FAZ 5) ===

  /// Add regular polygon (center, radius, number of sides)
  void addPolygon(const gp_Pnt2d &center, double radius, int sides);

  /// Create offset of selected entities by distance
  void offsetEntities(const std::vector<SketchEntity::Ptr> &entities,
                      double distance);

  /// Create offset of a single wire
  std::vector<SketchEntity::Ptr> offsetWire(const TopoDS_Wire &wire,
                                            double distance);

  // Remove entity
  void removeEntity(uint64_t entityId);
  void removeEntity(SketchEntity::Ptr entity);

  // Get entities
  SketchEntity::Ptr getEntity(uint64_t id) const;
  const std::vector<SketchEntity::Ptr> &entities() const { return m_entities; }
  std::vector<SketchEntity::Ptr> entitiesOfType(EntityType type) const;

  // === Constraint Management ===

  // Add constraints
  void addHorizontal(SketchLine::Ptr line);
  void addVertical(SketchLine::Ptr line);
  void addCoincident(SketchPoint::Ptr p1, SketchPoint::Ptr p2);
  void addCoincident(SketchLine::Ptr line1, int endpoint1,
                     SketchLine::Ptr line2, int endpoint2);
  void addDistance(SketchPoint::Ptr p1, SketchPoint::Ptr p2, double distance);
  void addLength(SketchLine::Ptr line, double length);
  void addRadius(SketchCircle::Ptr circle, double radius);
  void addRadius(SketchArc::Ptr arc, double radius);
  void addAngle(SketchLine::Ptr line1, SketchLine::Ptr line2,
                double angleDegrees);

  // Remove constraint
  void removeConstraint(uint64_t constraintId);
  void removeConstraint(Constraint::Ptr constraint);

  // Add generic constraint (for external constraint creation)
  void addConstraint(Constraint::Ptr constraint);

  // Get constraints
  Constraint::Ptr getConstraint(uint64_t id) const;
  const std::vector<Constraint::Ptr> &constraints() const {
    return m_constraints;
  }

  // Associative constraint update - updates constraints bound to parameters
  void
  updateBoundConstraints(const std::map<std::string, double> &parameterValues);
  // Get all constraints that are bound to parameters
  std::vector<Constraint::Ptr> getBoundConstraints() const;

  // === Solver ===

  SolverStatus solve();
  int remainingDOF() const;
  bool isFullyConstrained() const;

  // === Wire/Face Generation ===

  // Generate closed wire from sketch (for extrude/revolve)
  TopoDS_Wire buildWire() const;

  // Generate face from closed wire
  TopoDS_Face buildFace() const;

  // Check if sketch forms closed profiles
  bool isClosed() const;
  std::vector<TopoDS_Wire> buildAllWires() const;

  // Build compound shape containing all entities (for 3D display)
  TopoDS_Compound buildCompound() const;

  // === Multiple Closed Profile Detection ===

  // Detect all closed profiles in the sketch (returns vector of closed wires)
  std::vector<TopoDS_Wire> detectClosedProfiles() const;

  // Build faces from all closed profiles
  std::vector<TopoDS_Face> buildProfileFaces() const;

  // Get number of closed profiles
  int closedProfileCount() const;

  // === Selection ===

  void selectEntity(SketchEntity::Ptr entity);
  void deselectEntity(SketchEntity::Ptr entity);
  void clearSelection();
  std::vector<SketchEntity::Ptr> selectedEntities() const;

  // === Status ===

  SketchStatus status() const { return m_status; }
  void setStatus(SketchStatus status) { m_status = status; }

  bool isVisible() const;
  void setVisible(bool visible);

  // === Undo/Redo Support ===

  /// Add entity directly (for undo/redo restore)
  void addEntity(SketchEntity::Ptr entity);

  /// Clear all entities (for undo/redo restore)
  void clearEntities() { m_entities.clear(); }

  /// Save current state as a checkpoint
  void saveCheckpoint(const std::string &description = "Edit");

  /// Undo to previous checkpoint
  bool undo();

  /// Redo to next checkpoint
  bool redo();

  /// Check if undo is available
  bool canUndo() const { return m_undoIndex >= 0; }

  /// Check if redo is available
  bool canRedo() const {
    return m_undoIndex < static_cast<int>(m_undoHistory.size()) - 1;
  }

private:
  std::string m_name;
  SketchPlane m_plane;
  std::vector<SketchEntity::Ptr> m_entities;
  std::vector<Constraint::Ptr> m_constraints;
  ConstraintSolver m_solver;
  SketchStatus m_status;
  bool m_visible = true;

  // Undo/Redo history - stores snapshots of entities
  struct Snapshot {
    std::vector<SketchEntity::Ptr> entities;
    std::string description;
  };
  std::vector<Snapshot> m_undoHistory;
  int m_undoIndex = -1;
  static constexpr size_t MaxUndoHistory = 50;
};

} // namespace sketch
} // namespace opencad
