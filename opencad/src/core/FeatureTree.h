/**
 * @file FeatureTree.h
 * @brief Hierarchical feature tree management
 *
 * OpenCAD - Modular CAD/CAE Platform
 * Core Module
 */

#pragma once

#include "Feature.h"
#include <QObject>
#include <memory>
#include <vector>

namespace opencad {
namespace core {

/**
 * @class FeatureTree
 * @brief Manages hierarchical organization of features
 *
 * The FeatureTree maintains the construction history and manages
 * feature dependencies, ordering, and regeneration.
 */
class FeatureTree : public QObject {
  Q_OBJECT

public:
  explicit FeatureTree(QObject *parent = nullptr);
  ~FeatureTree() override;

  // Feature management
  /**
   * @brief Add a feature to the tree
   * @param feature Feature to add (ownership transferred)
   * @param parent Parent feature (nullptr for root-level features)
   * @return True if added successfully
   */
  bool addFeature(std::shared_ptr<Feature> feature, Feature *parent = nullptr);

  /**
   * @brief Remove a feature from the tree
   * @param feature Feature to remove
   * @return True if removed successfully
   */
  bool removeFeature(Feature *feature);

  /**
   * @brief Move a feature to a new position
   * @param feature Feature to move
   * @param newIndex New index in the feature list
   * @return True if moved successfully
   */
  bool moveFeature(Feature *feature, int newIndex);

  /**
   * @brief Find a feature by name
   * @param name Feature name
   * @return Feature pointer or nullptr if not found
   */
  Feature *findFeature(const QString &name) const;

  /**
   * @brief Find a feature by index
   * @param index Feature index
   * @return Feature pointer or nullptr if invalid index
   */
  Feature *featureAt(int index) const;

  /**
   * @brief Get index of a feature
   * @param feature Feature to find
   * @return Index or -1 if not found
   */
  int indexOf(const Feature *feature) const;

  /**
   * @brief Get all features in order
   * @return List of all features
   */
  std::vector<Feature *> allFeatures() const;

  /**
   * @brief Get root-level features (no parent)
   * @return List of root features
   */
  std::vector<Feature *> rootFeatures() const;

  /**
   * @brief Get number of features
   */
  int featureCount() const { return static_cast<int>(m_features.size()); }

  /**
   * @brief Clear all features
   */
  void clear();

  // Feature operations
  /**
   * @brief Suppress/unsuppress a feature
   * @param feature Feature to modify
   * @param suppressed Suppression state
   */
  void suppressFeature(Feature *feature, bool suppressed);

  /**
   * @brief Set feature visibility
   * @param feature Feature to modify
   * @param visible Visibility state
   */
  void setFeatureVisible(Feature *feature, bool visible);

  // Regeneration
  /**
   * @brief Regenerate all features from a starting point
   * @param fromFeature Starting feature (nullptr = regenerate all)
   * @return True if regeneration succeeded
   */
  bool regenerate(Feature *fromFeature = nullptr);

  /**
   * @brief Regenerate a single feature
   * @param feature Feature to regenerate
   * @return True if regeneration succeeded
   */
  bool regenerateFeature(Feature *feature);

  /**
   * @brief Check if tree needs regeneration
   */
  bool needsRegeneration() const;

  // Dependency analysis
  /**
   * @brief Get features that depend on a given feature
   * @param feature Feature to check
   * @return List of dependent features
   */
  std::vector<Feature *> getDependents(const Feature *feature) const;

  /**
   * @brief Check if reordering would create circular dependency
   * @param feature Feature to move
   * @param newIndex New position
   * @return True if move would be valid
   */
  bool canReorder(const Feature *feature, int newIndex) const;

  // Serialization
  /**
   * @brief Serialize feature tree to JSON-compatible map
   */
  QVariantMap serialize() const;

  /**
   * @brief Deserialize feature tree from JSON-compatible map
   */
  bool deserialize(const QVariantMap &data);

signals:
  /**
   * @brief Emitted when a feature is added
   */
  void featureAdded(Feature *feature);

  /**
   * @brief Emitted when a feature is removed
   */
  void featureRemoved(Feature *feature);

  /**
   * @brief Emitted when a feature is modified
   */
  void featureModified(Feature *feature);

  /**
   * @brief Emitted when tree structure changes
   */
  void treeStructureChanged();

  /**
   * @brief Emitted when regeneration starts
   */
  void regenerationStarted();

  /**
   * @brief Emitted when regeneration completes
   */
  void regenerationCompleted(bool success);

private:
  // Internal helpers
  void buildDependencyOrder();
  std::vector<Feature *> getRegenerationOrder(Feature *fromFeature) const;
  bool hasCircularDependency(const Feature *feature,
                             const Feature *potentialDep) const;

  // Feature storage (ordered list)
  std::vector<std::shared_ptr<Feature>> m_features;

  // Dependency-ordered list (for regeneration)
  std::vector<Feature *> m_dependencyOrder;
};

} // namespace core
} // namespace opencad
