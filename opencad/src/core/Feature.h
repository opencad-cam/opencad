/**
 * @file Feature.h
 * @brief Base class for all CAD features
 *
 * OpenCAD - Modular CAD/CAE Platform
 * Core Module
 */

#pragma once

#include <QDateTime>
#include <QMap>
#include <QString>
#include <QVariant>
#include <TopoDS_Shape.hxx>
#include <memory>
#include <vector>

namespace opencad {
namespace core {

class ParameterManager;

/**
 * @enum FeatureType
 * @brief Types of CAD features
 */
enum class FeatureType {
  Unknown,
  // Sketch-based features
  Extrude,
  Cut,
  Revolve,
  Sweep,
  Loft,
  // Dress-up features
  Fillet,
  Chamfer,
  Shell,
  Draft,
  // Pattern features
  LinearPattern,
  CircularPattern,
  Mirror,
  // Reference features
  Plane,
  Axis,
  Point,
  // Advanced features
  Hole,
  Rib,
  Dome,
  Thicken,
  OffsetSurface,
  Split,
  Scale,
  // Boolean operations
  BooleanFuse,
  BooleanCut,
  BooleanCommon,
  // Sketch
  Sketch
};

/**
 * @class Feature
 * @brief Abstract base class for all CAD features
 *
 * Features are parametric operations that create or modify geometry.
 * They form a tree structure representing the construction history.
 */
class Feature {
public:
  /**
   * @brief Constructor
   * @param name Feature name
   * @param type Feature type
   */
  explicit Feature(const QString &name, FeatureType type);
  virtual ~Feature() = default;

  // Core execution
  /**
   * @brief Execute the feature to generate geometry
   * @return Resulting shape, or null shape on error
   */
  virtual TopoDS_Shape execute() = 0;

  /**
   * @brief Check if feature can execute
   * @return True if all dependencies and parameters are valid
   */
  virtual bool canExecute() const;

  /**
   * @brief Get last error message
   */
  QString errorMessage() const { return m_errorMessage; }

  // Properties
  QString name() const { return m_name; }
  void setName(const QString &name) { m_name = name; }

  FeatureType type() const { return m_type; }
  QString typeString() const;

  bool isVisible() const { return m_visible; }
  void setVisible(bool visible);

  bool isSuppressed() const { return m_suppressed; }
  void setSuppressed(bool suppressed);

  QDateTime creationTime() const { return m_creationTime; }
  QDateTime modificationTime() const { return m_modificationTime; }

  // Result shape
  TopoDS_Shape resultShape() const { return m_resultShape; }
  bool hasValidResult() const { return !m_resultShape.IsNull(); }

  // Parent-child relationships
  Feature *parent() const { return m_parent; }
  void setParent(Feature *parent) { m_parent = parent; }

  const std::vector<Feature *> &children() const { return m_children; }
  void addChild(Feature *child);
  void removeChild(Feature *child);

  // Dependencies
  const std::vector<Feature *> &dependencies() const { return m_dependencies; }
  void addDependency(Feature *feature);
  void removeDependency(Feature *feature);
  bool dependsOn(const Feature *feature) const;

  // Parameters
  void setParameter(const QString &name, const QVariant &value);
  QVariant parameter(const QString &name) const;
  bool hasParameter(const QString &name) const;
  QMap<QString, QVariant> allParameters() const { return m_parameters; }

  // Serialization
  /**
   * @brief Serialize feature to JSON-compatible map
   */
  virtual QVariantMap serialize() const;

  /**
   * @brief Deserialize feature from JSON-compatible map
   */
  virtual bool deserialize(const QVariantMap &data);

  // Regeneration
  /**
   * @brief Mark feature as needing regeneration
   */
  void setNeedsRegeneration(bool needs = true) { m_needsRegeneration = needs; }
  bool needsRegeneration() const { return m_needsRegeneration; }

  /**
   * @brief Regenerate this feature and update result
   */
  bool regenerate();

protected:
  // Helper for derived classes to set error messages
  void setError(const QString &message) { m_errorMessage = message; }

  // Helper to update modification time
  void touch() { m_modificationTime = QDateTime::currentDateTime(); }

  // Result storage
  void setResultShape(const TopoDS_Shape &shape) { m_resultShape = shape; }

private:
  // Basic properties
  QString m_name;
  FeatureType m_type;
  bool m_visible = true;
  bool m_suppressed = false;
  QDateTime m_creationTime;
  QDateTime m_modificationTime;

  // Execution state
  TopoDS_Shape m_resultShape;
  QString m_errorMessage;
  bool m_needsRegeneration = true;

  // Relationships
  Feature *m_parent = nullptr;
  std::vector<Feature *> m_children;
  std::vector<Feature *> m_dependencies;

  // Parameters (feature-specific data)
  QMap<QString, QVariant> m_parameters;
};

/**
 * @brief Convert FeatureType to string
 */
QString featureTypeToString(FeatureType type);

/**
 * @brief Convert string to FeatureType
 */
FeatureType stringToFeatureType(const QString &str);

} // namespace core
} // namespace opencad
