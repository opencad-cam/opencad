/**
 * @file UndoRedoManager.h
 * @brief Simple and working Undo/Redo system
 */

#pragma once

#include <TopoDS_Shape.hxx>
#include <deque>
#include <string>
#include <vector>

namespace opencad {
namespace core {

/**
 * @struct ShapeSnapshot
 * @brief A snapshot of all shapes at a point in time
 */
struct ShapeSnapshot {
  std::vector<TopoDS_Shape> shapes;
  std::string description;
};

/**
 * @class UndoRedoManager
 * @brief Manages undo/redo operations for shape modifications
 *
 * Usage:
 * 1. Call checkpoint() AFTER each operation to save current state
 * 2. Call undo() to restore previous state
 * 3. Call redo() to restore next state
 */
class UndoRedoManager {
public:
  explicit UndoRedoManager(size_t maxHistory = 50);

  /// Save current state as a checkpoint (call AFTER making changes)
  void checkpoint(const std::vector<TopoDS_Shape> &currentShapes,
                  const std::string &description);

  /// Undo to previous state, returns the shapes to restore
  bool undo(std::vector<TopoDS_Shape> &outShapes, std::string &outDescription);

  /// Redo to next state, returns the shapes to restore
  bool redo(std::vector<TopoDS_Shape> &outShapes, std::string &outDescription);

  /// Check if undo is available
  bool canUndo() const { return m_currentIndex > 0; }

  /// Check if redo is available
  bool canRedo() const {
    return m_currentIndex < static_cast<int>(m_history.size()) - 1;
  }

  /// Get description of what will be undone
  std::string undoDescription() const;

  /// Get description of what will be redone
  std::string redoDescription() const;

  /// Clear all history
  void clear();

  /// Get current history size
  size_t historySize() const { return m_history.size(); }

  /// Get current index
  int currentIndex() const { return m_currentIndex; }

private:
  std::deque<ShapeSnapshot> m_history;
  int m_currentIndex = -1; // Points to current state in history
  size_t m_maxHistory;
};

} // namespace core
} // namespace opencad
