/**
 * @file Document.cpp
 * @brief Central document management class - Implementation
 */

#include "Document.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QUuid>
#include <algorithm>
#include <vector>

// Define AssemblySnapshot locally for now
namespace opencad {
namespace core {

struct ComponentData {
  QUuid id;
  gp_Trsf placement;
  std::string name;
  std::shared_ptr<core::Shape> shape;
};

struct ConstraintData {
  assembly::ConstraintType type;
  double value;
  QUuid c1_id;
  QUuid c2_id;
};

struct AssemblySnapshot : public Snapshot {
  std::vector<ComponentData> components;
  std::vector<ConstraintData> constraints;
};

Document::Document(QObject *parent)
    : QObject(parent), m_name("Untitled"),
      m_creationDate(QDateTime::currentDateTime()),
      m_lastModifiedDate(QDateTime::currentDateTime()),
      m_featureTree(std::make_unique<FeatureTree>(this)),
      m_parameterManager(std::make_unique<ParameterManager>(this)),
      m_undoRedoManager(std::make_unique<UndoRedoManager>()),
      m_assembly(std::make_shared<assembly::Assembly>()) {
  connectSignals();
}

Document::~Document() = default;

void Document::connectSignals() {
  // Connect feature tree signals
  connect(m_featureTree.get(), &FeatureTree::featureAdded, this,
          &Document::featureAdded);
  connect(m_featureTree.get(), &FeatureTree::featureRemoved, this,
          &Document::featureRemoved);
  connect(m_featureTree.get(), &FeatureTree::treeStructureChanged, this,
          &Document::onFeatureTreeChanged);
  connect(m_featureTree.get(), &FeatureTree::featureModified, this,
          &Document::onFeatureTreeChanged);
  connect(m_featureTree.get(), &FeatureTree::regenerationCompleted, this,
          &Document::regenerationCompleted);

  // Connect parameter manager signals
  connect(m_parameterManager.get(), &ParameterManager::parameterChanged, this,
          &Document::onParameterChanged);
}

void Document::setFilePath(const QString &path) {
  if (m_filePath != path) {
    m_filePath = path;
    // Extract name from file path
    if (!path.isEmpty()) {
      QFileInfo fileInfo(path);
      m_name = fileInfo.baseName();
    }
  }
}

void Document::setName(const QString &name) {
  if (m_name != name) {
    m_name = name;
    setModified(true);
  }
}

void Document::setModified(bool modified) {
  if (m_modified != modified) {
    m_modified = modified;
    if (modified) {
      updateModificationDate();
    }
    emit modifiedChanged(modified);
  }
}

bool Document::addFeature(std::shared_ptr<Feature> feature, Feature *parent) {
  if (!feature) {
    return false;
  }

  bool success = m_featureTree->addFeature(feature, parent);
  if (success) {
    setModified(true);
  }
  return success;
}

bool Document::removeFeature(Feature *feature) {
  if (!feature) {
    return false;
  }

  bool success = m_featureTree->removeFeature(feature);
  if (success) {
    setModified(true);
  }
  return success;
}

std::vector<TopoDS_Shape> Document::getAllShapes() const {
  std::vector<TopoDS_Shape> shapes;

  // Add shapes from features
  for (auto *feature : m_featureTree->allFeatures()) {
    if (feature->isVisible() && !feature->isSuppressed() &&
        feature->hasValidResult()) {
      shapes.push_back(feature->resultShape());
    }
  }

  // Add temporary shapes (backward compatibility)
  for (const auto &shape : m_temporaryShapes) {
    if (!shape.IsNull()) {
      shapes.push_back(shape);
    }
  }

  return shapes;
}

void Document::addTemporaryShape(const TopoDS_Shape &shape) {
  if (!shape.IsNull()) {
    m_temporaryShapes.push_back(shape);
    setModified(true);
  }
}

void Document::clearTemporaryShapes() {
  m_temporaryShapes.clear();
  setModified(true);
}

void Document::checkpoint(const QString &description, const QStringList &featureList) {
  // Determine if we are in Part or Assembly mode
  // For now, simpler heuristic: if assembly has components, snap assembly.
  // Ideally checking active document type.

  // TODO: Better mode check.
  bool isAssembly = !m_assembly->getComponents().empty() ||
                    !m_assembly->getConstraints().empty();

  std::vector<std::string> featureListStd;
  for (const QString& str : featureList) {
    featureListStd.push_back(str.toStdString());
  }

  if (isAssembly) {
    auto snapshot = std::make_shared<AssemblySnapshot>();
    snapshot->description = description.toStdString();
    snapshot->featureListItems = featureListStd;

    // Snapshot Components
    for (const auto &comp : m_assembly->getComponents()) {
      ComponentData func;
      func.id = comp->id();
      func.placement = comp->getPlacement();
      func.name = comp->getName();
      func.shape = comp->getShape();
      snapshot->components.push_back(func);
    }

    // Snapshot Constraints
    for (const auto &constr : m_assembly->getConstraints()) {
      ConstraintData cdata;
      cdata.type = constr->getType();
      cdata.value = constr->getValue();
      cdata.c1_id = constr->getComponent1()->id();
      cdata.c2_id = constr->getComponent2()->id();
      snapshot->constraints.push_back(cdata);
    }

    m_undoRedoManager->checkpoint(snapshot);

  } else {
    // Snapshot Shapes (Part Mode)
    auto shapes = getAllShapes();
    auto snapshot = std::make_shared<ShapeSnapshot>();
    snapshot->description = description.toStdString();
    snapshot->featureListItems = featureListStd;
    snapshot->shapes = shapes;

    m_undoRedoManager->checkpoint(snapshot);
  }
}

bool Document::undo() {
  auto snapshot = m_undoRedoManager->undo();
  if (!snapshot)
    return false;

  if (auto partSnap = std::dynamic_pointer_cast<ShapeSnapshot>(snapshot)) {
    // Restore Part
    m_temporaryShapes = partSnap->shapes; // Simplified restoration
    
    QStringList featureList;
    for (const auto& str : partSnap->featureListItems) {
      featureList.append(QString::fromStdString(str));
    }
    emit featureListRestored(featureList);

    setModified(true);
    return true;
  } else if (auto asmSnap =
                 std::dynamic_pointer_cast<AssemblySnapshot>(snapshot)) {
    // Restore Assembly
    m_assembly->clear(); // Clear current assembly

    // Recreate Components
    for (const auto &cdata : asmSnap->components) {
      auto comp = std::make_shared<assembly::Component>(cdata.shape);
      comp->setPlacement(cdata.placement);
      comp->setName(cdata.name);
      // FORCE ID restoration (hacky but needed for constraints)
      // We need to add setID to Component or use friend/const_cast if needed.
      // For now, let's assume we can map old IDs to new Components via a map
      // locally logic. Actually, we can't easily "restore" the UUID unless we
      // added setId. But wait, constraints need to point to the NEW component
      // instances. So we map OLD ID (from snapshot) to NEW Component Instance.
      m_assembly->addComponent(comp);
    }

    // Re-link constraints
    // This requires finding components by the IDs stored in snapshot.
    // But we just created NEW components with NEW IDs (unless we force set ID).
    // We should match by index if order is preserved? Order IS preserved in
    // vector. Better: Map <SnapshotID, NewComponentPtr>

    std::map<QUuid, std::shared_ptr<assembly::Component>> idMap;
    auto currentComponents =
        m_assembly->getComponents(); // These are the newly added ones

    if (currentComponents.size() == asmSnap->components.size()) {
      for (size_t i = 0; i < currentComponents.size(); ++i) {
        idMap[asmSnap->components[i].id] = currentComponents[i];
        // We should ideally set the ID of the new component to match the old
        // one so subsequent Undos work consistently. But Component::m_id is
        // private and const? It was initialized in constructor. We need a way
        // to set it, or construct with it.
      }
    }

    for (const auto &cdata : asmSnap->constraints) {
      auto c1 = idMap[cdata.c1_id];
      auto c2 = idMap[cdata.c2_id];
      if (c1 && c2) {
        auto constraint =
            std::make_shared<assembly::AssemblyConstraint>(cdata.type, c1, c2);
        constraint->setValue(cdata.value);
        m_assembly->addConstraint(constraint);
      }
    }

    QStringList featureList;
    for (const auto& str : asmSnap->featureListItems) {
      featureList.append(QString::fromStdString(str));
    }
    emit featureListRestored(featureList);

    setModified(true);
    return true;
  }

  return false;
}

bool Document::redo() {
  auto snapshot = m_undoRedoManager->redo();
  if (!snapshot)
    return false;

  if (auto partSnap = std::dynamic_pointer_cast<ShapeSnapshot>(snapshot)) {
    m_temporaryShapes = partSnap->shapes;
    
    QStringList featureList;
    for (const auto& str : partSnap->featureListItems) {
      featureList.append(QString::fromStdString(str));
    }
    emit featureListRestored(featureList);

    setModified(true);
    return true;
  } else if (auto asmSnap =
                 std::dynamic_pointer_cast<AssemblySnapshot>(snapshot)) {
    // Same logic as undo
    m_assembly->clear();

    for (const auto &cdata : asmSnap->components) {
      auto comp = std::make_shared<assembly::Component>(cdata.shape);
      comp->setPlacement(cdata.placement);
      comp->setName(cdata.name);
      m_assembly->addComponent(comp);
    }

    std::map<QUuid, std::shared_ptr<assembly::Component>> idMap;
    auto currentComponents = m_assembly->getComponents();

    if (currentComponents.size() == asmSnap->components.size()) {
      for (size_t i = 0; i < currentComponents.size(); ++i) {
        idMap[asmSnap->components[i].id] = currentComponents[i];
      }
    }

    for (const auto &cdata : asmSnap->constraints) {
      auto c1 = idMap[cdata.c1_id];
      auto c2 = idMap[cdata.c2_id];
      if (c1 && c2) {
        auto constraint =
            std::make_shared<assembly::AssemblyConstraint>(cdata.type, c1, c2);
        constraint->setValue(cdata.value);
        m_assembly->addConstraint(constraint);
      }
    }

    QStringList featureList;
    for (const auto& str : asmSnap->featureListItems) {
      featureList.append(QString::fromStdString(str));
    }
    emit featureListRestored(featureList);

    setModified(true);
    return true;
  }
  return false;
}

bool Document::canUndo() const { return m_undoRedoManager->canUndo(); }

bool Document::canRedo() const { return m_undoRedoManager->canRedo(); }

QString Document::undoDescription() const {
  return QString::fromStdString(m_undoRedoManager->undoDescription());
}

QString Document::redoDescription() const {
  return QString::fromStdString(m_undoRedoManager->redoDescription());
}

void Document::addSketch(std::shared_ptr<sketch::Sketch> sketch) {
  if (sketch) {
    m_sketches.push_back(sketch);
    setModified(true);
  }
}

void Document::removeSketch(sketch::Sketch *sketch) {
  auto it = std::find_if(m_sketches.begin(), m_sketches.end(),
                         [sketch](const std::shared_ptr<sketch::Sketch> &s) {
                           return s.get() == sketch;
                         });

  if (it != m_sketches.end()) {
    m_sketches.erase(it);
    setModified(true);
  }
}

bool Document::regenerate() { return m_featureTree->regenerate(); }

bool Document::regenerateFrom(Feature *fromFeature) {
  return m_featureTree->regenerate(fromFeature);
}

bool Document::save(const QString &filePath) {
  QString savePath = filePath.isEmpty() ? m_filePath : filePath;

  if (savePath.isEmpty()) {
    return false; // No path specified
  }

  // Serialize document to JSON
  QVariantMap data = serialize();
  QJsonDocument jsonDoc = QJsonDocument::fromVariant(data);

  // Write to file
  QFile file(savePath);
  if (!file.open(QIODevice::WriteOnly)) {
    return false;
  }

  file.write(jsonDoc.toJson(QJsonDocument::Indented));
  file.close();

  // Update document state
  setFilePath(savePath);
  setModified(false);

  emit documentSaved(savePath);
  return true;
}

bool Document::load(const QString &filePath) {
  if (filePath.isEmpty()) {
    return false;
  }

  // Read file
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    return false;
  }

  QByteArray data = file.readAll();
  file.close();

  // Parse JSON
  QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
  if (jsonDoc.isNull()) {
    return false;
  }

  // Deserialize
  QVariantMap docData = jsonDoc.toVariant().toMap();
  if (!deserialize(docData)) {
    return false;
  }

  // Update document state
  setFilePath(filePath);
  setModified(false);

  emit documentLoaded(filePath);
  return true;
}

void Document::newDocument() {
  // Clear all data
  m_featureTree->clear();
  m_sketches.clear();
  m_undoRedoManager->clear();

  // Reset metadata
  m_filePath.clear();
  m_name = "Untitled";
  m_creationDate = QDateTime::currentDateTime();
  m_lastModifiedDate = m_creationDate;
  setModified(false);
}

bool Document::close() {
  // In a full implementation, this would prompt to save if modified
  // For now, just clear the document
  newDocument();
  return true;
}

QVariantMap Document::serialize() const {
  QVariantMap data;

  // Document metadata
  data["name"] = m_name;
  data["creationDate"] = m_creationDate.toString(Qt::ISODate);
  data["lastModifiedDate"] = m_lastModifiedDate.toString(Qt::ISODate);

  // Feature tree
  data["featureTree"] = m_featureTree->serialize();

  // Parameters (if needed)
  // data["parameters"] = ...;

  // Sketches (simplified - would need proper serialization)
  data["sketchCount"] = static_cast<int>(m_sketches.size());

  return data;
}

bool Document::deserialize(const QVariantMap &data) {
  if (!data.contains("name")) {
    return false;
  }

  // Clear current state
  newDocument();

  // Restore metadata
  m_name = data["name"].toString();
  if (data.contains("creationDate")) {
    m_creationDate =
        QDateTime::fromString(data["creationDate"].toString(), Qt::ISODate);
  }
  if (data.contains("lastModifiedDate")) {
    m_lastModifiedDate =
        QDateTime::fromString(data["lastModifiedDate"].toString(), Qt::ISODate);
  }

  // Restore feature tree
  if (data.contains("featureTree")) {
    m_featureTree->deserialize(data["featureTree"].toMap());
  }

  return true;
}

void Document::onFeatureTreeChanged() { setModified(true); }

void Document::onParameterChanged(const QString &name, double value) {
  Q_UNUSED(name);
  Q_UNUSED(value);

  // When a parameter changes, mark features as needing regeneration
  // In a full implementation, we would track which features use which
  // parameters
  setModified(true);
}

void Document::updateModificationDate() {
  m_lastModifiedDate = QDateTime::currentDateTime();
}

} // namespace core
} // namespace opencad
