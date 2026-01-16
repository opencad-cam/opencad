/**
 * @file Document.h
 * @brief Central document management class
 *
 * OpenCAD - Modular CAD/CAE Platform
 * Core Module
 */

#pragma once

#include "FeatureTree.h"
#include "Parameter.h"
#include "UndoRedoManager.h"
#include <QDateTime>
#include <QObject>
#include <QString>
#include <TopoDS_Shape.hxx>
#include <memory>
#include <vector>

namespace opencad {
namespace sketch {
class Sketch;
}

namespace core {

/**
 * @class Document
 * @brief Central document management integrating all CAD systems
 *
 * The Document class is the main container for a CAD project, managing:
 * - Feature tree (construction history)
 * - Undo/Redo system
 * - Parameter system
 * - Sketches
 * - Document metadata
 */
class Document : public QObject {
  Q_OBJECT

public:
  explicit Document(QObject *parent = nullptr);
  ~Document() override;

  // Document properties
  QString filePath() const { return m_filePath; }
  void setFilePath(const QString &path);

  QString name() const { return m_name; }
  void setName(const QString &name);

  bool isModified() const { return m_modified; }
  void setModified(bool modified);

  QDateTime creationDate() const { return m_creationDate; }
  QDateTime lastModifiedDate() const { return m_lastModifiedDate; }

  // Feature management
  FeatureTree *featureTree() { return m_featureTree.get(); }
  const FeatureTree *featureTree() const { return m_featureTree.get(); }

  /**
   * @brief Add a feature to the document
   * @param feature Feature to add
   * @param parent Parent feature (nullptr for root)
   * @return True if added successfully
   */
  bool addFeature(std::shared_ptr<Feature> feature, Feature *parent = nullptr);

  /**
   * @brief Remove a feature from the document
   * @param feature Feature to remove
   * @return True if removed successfully
   */
  bool removeFeature(Feature *feature);

  /**
   * @brief Get all shapes from features
   * @return Vector of all valid feature result shapes
   */
  std::vector<TopoDS_Shape> getAllShapes() const;

  // Parameter management
  ParameterManager *parameterManager() { return m_parameterManager.get(); }
  const ParameterManager *parameterManager() const {
    return m_parameterManager.get();
  }

  // Undo/Redo management
  UndoRedoManager *undoRedoManager() { return m_undoRedoManager.get(); }
  const UndoRedoManager *undoRedoManager() const {
    return m_undoRedoManager.get();
  }

  /**
   * @brief Create a checkpoint for undo/redo
   * @param description Description of the operation
   */
  void checkpoint(const QString &description);

  /**
   * @brief Undo last operation
   * @return True if undo succeeded
   */
  bool undo();

  /**
   * @brief Redo last undone operation
   * @return True if redo succeeded
   */
  bool redo();

  bool canUndo() const;
  bool canRedo() const;
  QString undoDescription() const;
  QString redoDescription() const;

  // Sketch management
  void addSketch(std::shared_ptr<sketch::Sketch> sketch);
  void removeSketch(sketch::Sketch *sketch);
  const std::vector<std::shared_ptr<sketch::Sketch>> &sketches() const {
    return m_sketches;
  }

  // Temporary shape storage (for backward compatibility until full Feature
  // migration)
  void addTemporaryShape(const TopoDS_Shape &shape);
  void clearTemporaryShapes();
  std::vector<TopoDS_Shape> &temporaryShapes() { return m_temporaryShapes; }

  // Regeneration
  /**
   * @brief Regenerate all features
   * @return True if regeneration succeeded
   */
  bool regenerate();

  /**
   * @brief Regenerate from a specific feature onwards
   * @param fromFeature Starting feature
   * @return True if regeneration succeeded
   */
  bool regenerateFrom(Feature *fromFeature);

  // File operations
  /**
   * @brief Save document to file
   * @param filePath Path to save to (uses current path if empty)
   * @return True if save succeeded
   */
  bool save(const QString &filePath = QString());

  /**
   * @brief Load document from file
   * @param filePath Path to load from
   * @return True if load succeeded
   */
  bool load(const QString &filePath);

  /**
   * @brief Create a new empty document
   */
  void newDocument();

  /**
   * @brief Close document
   * @return True if close succeeded (may prompt to save)
   */
  bool close();

  // Serialization
  QVariantMap serialize() const;
  bool deserialize(const QVariantMap &data);

signals:
  /**
   * @brief Emitted when document is modified
   */
  void modifiedChanged(bool modified);

  /**
   * @brief Emitted when document is saved
   */
  void documentSaved(const QString &filePath);

  /**
   * @brief Emitted when document is loaded
   */
  void documentLoaded(const QString &filePath);

  /**
   * @brief Emitted when a feature is added
   */
  void featureAdded(Feature *feature);

  /**
   * @brief Emitted when a feature is removed
   */
  void featureRemoved(Feature *feature);

  /**
   * @brief Emitted when regeneration completes
   */
  void regenerationCompleted(bool success);

private slots:
  void onFeatureTreeChanged();
  void onParameterChanged(const QString &name, double value);

private:
  void updateModificationDate();
  void connectSignals();

  // Document metadata
  QString m_filePath;
  QString m_name;
  bool m_modified = false;
  QDateTime m_creationDate;
  QDateTime m_lastModifiedDate;

  // Core systems
  std::unique_ptr<FeatureTree> m_featureTree;
  std::unique_ptr<ParameterManager> m_parameterManager;
  std::unique_ptr<UndoRedoManager> m_undoRedoManager;

  // Sketches (separate from features for now)
  std::vector<std::shared_ptr<sketch::Sketch>> m_sketches;

  // Temporary shape storage (backward compatibility)
  std::vector<TopoDS_Shape> m_temporaryShapes;
};

} // namespace core
} // namespace opencad
