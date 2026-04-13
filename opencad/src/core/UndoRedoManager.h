/**
 * @file UndoRedoManager.h
 * @brief Simple and working Undo/Redo system
 */

#pragma once

#include <TopoDS_Shape.hxx>
#include <deque>
#include <memory>
#include <string>
#include <vector>


namespace opencad {
namespace core {

/**
 * @struct Snapshot
 * @brief Base class for document snapshots
 */
struct Snapshot {
  std::string description;
  std::vector<std::string> featureListItems; // Added for UI synchronization
  virtual ~Snapshot() = default;
};

struct ShapeSnapshot : Snapshot {
  std::vector<TopoDS_Shape> shapes;
};

/**
 * @class UndoRedoManager
 * @brief Manages undo/redo operations for document state
 */
class UndoRedoManager {
public:
  explicit UndoRedoManager(size_t maxHistory = 50);

  /// Save current state as a checkpoint
  void checkpoint(std::shared_ptr<Snapshot> snapshot);

  /// Undo to previous state, returns the snapshot to restore
  std::shared_ptr<Snapshot> undo();

  /// Redo to next state, returns the snapshot to restore
  std::shared_ptr<Snapshot> redo();

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
  std::deque<std::shared_ptr<Snapshot>> m_history;
  int m_currentIndex = -1; // Points to current state in history
  size_t m_maxHistory;
};

} // namespace core
} // namespace opencad
