/**
 * @file FeatureTree.cpp
 * @brief Hierarchical feature tree management - Implementation
 */

#include "FeatureTree.h"
#include <algorithm>
#include <set>

namespace opencad {
namespace core {

FeatureTree::FeatureTree(QObject *parent) : QObject(parent) {}

FeatureTree::~FeatureTree() { clear(); }

bool FeatureTree::addFeature(std::shared_ptr<Feature> feature,
                             Feature *parent) {
  if (!feature) {
    return false;
  }

  // Check for duplicate names
  if (findFeature(feature->name())) {
    return false;
  }

  // Set parent relationship
  if (parent) {
    feature->setParent(parent);
    parent->addChild(feature.get());
  }

  // Add to feature list
  m_features.push_back(feature);

  // Rebuild dependency order
  buildDependencyOrder();

  emit featureAdded(feature.get());
  emit treeStructureChanged();

  return true;
}

bool FeatureTree::removeFeature(Feature *feature) {
  if (!feature) {
    return false;
  }

  // Find and remove the feature
  auto it = std::find_if(m_features.begin(), m_features.end(),
                         [feature](const std::shared_ptr<Feature> &f) {
                           return f.get() == feature;
                         });

  if (it == m_features.end()) {
    return false;
  }

  // Remove from parent
  if (feature->parent()) {
    feature->parent()->removeChild(feature);
  }

  // Remove children's parent reference
  for (auto *child : feature->children()) {
    child->setParent(nullptr);
  }

  // Remove from list
  m_features.erase(it);

  // Rebuild dependency order
  buildDependencyOrder();

  emit featureRemoved(feature);
  emit treeStructureChanged();

  return true;
}

bool FeatureTree::moveFeature(Feature *feature, int newIndex) {
  if (!feature || newIndex < 0 ||
      newIndex >= static_cast<int>(m_features.size())) {
    return false;
  }

  // Check if move would create circular dependency
  if (!canReorder(feature, newIndex)) {
    return false;
  }

  // Find current position
  auto it = std::find_if(m_features.begin(), m_features.end(),
                         [feature](const std::shared_ptr<Feature> &f) {
                           return f.get() == feature;
                         });

  if (it == m_features.end()) {
    return false;
  }

  // Move the feature
  auto featurePtr = *it;
  m_features.erase(it);
  m_features.insert(m_features.begin() + newIndex, featurePtr);

  // Rebuild dependency order
  buildDependencyOrder();

  emit treeStructureChanged();

  return true;
}

Feature *FeatureTree::findFeature(const QString &name) const {
  auto it = std::find_if(
      m_features.begin(), m_features.end(),
      [&name](const std::shared_ptr<Feature> &f) { return f->name() == name; });

  return (it != m_features.end()) ? it->get() : nullptr;
}

Feature *FeatureTree::featureAt(int index) const {
  if (index < 0 || index >= static_cast<int>(m_features.size())) {
    return nullptr;
  }
  return m_features[index].get();
}

int FeatureTree::indexOf(const Feature *feature) const {
  auto it = std::find_if(m_features.begin(), m_features.end(),
                         [feature](const std::shared_ptr<Feature> &f) {
                           return f.get() == feature;
                         });

  return (it != m_features.end())
             ? static_cast<int>(std::distance(m_features.begin(), it))
             : -1;
}

std::vector<Feature *> FeatureTree::allFeatures() const {
  std::vector<Feature *> result;
  result.reserve(m_features.size());
  for (const auto &f : m_features) {
    result.push_back(f.get());
  }
  return result;
}

std::vector<Feature *> FeatureTree::rootFeatures() const {
  std::vector<Feature *> result;
  for (const auto &f : m_features) {
    if (!f->parent()) {
      result.push_back(f.get());
    }
  }
  return result;
}

void FeatureTree::clear() {
  m_features.clear();
  m_dependencyOrder.clear();
  emit treeStructureChanged();
}

void FeatureTree::suppressFeature(Feature *feature, bool suppressed) {
  if (feature) {
    feature->setSuppressed(suppressed);
    emit featureModified(feature);
  }
}

void FeatureTree::setFeatureVisible(Feature *feature, bool visible) {
  if (feature) {
    feature->setVisible(visible);
    emit featureModified(feature);
  }
}

bool FeatureTree::regenerate(Feature *fromFeature) {
  emit regenerationStarted();

  // Get features to regenerate in dependency order
  auto features = getRegenerationOrder(fromFeature);

  bool success = true;
  for (auto *feature : features) {
    if (!feature->isSuppressed() && feature->needsRegeneration()) {
      if (!feature->regenerate()) {
        success = false;
        // Continue regenerating other features even if one fails
      }
    }
  }

  emit regenerationCompleted(success);
  return success;
}

bool FeatureTree::regenerateFeature(Feature *feature) {
  if (!feature) {
    return false;
  }

  emit regenerationStarted();
  bool success = feature->regenerate();
  emit regenerationCompleted(success);

  return success;
}

bool FeatureTree::needsRegeneration() const {
  for (const auto &f : m_features) {
    if (f->needsRegeneration() && !f->isSuppressed()) {
      return true;
    }
  }
  return false;
}

std::vector<Feature *>
FeatureTree::getDependents(const Feature *feature) const {
  std::vector<Feature *> dependents;

  for (const auto &f : m_features) {
    if (f->dependsOn(feature)) {
      dependents.push_back(f.get());
    }
  }

  return dependents;
}

bool FeatureTree::canReorder(const Feature *feature, int newIndex) const {
  if (!feature || newIndex < 0 ||
      newIndex >= static_cast<int>(m_features.size())) {
    return false;
  }

  // Check if any features before newIndex depend on this feature
  for (int i = 0; i < newIndex; ++i) {
    if (m_features[i]->dependsOn(feature)) {
      return false; // Would create invalid dependency order
    }
  }

  // Check if this feature depends on any features after newIndex
  for (int i = newIndex + 1; i < static_cast<int>(m_features.size()); ++i) {
    if (feature->dependsOn(m_features[i].get())) {
      return false; // Would create invalid dependency order
    }
  }

  return true;
}

QVariantMap FeatureTree::serialize() const {
  QVariantMap data;
  QVariantList featuresList;

  for (const auto &feature : m_features) {
    QVariantMap featureData = feature->serialize();

    // Add parent reference
    if (feature->parent()) {
      featureData["parentName"] = feature->parent()->name();
    }

    // Add dependency references
    QStringList depNames;
    for (const auto *dep : feature->dependencies()) {
      depNames.append(dep->name());
    }
    if (!depNames.isEmpty()) {
      featureData["dependencies"] = depNames;
    }

    featuresList.append(featureData);
  }

  data["features"] = featuresList;
  return data;
}

bool FeatureTree::deserialize(const QVariantMap &data) {
  if (!data.contains("features")) {
    return false;
  }

  clear();

  // Note: This is a simplified deserialization
  // In a real implementation, you would need to:
  // 1. Create feature instances based on type
  // 2. Restore parent-child relationships
  // 3. Restore dependencies
  // 4. Rebuild the tree structure

  // For now, we just return true as a placeholder
  // Actual feature creation will be handled by Document class
  return true;
}

void FeatureTree::buildDependencyOrder() {
  m_dependencyOrder.clear();

  // Topological sort using Kahn's algorithm
  std::set<Feature *> visited;
  std::vector<Feature *> result;

  // Helper function for DFS
  std::function<void(Feature *)> visit = [&](Feature *feature) {
    if (!feature || visited.count(feature)) {
      return;
    }

    visited.insert(feature);

    // Visit dependencies first
    for (auto *dep : feature->dependencies()) {
      visit(dep);
    }

    result.push_back(feature);
  };

  // Visit all features
  for (const auto &f : m_features) {
    visit(f.get());
  }

  m_dependencyOrder = result;
}

std::vector<Feature *>
FeatureTree::getRegenerationOrder(Feature *fromFeature) const {
  if (!fromFeature) {
    // Return all features in dependency order
    return m_dependencyOrder;
  }

  // Return features from fromFeature onwards (including dependents)
  std::vector<Feature *> result;
  bool found = false;

  for (auto *feature : m_dependencyOrder) {
    if (feature == fromFeature) {
      found = true;
    }
    if (found) {
      result.push_back(feature);
    }
  }

  // Also include all features that depend on fromFeature
  auto dependents = getDependents(fromFeature);
  for (auto *dep : dependents) {
    if (std::find(result.begin(), result.end(), dep) == result.end()) {
      result.push_back(dep);
    }
  }

  return result;
}

bool FeatureTree::hasCircularDependency(const Feature *feature,
                                        const Feature *potentialDep) const {
  if (!feature || !potentialDep) {
    return false;
  }

  // Check if potentialDep already depends on feature
  return potentialDep->dependsOn(feature);
}

} // namespace core
} // namespace opencad
