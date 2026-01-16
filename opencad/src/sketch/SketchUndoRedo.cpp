/**
 * @file SketchUndoRedo.cpp
 * @brief Implementation of sketch undo/redo system
 *
 * OpenCAD - Modular CAD/CAE Platform
 */

#include "SketchUndoRedo.h"
#include "Sketch.h"
#include "entities/SketchArc.h"
#include "entities/SketchCircle.h"
#include "entities/SketchEllipse.h"
#include "entities/SketchLine.h"
#include "entities/SketchPoint.h"
#include "entities/SketchRectangle.h"
#include "entities/SketchSpline.h"

namespace opencad {
namespace sketch {

SketchUndoRedo::SketchUndoRedo(Sketch *sketch, size_t maxHistory)
    : m_sketch(sketch), m_maxHistory(maxHistory), m_currentIndex(-1),
      m_recording(false) {}

void SketchUndoRedo::beginAction(const std::string &description,
                                 SketchActionType type) {
  if (m_recording) {
    // Already recording, cancel previous
    cancelAction();
  }

  m_currentAction = std::make_shared<SketchAction>();
  m_currentAction->type = type;
  m_currentAction->description = description;
  m_recording = true;
}

void SketchUndoRedo::endAction() {
  if (!m_recording || !m_currentAction) {
    return;
  }

  m_recording = false;

  // Remove any redo history (actions after current position)
  while (m_history.size() > static_cast<size_t>(m_currentIndex + 1)) {
    m_history.pop_back();
  }

  // Add new action
  m_history.push_back(m_currentAction);
  m_currentIndex = static_cast<int>(m_history.size()) - 1;

  // Trim history if too large
  while (m_history.size() > m_maxHistory) {
    m_history.pop_front();
    m_currentIndex--;
  }

  m_currentAction.reset();
}

void SketchUndoRedo::cancelAction() {
  m_recording = false;
  m_currentAction.reset();
}

void SketchUndoRedo::recordAddEntity(SketchEntity::Ptr entity) {
  if (!m_recording || !m_currentAction) {
    beginAction("Add Entity", SketchActionType::AddEntity);
  }
  m_currentAction->type = SketchActionType::AddEntity;
  m_currentAction->entity = entity;
}

void SketchUndoRedo::recordRemoveEntity(SketchEntity::Ptr entity) {
  if (!m_recording || !m_currentAction) {
    beginAction("Remove Entity", SketchActionType::RemoveEntity);
  }
  m_currentAction->type = SketchActionType::RemoveEntity;
  m_currentAction->entity = cloneEntity(entity); // Clone to preserve state
}

void SketchUndoRedo::recordEntityBefore(SketchEntity::Ptr entity) {
  if (!m_recording || !m_currentAction) {
    beginAction("Modify Entity", SketchActionType::ModifyEntity);
  }
  m_currentAction->type = SketchActionType::ModifyEntity;
  m_currentAction->entityBefore = cloneEntity(entity);
  m_currentAction->entity = entity; // Keep reference to actual entity
}

void SketchUndoRedo::recordEntityAfter(SketchEntity::Ptr entity) {
  if (!m_recording || !m_currentAction)
    return;
  m_currentAction->entityAfter = cloneEntity(entity);
}

void SketchUndoRedo::recordAddConstraint(Constraint::Ptr constraint) {
  if (!m_recording || !m_currentAction) {
    beginAction("Add Constraint", SketchActionType::AddConstraint);
  }
  m_currentAction->type = SketchActionType::AddConstraint;
  m_currentAction->constraint = constraint;
}

void SketchUndoRedo::recordRemoveConstraint(Constraint::Ptr constraint) {
  if (!m_recording || !m_currentAction) {
    beginAction("Remove Constraint", SketchActionType::RemoveConstraint);
  }
  m_currentAction->type = SketchActionType::RemoveConstraint;
  m_currentAction->constraint = cloneConstraint(constraint);
}

void SketchUndoRedo::recordConstraintChange(Constraint::Ptr constraint,
                                            double oldValue, double newValue) {
  if (!m_recording || !m_currentAction) {
    beginAction("Modify Constraint", SketchActionType::ModifyConstraint);
  }
  m_currentAction->type = SketchActionType::ModifyConstraint;
  m_currentAction->constraint = constraint;
  m_currentAction->constraintValueBefore = oldValue;
  m_currentAction->constraintValueAfter = newValue;
}

void SketchUndoRedo::recordMove(SketchEntity::Ptr entity, double oldX,
                                double oldY, double newX, double newY) {
  if (!m_recording || !m_currentAction) {
    beginAction("Move Entity", SketchActionType::MoveEntity);
  }
  m_currentAction->type = SketchActionType::MoveEntity;
  m_currentAction->entity = entity;
  m_currentAction->oldX = oldX;
  m_currentAction->oldY = oldY;
  m_currentAction->newX = newX;
  m_currentAction->newY = newY;
}

bool SketchUndoRedo::undo() {
  if (!canUndo()) {
    return false;
  }

  auto action = m_history[m_currentIndex];
  applyAction(*action, true); // Apply in reverse
  m_currentIndex--;
  return true;
}

bool SketchUndoRedo::redo() {
  if (!canRedo()) {
    return false;
  }

  m_currentIndex++;
  auto action = m_history[m_currentIndex];
  applyAction(*action, false); // Apply normally
  return true;
}

std::string SketchUndoRedo::undoDescription() const {
  if (!canUndo()) {
    return "";
  }
  return m_history[m_currentIndex]->description;
}

std::string SketchUndoRedo::redoDescription() const {
  if (!canRedo()) {
    return "";
  }
  return m_history[m_currentIndex + 1]->description;
}

void SketchUndoRedo::clear() {
  m_history.clear();
  m_currentIndex = -1;
  m_currentAction.reset();
  m_recording = false;
}

void SketchUndoRedo::applyAction(const SketchAction &action, bool reverse) {
  if (!m_sketch)
    return;

  switch (action.type) {
  case SketchActionType::AddEntity:
    if (reverse) {
      // Undo add = remove
      if (action.entity) {
        m_sketch->removeEntity(action.entity->id());
      }
    } else {
      // Redo add = add back
      if (action.entity) {
        m_sketch->addEntity(action.entity);
      }
    }
    break;

  case SketchActionType::RemoveEntity:
    if (reverse) {
      // Undo remove = add back
      if (action.entity) {
        m_sketch->addEntity(action.entity);
      }
    } else {
      // Redo remove = remove again
      if (action.entity) {
        m_sketch->removeEntity(action.entity->id());
      }
    }
    break;

  case SketchActionType::ModifyEntity:
    if (action.entity && action.entityBefore && action.entityAfter) {
      // Find the entity in sketch and restore state
      auto entity = m_sketch->getEntity(action.entity->id());
      if (entity) {
        auto source = reverse ? action.entityBefore : action.entityAfter;

        // Restore based on entity type
        if (entity->type() == EntityType::Line) {
          auto line = std::dynamic_pointer_cast<SketchLine>(entity);
          auto srcLine = std::dynamic_pointer_cast<SketchLine>(source);
          if (line && srcLine) {
            line->setStartPoint(srcLine->startPoint());
            line->setEndPoint(srcLine->endPoint());
          }
        } else if (entity->type() == EntityType::Circle) {
          auto circle = std::dynamic_pointer_cast<SketchCircle>(entity);
          auto srcCircle = std::dynamic_pointer_cast<SketchCircle>(source);
          if (circle && srcCircle) {
            circle->setCenter(srcCircle->center());
            circle->setRadius(srcCircle->radius());
          }
        } else if (entity->type() == EntityType::Point) {
          auto point = std::dynamic_pointer_cast<SketchPoint>(entity);
          auto srcPoint = std::dynamic_pointer_cast<SketchPoint>(source);
          if (point && srcPoint) {
            point->setX(srcPoint->x());
            point->setY(srcPoint->y());
          }
        } else if (entity->type() == EntityType::Arc) {
          auto arc = std::dynamic_pointer_cast<SketchArc>(entity);
          auto srcArc = std::dynamic_pointer_cast<SketchArc>(source);
          if (arc && srcArc) {
            arc->setCenter(srcArc->center());
            arc->setRadius(srcArc->radius());
            arc->setStartAngle(srcArc->startAngle());
            arc->setEndAngle(srcArc->endAngle());
          }
        }
      }
    }
    break;

  case SketchActionType::AddConstraint:
    if (reverse) {
      // Undo add constraint = remove
      if (action.constraint) {
        m_sketch->removeConstraint(action.constraint->id());
      }
    } else {
      // Redo = add back (need to re-implement constraint adding)
      // For now, just log
    }
    break;

  case SketchActionType::RemoveConstraint:
    if (reverse) {
      // Undo remove = restore constraint
      // Need to re-add constraint to sketch
    } else {
      // Redo remove = remove again
      if (action.constraint) {
        m_sketch->removeConstraint(action.constraint->id());
      }
    }
    break;

  case SketchActionType::ModifyConstraint:
    if (action.constraint) {
      double value =
          reverse ? action.constraintValueBefore : action.constraintValueAfter;
      action.constraint->setDimension(value);
    }
    break;

  case SketchActionType::MoveEntity:
    if (action.entity) {
      double targetX = reverse ? action.oldX : action.newX;
      double targetY = reverse ? action.oldY : action.newY;
      double currentX = reverse ? action.newX : action.oldX;
      double currentY = reverse ? action.newY : action.oldY;

      double dx = targetX - currentX;
      double dy = targetY - currentY;

      // Move the entity
      if (action.entity->type() == EntityType::Point) {
        auto point = std::dynamic_pointer_cast<SketchPoint>(action.entity);
        if (point) {
          point->setX(targetX);
          point->setY(targetY);
        }
      } else if (action.entity->type() == EntityType::Line) {
        auto line = std::dynamic_pointer_cast<SketchLine>(action.entity);
        if (line) {
          gp_Pnt2d start = line->startPoint();
          gp_Pnt2d end = line->endPoint();
          line->setStartPoint(gp_Pnt2d(start.X() + dx, start.Y() + dy));
          line->setEndPoint(gp_Pnt2d(end.X() + dx, end.Y() + dy));
        }
      } else if (action.entity->type() == EntityType::Circle) {
        auto circle = std::dynamic_pointer_cast<SketchCircle>(action.entity);
        if (circle) {
          gp_Pnt2d center = circle->center();
          circle->setCenter(gp_Pnt2d(center.X() + dx, center.Y() + dy));
        }
      }
    }
    break;

  case SketchActionType::Composite:
    // Apply all sub-actions in order (or reverse order for undo)
    if (reverse) {
      for (auto it = action.subActions.rbegin(); it != action.subActions.rend();
           ++it) {
        applyAction(**it, true);
      }
    } else {
      for (const auto &subAction : action.subActions) {
        applyAction(*subAction, false);
      }
    }
    break;
  }

  // Re-solve constraints after any change
  m_sketch->solve();
}

SketchEntity::Ptr SketchUndoRedo::cloneEntity(SketchEntity::Ptr entity) const {
  if (!entity)
    return nullptr;

  switch (entity->type()) {
  case EntityType::Point: {
    auto point = std::dynamic_pointer_cast<SketchPoint>(entity);
    if (point) {
      auto clone = std::make_shared<SketchPoint>(point->x(), point->y());
      clone->setId(point->id());
      return clone;
    }
    break;
  }
  case EntityType::Line: {
    auto line = std::dynamic_pointer_cast<SketchLine>(entity);
    if (line) {
      auto clone =
          std::make_shared<SketchLine>(line->startPoint(), line->endPoint());
      clone->setId(line->id());
      clone->setConstruction(line->isConstruction());
      return clone;
    }
    break;
  }
  case EntityType::Circle: {
    auto circle = std::dynamic_pointer_cast<SketchCircle>(entity);
    if (circle) {
      auto clone =
          std::make_shared<SketchCircle>(circle->center(), circle->radius());
      clone->setId(circle->id());
      return clone;
    }
    break;
  }
  case EntityType::Arc: {
    auto arc = std::dynamic_pointer_cast<SketchArc>(entity);
    if (arc) {
      auto clone = std::make_shared<SketchArc>(
          arc->center(), arc->radius(), arc->startAngle(), arc->endAngle());
      clone->setId(arc->id());
      return clone;
    }
    break;
  }
  default:
    break;
  }

  return nullptr;
}

Constraint::Ptr
SketchUndoRedo::cloneConstraint(Constraint::Ptr constraint) const {
  if (!constraint)
    return nullptr;
  return constraint->clone();
}

} // namespace sketch
} // namespace opencad
