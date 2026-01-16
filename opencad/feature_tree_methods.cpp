// ==================== FEATURE TREE UI INTERACTION ====================

void MainWindow::updateFeatureList() {
  if (!m_featureList)
    return;

  m_featureList->clear();

  // Add default origin items
  m_featureList->addItem("🌐 Origin");
  m_featureList->addItem("  ⬜ XY Plane");
  m_featureList->addItem("  ⬜ XZ Plane");
  m_featureList->addItem("  ⬜ YZ Plane");

  // Add features from document
  for (auto *feature : m_document->featureTree()->allFeatures()) {
    QString icon = feature->isSuppressed() ? "⏸️" : "✅";
    QString name = QString("%1 %2").arg(icon).arg(feature->name());

    auto *item = new QListWidgetItem(name);
    item->setData(Qt::UserRole, QVariant::fromValue((void *)feature));

    // Visual feedback for suppressed features
    if (feature->isSuppressed()) {
      item->setForeground(Qt::gray);
      QFont font = item->font();
      font.setItalic(true);
      item->setFont(font);
    }

    m_featureList->addItem(item);
  }
}

void MainWindow::onFeatureSelected(QListWidgetItem *item) {
  if (!item)
    return;

  // Skip origin items
  if (item->text().contains("Origin") || item->text().contains("Plane")) {
    return;
  }

  // Get feature from item data
  auto *feature =
      static_cast<core::Feature *>(item->data(Qt::UserRole).value<void *>());
  if (!feature)
    return;

  // Highlight feature shape in viewport
  if (feature->hasValidResult()) {
    // TODO: Add highlight method to viewport
    // m_viewport->highlightShape(feature->resultShape());
  }

  statusBar()->showMessage(
      QString("Selected: %1 (%2)")
          .arg(feature->name())
          .arg(core::featureTypeToString(feature->type())));
}

void MainWindow::onFeatureContextMenu(const QPoint &pos) {
  auto *item = m_featureList->itemAt(pos);
  if (!item)
    return;

  // Skip origin items
  if (item->text().contains("Origin") || item->text().contains("Plane")) {
    return;
  }

  auto *feature =
      static_cast<core::Feature *>(item->data(Qt::UserRole).value<void *>());
  if (!feature)
    return;

  QMenu menu(this);

  // Suppress/Unsuppress action
  QString suppressText = feature->isSuppressed() ? "Unsuppress" : "Suppress";
  auto *suppressAction = menu.addAction(suppressText);
  connect(suppressAction, &QAction::triggered, this,
          &MainWindow::onToggleFeatureSuppression);

  menu.addSeparator();

  // Edit action (disabled for now)
  auto *editAction = menu.addAction("Edit Feature...");
  editAction->setEnabled(false);

  // Delete action
  auto *deleteAction = menu.addAction("Delete");
  connect(deleteAction, &QAction::triggered, [this, feature]() {
    if (m_document->removeFeature(feature)) {
      updateFeatureList();
      displayAllShapes();
      m_document->checkpoint("Delete Feature");
      statusBar()->showMessage(QString("Deleted: %1").arg(feature->name()));
    }
  });

  menu.exec(m_featureList->mapToGlobal(pos));
}

void MainWindow::onToggleFeatureSuppression() {
  auto *item = m_featureList->currentItem();
  if (!item)
    return;

  auto *feature =
      static_cast<core::Feature *>(item->data(Qt::UserRole).value<void *>());
  if (!feature)
    return;

  // Toggle suppression
  bool newState = !feature->isSuppressed();
  m_document->featureTree()->suppressFeature(feature, newState);

  // Regenerate from this feature
  m_document->regenerateFrom(feature);

  // Update UI
  updateFeatureList();
  displayAllShapes();

  QString action = newState ? "Suppressed" : "Unsuppressed";
  m_document->checkpoint(QString("%1 %2").arg(action).arg(feature->name()));

  statusBar()->showMessage(QString("%1: %2").arg(action).arg(feature->name()));
}

void MainWindow::onFeatureReordered() {
  // Get new order from list widget
  QList<core::Feature *> newOrder;

  for (int i = 4; i < m_featureList->count(); ++i) { // Skip origin items
    auto *item = m_featureList->item(i);
    auto *feature =
        static_cast<core::Feature *>(item->data(Qt::UserRole).value<void *>());
    if (feature) {
      newOrder.append(feature);
    }
  }

  // TODO: Implement FeatureTree::reorderFeatures with dependency validation
  // For now, just regenerate
  m_document->regenerate();
  displayAllShapes();

  m_document->checkpoint("Reorder Features");
  statusBar()->showMessage("Features reordered");
}

} // namespace ui
} // namespace opencad
