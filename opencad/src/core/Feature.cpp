/**
 * @file Feature.cpp
 * @brief Base class for all CAD features - Implementation
 */

#include "Feature.h"
#include <algorithm>

namespace opencad {
namespace core {

Feature::Feature(const QString &name, FeatureType type)
    : m_name(name), m_type(type), m_creationTime(QDateTime::currentDateTime()),
      m_modificationTime(QDateTime::currentDateTime()) {}

bool Feature::canExecute() const {
  // Cannot execute if suppressed
  if (m_suppressed) {
    return false;
  }

  // Check if all dependencies have valid results
  for (const auto *dep : m_dependencies) {
    if (!dep || !dep->hasValidResult()) {
      return false;
    }
  }

  return true;
}

void Feature::setVisible(bool visible) {
  if (m_visible != visible) {
    m_visible = visible;
    touch();
  }
}

void Feature::setSuppressed(bool suppressed) {
  if (m_suppressed != suppressed) {
    m_suppressed = suppressed;
    m_needsRegeneration = true;
    touch();

    // Mark all children as needing regeneration
    for (auto *child : m_children) {
      child->setNeedsRegeneration(true);
    }
  }
}

void Feature::addChild(Feature *child) {
  if (child && std::find(m_children.begin(), m_children.end(), child) ==
                   m_children.end()) {
    m_children.push_back(child);
    child->setParent(this);
  }
}

void Feature::removeChild(Feature *child) {
  auto it = std::find(m_children.begin(), m_children.end(), child);
  if (it != m_children.end()) {
    m_children.erase(it);
    if (child) {
      child->setParent(nullptr);
    }
  }
}

void Feature::addDependency(Feature *feature) {
  if (feature && std::find(m_dependencies.begin(), m_dependencies.end(),
                           feature) == m_dependencies.end()) {
    m_dependencies.push_back(feature);
  }
}

void Feature::removeDependency(Feature *feature) {
  auto it = std::find(m_dependencies.begin(), m_dependencies.end(), feature);
  if (it != m_dependencies.end()) {
    m_dependencies.erase(it);
  }
}

bool Feature::dependsOn(const Feature *feature) const {
  if (!feature) {
    return false;
  }

  // Direct dependency
  if (std::find(m_dependencies.begin(), m_dependencies.end(), feature) !=
      m_dependencies.end()) {
    return true;
  }

  // Recursive dependency check
  for (const auto *dep : m_dependencies) {
    if (dep->dependsOn(feature)) {
      return true;
    }
  }

  return false;
}

void Feature::setParameter(const QString &name, const QVariant &value) {
  if (m_parameters.value(name) != value) {
    m_parameters[name] = value;
    m_needsRegeneration = true;
    touch();
  }
}

QVariant Feature::parameter(const QString &name) const {
  return m_parameters.value(name);
}

bool Feature::hasParameter(const QString &name) const {
  return m_parameters.contains(name);
}

bool Feature::regenerate() {
  if (!canExecute()) {
    m_errorMessage =
        "Feature cannot execute (suppressed or invalid dependencies)";
    m_resultShape = TopoDS_Shape();
    return false;
  }

  m_errorMessage.clear();
  m_resultShape = execute();
  m_needsRegeneration = false;
  touch();

  return !m_resultShape.IsNull();
}

QVariantMap Feature::serialize() const {
  QVariantMap data;
  data["name"] = m_name;
  data["type"] = featureTypeToString(m_type);
  data["visible"] = m_visible;
  data["suppressed"] = m_suppressed;
  data["creationTime"] = m_creationTime.toString(Qt::ISODate);
  data["modificationTime"] = m_modificationTime.toString(Qt::ISODate);
  data["parameters"] = m_parameters;

  return data;
}

bool Feature::deserialize(const QVariantMap &data) {
  if (!data.contains("name") || !data.contains("type")) {
    return false;
  }

  m_name = data["name"].toString();
  m_type = stringToFeatureType(data["type"].toString());
  m_visible = data.value("visible", true).toBool();
  m_suppressed = data.value("suppressed", false).toBool();

  if (data.contains("creationTime")) {
    m_creationTime =
        QDateTime::fromString(data["creationTime"].toString(), Qt::ISODate);
  }
  if (data.contains("modificationTime")) {
    m_modificationTime =
        QDateTime::fromString(data["modificationTime"].toString(), Qt::ISODate);
  }

  if (data.contains("parameters")) {
    m_parameters = data["parameters"].toMap();
  }

  m_needsRegeneration = true;
  return true;
}

QString Feature::typeString() const { return featureTypeToString(m_type); }

QString featureTypeToString(FeatureType type) {
  switch (type) {
  case FeatureType::Extrude:
    return "Extrude";
  case FeatureType::Cut:
    return "Cut";
  case FeatureType::Revolve:
    return "Revolve";
  case FeatureType::Sweep:
    return "Sweep";
  case FeatureType::Loft:
    return "Loft";
  case FeatureType::Fillet:
    return "Fillet";
  case FeatureType::Chamfer:
    return "Chamfer";
  case FeatureType::Shell:
    return "Shell";
  case FeatureType::Draft:
    return "Draft";
  case FeatureType::LinearPattern:
    return "LinearPattern";
  case FeatureType::CircularPattern:
    return "CircularPattern";
  case FeatureType::Mirror:
    return "Mirror";
  case FeatureType::Plane:
    return "Plane";
  case FeatureType::Axis:
    return "Axis";
  case FeatureType::Point:
    return "Point";
  case FeatureType::Hole:
    return "Hole";
  case FeatureType::Rib:
    return "Rib";
  case FeatureType::Dome:
    return "Dome";
  case FeatureType::Thicken:
    return "Thicken";
  case FeatureType::OffsetSurface:
    return "OffsetSurface";
  case FeatureType::Split:
    return "Split";
  case FeatureType::Scale:
    return "Scale";
  case FeatureType::BooleanFuse:
    return "BooleanFuse";
  case FeatureType::BooleanCut:
    return "BooleanCut";
  case FeatureType::BooleanCommon:
    return "BooleanCommon";
  case FeatureType::Sketch:
    return "Sketch";
  default:
    return "Unknown";
  }
}

FeatureType stringToFeatureType(const QString &str) {
  if (str == "Extrude")
    return FeatureType::Extrude;
  if (str == "Cut")
    return FeatureType::Cut;
  if (str == "Revolve")
    return FeatureType::Revolve;
  if (str == "Sweep")
    return FeatureType::Sweep;
  if (str == "Loft")
    return FeatureType::Loft;
  if (str == "Fillet")
    return FeatureType::Fillet;
  if (str == "Chamfer")
    return FeatureType::Chamfer;
  if (str == "Shell")
    return FeatureType::Shell;
  if (str == "Draft")
    return FeatureType::Draft;
  if (str == "LinearPattern")
    return FeatureType::LinearPattern;
  if (str == "CircularPattern")
    return FeatureType::CircularPattern;
  if (str == "Mirror")
    return FeatureType::Mirror;
  if (str == "Plane")
    return FeatureType::Plane;
  if (str == "Axis")
    return FeatureType::Axis;
  if (str == "Point")
    return FeatureType::Point;
  if (str == "Hole")
    return FeatureType::Hole;
  if (str == "Rib")
    return FeatureType::Rib;
  if (str == "Dome")
    return FeatureType::Dome;
  if (str == "Thicken")
    return FeatureType::Thicken;
  if (str == "OffsetSurface")
    return FeatureType::OffsetSurface;
  if (str == "Split")
    return FeatureType::Split;
  if (str == "Scale")
    return FeatureType::Scale;
  if (str == "BooleanFuse")
    return FeatureType::BooleanFuse;
  if (str == "BooleanCut")
    return FeatureType::BooleanCut;
  if (str == "BooleanCommon")
    return FeatureType::BooleanCommon;
  if (str == "Sketch")
    return FeatureType::Sketch;
  return FeatureType::Unknown;
}

} // namespace core
} // namespace opencad
