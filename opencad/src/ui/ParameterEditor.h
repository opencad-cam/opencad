/**
 * @file ParameterEditor.h
 * @brief Parameter editor panel for associative design
 */

#pragma once

#include <QDialog>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <core/Parameter.h>

namespace opencad {
namespace ui {

/**
 * @brief Dialog for editing expressions
 */
class ExpressionDialog : public QDialog {
  Q_OBJECT
public:
  explicit ExpressionDialog(const QString &paramName,
                            const QString &currentExpr,
                            const QStringList &availableParams,
                            QWidget *parent = nullptr);

  QString expression() const;

private:
  void setupUI();
  void insertParameter(const QString &param);

  QString m_paramName;
  QLineEdit *m_exprEdit;
  QListWidget *m_paramList;
};

/**
 * @brief Panel for editing and managing parameters
 */
class ParameterEditor : public QWidget {
  Q_OBJECT

public:
  explicit ParameterEditor(QWidget *parent = nullptr);
  ~ParameterEditor() override = default;

  // Set the parameter manager to edit
  void setParameterManager(core::ParameterManager *manager);

  // Refresh the display
  void refresh();

signals:
  void parameterChanged(const QString &name, double value);
  void expressionChanged(const QString &name, const QString &expr);

private slots:
  void onItemDoubleClicked(QTreeWidgetItem *item, int column);
  void onAddParameter();
  void onRemoveParameter();
  void onMakeAssociative();
  void onClearExpression();
  void onContextMenu(const QPoint &pos);

private:
  void setupUI();
  void updateTree();
  QTreeWidgetItem *createParameterItem(core::Parameter *param);

  core::ParameterManager *m_manager = nullptr;
  QTreeWidget *m_tree;
  QPushButton *m_addBtn;
  QPushButton *m_removeBtn;
  QPushButton *m_associativeBtn;
};

} // namespace ui
} // namespace opencad
