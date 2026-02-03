/**
 * @file UndoRedoManager.cpp
 * @brief Simple and working Undo/Redo system implementation
 */

#include "UndoRedoManager.h"
#include <QDebug>
#include <QString>

namespace opencad {
namespace core {

UndoRedoManager::UndoRedoManager(size_t maxHistory)
    : m_maxHistory(maxHistory) {}

void UndoRedoManager::checkpoint(std::shared_ptr<Snapshot> snapshot) {
  // If we're not at the end of history, remove all future states
  while (m_currentIndex < static_cast<int>(m_history.size()) - 1) {
    m_history.pop_back();
  }

  // Add to history
  m_history.push_back(snapshot);
  m_currentIndex = static_cast<int>(m_history.size()) - 1;

  // Limit history size
  while (m_history.size() > m_maxHistory) {
    m_history.pop_front();
    m_currentIndex--;
  }

  qDebug() << "Checkpoint:" << QString::fromStdString(snapshot->description)
           << "| History:" << m_history.size() << "| Index:" << m_currentIndex
           << "| canUndo:" << canUndo();
}

std::shared_ptr<Snapshot> UndoRedoManager::undo() {
  if (!canUndo()) {
    qDebug() << "Cannot undo: index=" << m_currentIndex
             << "size=" << m_history.size();
    return nullptr;
  }

  // Move back one step
  m_currentIndex--;

  // Return the state at current index
  auto snapshot = m_history[m_currentIndex];

  qDebug() << "Undo:"
           << QString::fromStdString(
                  m_history[m_currentIndex + 1]
                      ->description) // Description of action undone
           << "| Now at index:" << m_currentIndex;

  return snapshot;
}

std::shared_ptr<Snapshot> UndoRedoManager::redo() {
  if (!canRedo()) {
    qDebug() << "Cannot redo: index=" << m_currentIndex
             << "size=" << m_history.size();
    return nullptr;
  }

  // Move forward one step
  m_currentIndex++;

  // Return the state at new index
  auto snapshot = m_history[m_currentIndex];

  qDebug() << "Redo:" << QString::fromStdString(snapshot->description)
           << "| Now at index:" << m_currentIndex;

  return snapshot;
}

std::string UndoRedoManager::undoDescription() const {
  if (canUndo() && m_currentIndex >= 0 &&
      m_currentIndex < static_cast<int>(m_history.size())) {
    return m_history[m_currentIndex]->description;
  }
  return "";
}

std::string UndoRedoManager::redoDescription() const {
  if (canRedo()) {
    return m_history[m_currentIndex + 1]->description;
  }
  return "";
}

void UndoRedoManager::clear() {
  m_history.clear();
  m_currentIndex = -1;
}

} // namespace core
} // namespace opencad
