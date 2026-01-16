/**
 * @file SketchUndoRedo.h
 * @brief Undo/Redo system specifically for sketch operations
 *
 * OpenCAD - Modular CAD/CAE Platform
 */
#pragma once

#include "constraints/Constraint.h"
#include "entities/SketchEntity.h"
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <vector>


namespace opencad {
namespace sketch {

class Sketch;

/**
 * @enum SketchActionType
 * @brief Types of sketch actions that can be undone/redone
 */
enum class SketchActionType {
  AddEntity,        // Entity was added
  RemoveEntity,     // Entity was removed
  ModifyEntity,     // Entity was modified (position, size, etc.)
  AddConstraint,    // Constraint was added
  RemoveConstraint, // Constraint was removed
  ModifyConstraint, // Constraint value was changed
  MoveEntity,       // Entity was moved/dragged
  Composite         // Multiple actions grouped together
};

/**
 * @struct SketchAction
 * @brief Represents a single undoable/redoable sketch action
 */
struct SketchAction {
  SketchActionType type;
  std::string description;

  // Entity data (for add/remove/modify entity)
  SketchEntity::Ptr entity;
  SketchEntity::Ptr entityBefore; // State before modification
  SketchEntity::Ptr entityAfter;  // State after modification

  // Constraint data (for add/remove/modify constraint)
  Constraint::Ptr constraint;
  double constraintValueBefore;
  double constraintValueAfter;

  // Composite action (multiple sub-actions)
  std::vector<std::shared_ptr<SketchAction>> subActions;

  // Position data for move operations
  double oldX = 0, oldY = 0;
  double newX = 0, newY = 0;

  SketchAction()
      : type(SketchActionType::AddEntity), constraintValueBefore(0),
        constraintValueAfter(0) {}
};

/**
 * @class SketchUndoRedo
 * @brief Manages undo/redo operations for sketch editing
 *
 * Usage:
 * 1. Call beginAction() before making changes
 * 2. Make changes to the sketch
 * 3. Call endAction() to record the change
 * 4. Use undo()/redo() to navigate history
 */
class SketchUndoRedo {
public:
  explicit SketchUndoRedo(Sketch *sketch, size_t maxHistory = 100);

  // === Action Recording ===

  /**
   * @brief Begin recording a new action
   * @param description Human-readable description
   * @param type Type of action
   */
  void beginAction(const std::string &description, SketchActionType type);

  /**
   * @brief End recording and save to history
   */
  void endAction();

  /**
   * @brief Cancel the current action (don't save to history)
   */
  void cancelAction();

  /**
   * @brief Record entity addition
   */
  void recordAddEntity(SketchEntity::Ptr entity);

  /**
   * @brief Record entity removal
   */
  void recordRemoveEntity(SketchEntity::Ptr entity);

  /**
   * @brief Record entity modification (save before state)
   */
  void recordEntityBefore(SketchEntity::Ptr entity);

  /**
   * @brief Record entity modification (save after state)
   */
  void recordEntityAfter(SketchEntity::Ptr entity);

  /**
   * @brief Record constraint addition
   */
  void recordAddConstraint(Constraint::Ptr constraint);

  /**
   * @brief Record constraint removal
   */
  void recordRemoveConstraint(Constraint::Ptr constraint);

  /**
   * @brief Record constraint value change
   */
  void recordConstraintChange(Constraint::Ptr constraint, double oldValue,
                              double newValue);

  /**
   * @brief Record entity move
   */
  void recordMove(SketchEntity::Ptr entity, double oldX, double oldY,
                  double newX, double newY);

  // === Undo/Redo Operations ===

  /**
   * @brief Undo the last action
   * @return true if undo was successful
   */
  bool undo();

  /**
   * @brief Redo the previously undone action
   * @return true if redo was successful
   */
  bool redo();

  /**
   * @brief Check if undo is available
   */
  bool canUndo() const { return m_currentIndex >= 0; }

  /**
   * @brief Check if redo is available
   */
  bool canRedo() const {
    return m_currentIndex < static_cast<int>(m_history.size()) - 1;
  }

  /**
   * @brief Get description of action that will be undone
   */
  std::string undoDescription() const;

  /**
   * @brief Get description of action that will be redone
   */
  std::string redoDescription() const;

  // === History Management ===

  /**
   * @brief Clear all history
   */
  void clear();

  /**
   * @brief Get current history size
   */
  size_t historySize() const { return m_history.size(); }

  /**
   * @brief Get current position in history
   */
  int currentIndex() const { return m_currentIndex; }

  /**
   * @brief Set maximum history size
   */
  void setMaxHistory(size_t max) { m_maxHistory = max; }

private:
  Sketch *m_sketch;
  std::deque<std::shared_ptr<SketchAction>> m_history;
  int m_currentIndex = -1;
  size_t m_maxHistory;

  // Current action being recorded
  std::shared_ptr<SketchAction> m_currentAction;
  bool m_recording = false;

  // Apply an action (for undo/redo)
  void applyAction(const SketchAction &action, bool reverse);

  // Clone an entity for state preservation
  SketchEntity::Ptr cloneEntity(SketchEntity::Ptr entity) const;

  // Clone a constraint for state preservation
  Constraint::Ptr cloneConstraint(Constraint::Ptr constraint) const;
};

} // namespace sketch
} // namespace opencad
