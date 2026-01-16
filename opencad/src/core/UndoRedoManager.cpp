/**
 * @file UndoRedoManager.cpp
 * @brief Simple and working Undo/Redo system implementation
 */

#include "UndoRedoManager.h"
#include <QDebug>

namespace opencad {
namespace core {

UndoRedoManager::UndoRedoManager(size_t maxHistory)
    : m_maxHistory(maxHistory) {}

void UndoRedoManager::checkpoint(const std::vector<TopoDS_Shape> &currentShapes,
                                 const std::string &description) {
  // If we're not at the end of history, remove all future states
  while (m_currentIndex < static_cast<int>(m_history.size()) - 1) {
    m_history.pop_back();
  }

  // Create new snapshot
  ShapeSnapshot snapshot;
  snapshot.shapes = currentShapes; // Copy all shapes
  snapshot.description = description;

  // Add to history
  m_history.push_back(std::move(snapshot));
  m_currentIndex = static_cast<int>(m_history.size()) - 1;

  // Limit history size
  while (m_history.size() > m_maxHistory) {
    m_history.pop_front();
    m_currentIndex--;
  }

  qDebug() << "Checkpoint:" << QString::fromStdString(description)
           << "| Shapes:" << currentShapes.size()
           << "| History:" << m_history.size() << "| Index:" << m_currentIndex
           << "| canUndo:" << canUndo();
}

bool UndoRedoManager::undo(std::vector<TopoDS_Shape> &outShapes,
                           std::string &outDescription) {
  if (!canUndo()) {
    qDebug() << "Cannot undo: index=" << m_currentIndex
             << "size=" << m_history.size();
    return false;
  }

  // Move back one step
  m_currentIndex--;

  // Return the previous state
  outShapes = m_history[m_currentIndex].shapes;
  outDescription = m_history[m_currentIndex + 1].description; // What was undone

  qDebug() << "Undo:" << QString::fromStdString(outDescription)
           << "| Now at index:" << m_currentIndex
           << "| Shapes:" << outShapes.size();

  return true;
}

bool UndoRedoManager::redo(std::vector<TopoDS_Shape> &outShapes,
                           std::string &outDescription) {
  if (!canRedo()) {
    qDebug() << "Cannot redo: index=" << m_currentIndex
             << "size=" << m_history.size();
    return false;
  }

  // Move forward one step
  m_currentIndex++;

  // Return the next state
  outShapes = m_history[m_currentIndex].shapes;
  outDescription = m_history[m_currentIndex].description; // What was redone

  qDebug() << "Redo:" << QString::fromStdString(outDescription)
           << "| Now at index:" << m_currentIndex
           << "| Shapes:" << outShapes.size();

  return true;
}

std::string UndoRedoManager::undoDescription() const {
  if (canUndo() && m_currentIndex >= 0 &&
      m_currentIndex < static_cast<int>(m_history.size())) {
    return m_history[m_currentIndex].description;
  }
  return "";
}

std::string UndoRedoManager::redoDescription() const {
  if (canRedo()) {
    return m_history[m_currentIndex + 1].description;
  }
  return "";
}

void UndoRedoManager::clear() {
  m_history.clear();
  m_currentIndex = -1;
}

} // namespace core
} // namespace opencad
