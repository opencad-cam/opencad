/**
 * @file Document.cpp
 * @brief Central document management class - Implementation
 */

#include "Document.h"
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>

namespace opencad {
namespace core {

Document::Document(QObject *parent)
    : QObject(parent), m_name("Untitled"),
      m_creationDate(QDateTime::currentDateTime()),
      m_lastModifiedDate(QDateTime::currentDateTime()),
      m_featureTree(std::make_unique<FeatureTree>(this)),
      m_parameterManager(std::make_unique<ParameterManager>(this)),
      m_undoRedoManager(std::make_unique<UndoRedoManager>()) {
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

void Document::checkpoint(const QString &description) {
  // Get all current shapes
  auto shapes = getAllShapes();

  // Create checkpoint in undo/redo manager
  m_undoRedoManager->checkpoint(shapes, description.toStdString());
}

bool Document::undo() {
  std::vector<TopoDS_Shape> shapes;
  std::string description;

  if (m_undoRedoManager->undo(shapes, description)) {
    // Note: In a full implementation, we would restore feature states
    // For now, this is a simplified version
    setModified(true);
    return true;
  }

  return false;
}

bool Document::redo() {
  std::vector<TopoDS_Shape> shapes;
  std::string description;

  if (m_undoRedoManager->redo(shapes, description)) {
    // Note: In a full implementation, we would restore feature states
    // For now, this is a simplified version
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
    emit sketchAdded(sketch.get());
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
    emit sketchRemoved(sketch);
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
  emit sketchesCleared();

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
