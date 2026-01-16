/**
 * @file ParameterEditor.cpp
 * @brief Implementation of parameter editor panel
 */

#include "ParameterEditor.h"
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>

namespace opencad {
namespace ui {

// ============== ExpressionDialog ==============

ExpressionDialog::ExpressionDialog(const QString &paramName,
                                   const QString &currentExpr,
                                   const QStringList &availableParams,
                                   QWidget *parent)
    : QDialog(parent), m_paramName(paramName) {
  setWindowTitle("Edit Expression - " + paramName);
  setMinimumSize(400, 300);
  setupUI();

  m_exprEdit->setText(currentExpr);
  m_paramList->addItems(availableParams);
}

void ExpressionDialog::setupUI() {
  auto *layout = new QVBoxLayout(this);

  // Info label
  auto *infoLabel =
      new QLabel("Enter an expression using parameters and math operators:\n"
                 "Examples: width * 2, length + 10, height / 2");
  infoLabel->setStyleSheet("color: #888;");
  layout->addWidget(infoLabel);

  // Expression input
  auto *exprGroup = new QGroupBox("Expression");
  auto *exprLayout = new QVBoxLayout(exprGroup);
  m_exprEdit = new QLineEdit();
  m_exprEdit->setPlaceholderText("e.g., width * height");
  m_exprEdit->setStyleSheet("font-family: monospace; font-size: 14px;");
  exprLayout->addWidget(m_exprEdit);
  layout->addWidget(exprGroup);

  // Available parameters
  auto *paramGroup =
      new QGroupBox("Available Parameters (double-click to insert)");
  auto *paramLayout = new QVBoxLayout(paramGroup);
  m_paramList = new QListWidget();
  m_paramList->setMaximumHeight(120);
  connect(m_paramList, &QListWidget::itemDoubleClicked,
          [this](QListWidgetItem *item) { insertParameter(item->text()); });
  paramLayout->addWidget(m_paramList);
  layout->addWidget(paramGroup);

  // Buttons
  auto *buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  layout->addWidget(buttonBox);
}

QString ExpressionDialog::expression() const {
  return m_exprEdit->text().trimmed();
}

void ExpressionDialog::insertParameter(const QString &param) {
  m_exprEdit->insert(param);
  m_exprEdit->setFocus();
}

// ============== ParameterEditor ==============

ParameterEditor::ParameterEditor(QWidget *parent) : QWidget(parent) {
  setupUI();
}

void ParameterEditor::setupUI() {
  auto *layout = new QVBoxLayout(this);
  layout->setSpacing(6);
  layout->setContentsMargins(8, 8, 8, 8);

  // Title
  auto *titleLabel = new QLabel("<b>📐 Parameters</b>");
  titleLabel->setStyleSheet("color: #4caf50; font-size: 14px;");
  layout->addWidget(titleLabel);

  // Tree widget
  m_tree = new QTreeWidget();
  m_tree->setHeaderLabels({"Name", "Value", "Expression"});
  m_tree->header()->setStretchLastSection(true);
  m_tree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
  m_tree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
  m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
  m_tree->setAlternatingRowColors(true);
  m_tree->setStyleSheet(R"(
        QTreeWidget {
            background-color: #2a2a2a;
            border: 1px solid #444;
            border-radius: 4px;
            color: #e0e0e0;
        }
        QTreeWidget::item {
            padding: 4px;
        }
        QTreeWidget::item:selected {
            background-color: #3d5afe;
        }
        QHeaderView::section {
            background-color: #333;
            color: #aaa;
            padding: 4px;
            border: none;
        }
    )");

  connect(m_tree, &QTreeWidget::itemDoubleClicked, this,
          &ParameterEditor::onItemDoubleClicked);
  connect(m_tree, &QTreeWidget::customContextMenuRequested, this,
          &ParameterEditor::onContextMenu);
  layout->addWidget(m_tree);

  // Buttons
  auto *btnLayout = new QHBoxLayout();

  m_addBtn = new QPushButton("+ Add");
  m_addBtn->setStyleSheet("background-color: #4caf50; color: white;");
  connect(m_addBtn, &QPushButton::clicked, this,
          &ParameterEditor::onAddParameter);
  btnLayout->addWidget(m_addBtn);

  m_removeBtn = new QPushButton("- Remove");
  m_removeBtn->setStyleSheet("background-color: #f44336; color: white;");
  connect(m_removeBtn, &QPushButton::clicked, this,
          &ParameterEditor::onRemoveParameter);
  btnLayout->addWidget(m_removeBtn);

  m_associativeBtn = new QPushButton("🔗 Make Associative");
  m_associativeBtn->setStyleSheet("background-color: #2196f3; color: white;");
  connect(m_associativeBtn, &QPushButton::clicked, this,
          &ParameterEditor::onMakeAssociative);
  btnLayout->addWidget(m_associativeBtn);

  layout->addLayout(btnLayout);
}

void ParameterEditor::setParameterManager(core::ParameterManager *manager) {
  m_manager = manager;

  if (m_manager) {
    connect(m_manager, &core::ParameterManager::parameterChanged,
            [this](const QString & /*name*/, double /*value*/) { refresh(); });
  }

  refresh();
}

void ParameterEditor::refresh() { updateTree(); }

void ParameterEditor::updateTree() {
  m_tree->clear();

  if (!m_manager)
    return;

  // Geometry parameters group
  auto *geoGroup = new QTreeWidgetItem(m_tree);
  geoGroup->setText(0, "📦 Geometry");
  geoGroup->setExpanded(true);

  // User parameters group
  auto *userGroup = new QTreeWidgetItem(m_tree);
  userGroup->setText(0, "📝 User Parameters");
  userGroup->setExpanded(true);

  for (auto *param : m_manager->allParameters()) {
    auto *item = createParameterItem(param);

    // Built-in geometry params (including Center of Mass)
    if (param->name() == "Volume" || param->name() == "Area" ||
        param->name() == "LengthX" || param->name() == "LengthY" ||
        param->name() == "LengthZ" || param->name() == "CoM_X" ||
        param->name() == "CoM_Y" || param->name() == "CoM_Z") {
      geoGroup->addChild(item);
    } else {
      userGroup->addChild(item);
    }
  }
}

QTreeWidgetItem *ParameterEditor::createParameterItem(core::Parameter *param) {
  auto *item = new QTreeWidgetItem();
  item->setText(0, param->name());
  item->setText(1, QString::number(param->value(), 'f', 3) + " " +
                       param->unitString());

  if (param->hasExpression()) {
    item->setText(2, "= " + param->expression());
    item->setForeground(2, QBrush(QColor("#03a9f4"))); // Blue for expressions
  } else {
    item->setText(2, "-");
    item->setForeground(2, QBrush(QColor("#666")));
  }

  item->setData(0, Qt::UserRole, param->name()); // Store name for lookup
  return item;
}

void ParameterEditor::onItemDoubleClicked(QTreeWidgetItem *item, int column) {
  if (!m_manager || !item->parent())
    return; // Ignore group items

  QString paramName = item->data(0, Qt::UserRole).toString();

  if (column == 1) {
    // Edit value
    bool ok;
    double current = m_manager->getValue(paramName);
    double newValue = QInputDialog::getDouble(
        this, "Edit Value", paramName + ":", current, -1e9, 1e9, 3, &ok);

    if (ok) {
      m_manager->setValue(paramName, newValue);
      emit parameterChanged(paramName, newValue);
    }
  } else if (column == 2) {
    // Edit expression
    onMakeAssociative();
  }
}

void ParameterEditor::onAddParameter() {
  if (!m_manager)
    return;

  bool ok;
  QString name = QInputDialog::getText(
      this, "Add Parameter", "Parameter name:", QLineEdit::Normal, "", &ok);
  if (!ok || name.isEmpty())
    return;

  if (m_manager->hasParameter(name)) {
    QMessageBox::warning(this, "Error",
                         "Parameter '" + name + "' already exists.");
    return;
  }

  double value = QInputDialog::getDouble(
      this, "Add Parameter", name + " value:", 0, -1e9, 1e9, 3, &ok);
  if (!ok)
    return;

  m_manager->addParameter(name, value, core::ParameterType::Scalar);
  refresh();
}

void ParameterEditor::onRemoveParameter() {
  if (!m_manager)
    return;

  auto *item = m_tree->currentItem();
  if (!item || !item->parent())
    return;

  QString paramName = item->data(0, Qt::UserRole).toString();

  // Don't allow removing built-in params (including CoM)
  if (paramName == "Volume" || paramName == "Area" || paramName == "LengthX" ||
      paramName == "LengthY" || paramName == "LengthZ" ||
      paramName == "CoM_X" || paramName == "CoM_Y" || paramName == "CoM_Z") {
    QMessageBox::warning(this, "Error",
                         "Cannot remove built-in geometry parameter.");
    return;
  }

  if (QMessageBox::question(this, "Remove Parameter",
                            "Remove parameter '" + paramName + "'?") ==
      QMessageBox::Yes) {
    m_manager->removeParameter(paramName);
    refresh();
  }
}

void ParameterEditor::onMakeAssociative() {
  if (!m_manager)
    return;

  auto *item = m_tree->currentItem();
  if (!item || !item->parent())
    return;

  QString paramName = item->data(0, Qt::UserRole).toString();

  // Get available parameters (excluding self)
  QStringList available;
  for (const QString &name : m_manager->parameterNames()) {
    if (name != paramName) {
      available.append(name);
    }
  }

  // Get current expression
  QString currentExpr;
  for (auto *param : m_manager->allParameters()) {
    if (param->name() == paramName) {
      currentExpr = param->expression();
      break;
    }
  }

  ExpressionDialog dlg(paramName, currentExpr, available, this);
  if (dlg.exec() == QDialog::Accepted) {
    QString expr = dlg.expression();
    if (expr.isEmpty()) {
      m_manager->clearExpression(paramName);
    } else {
      m_manager->setExpression(paramName, expr);
    }
    emit expressionChanged(paramName, expr);
    refresh();
  }
}

void ParameterEditor::onClearExpression() {
  if (!m_manager)
    return;

  auto *item = m_tree->currentItem();
  if (!item || !item->parent())
    return;

  QString paramName = item->data(0, Qt::UserRole).toString();
  m_manager->clearExpression(paramName);
  refresh();
}

void ParameterEditor::onContextMenu(const QPoint &pos) {
  auto *item = m_tree->itemAt(pos);
  if (!item || !item->parent())
    return;

  QString paramName = item->data(0, Qt::UserRole).toString();

  QMenu menu;
  menu.addAction("Edit Value", this, [this, paramName]() {
    bool ok;
    double current = m_manager->getValue(paramName);
    double newValue = QInputDialog::getDouble(
        this, "Edit Value", paramName + ":", current, -1e9, 1e9, 3, &ok);
    if (ok) {
      m_manager->setValue(paramName, newValue);
    }
  });

  menu.addAction("🔗 Make Associative", this,
                 &ParameterEditor::onMakeAssociative);
  menu.addAction("Clear Expression", this, &ParameterEditor::onClearExpression);
  menu.addSeparator();

  // Don't show remove for built-in params (including CoM)
  if (paramName != "Volume" && paramName != "Area" && paramName != "LengthX" &&
      paramName != "LengthY" && paramName != "LengthZ" &&
      paramName != "CoM_X" && paramName != "CoM_Y" && paramName != "CoM_Z") {
    menu.addAction("Remove", this, &ParameterEditor::onRemoveParameter);
  }

  menu.exec(m_tree->mapToGlobal(pos));
}

} // namespace ui
} // namespace opencad
