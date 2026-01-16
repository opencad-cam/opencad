/**
 * @file Parameter.cpp
 * @brief Implementation of parametric system
 */

#include "Parameter.h"
#include <QRegularExpression>
#include <cmath>

namespace opencad {
namespace core {

// ============ Parameter ============

Parameter::Parameter(const QString &name, double value, ParameterType type)
    : m_name(name), m_value(value), m_type(type) {}

void Parameter::setValue(double value) { m_value = value; }

void Parameter::setExpression(const QString &expr) { m_expression = expr; }

QString Parameter::unitString() const {
  switch (m_type) {
  case ParameterType::Length:
    return "mm";
  case ParameterType::Area:
    return "mm²";
  case ParameterType::Volume:
    return "mm³";
  case ParameterType::Angle:
    return "°";
  case ParameterType::Mass:
    return "kg";
  default:
    return "";
  }
}

QString Parameter::displayString() const {
  QString unit = unitString();
  if (unit.isEmpty()) {
    return QString("%1 = %2").arg(m_name).arg(m_value, 0, 'f', 3);
  }
  return QString("%1 = %2 %3").arg(m_name).arg(m_value, 0, 'f', 3).arg(unit);
}

// ============ ExpressionEvaluator ============

double ExpressionEvaluator::evaluate(const QString &expression,
                                     const QMap<QString, double> &variables) {
  QString expr = expression;

  // Replace variable names with values
  for (auto it = variables.begin(); it != variables.end(); ++it) {
    QRegularExpression re("\\b" + QRegularExpression::escape(it.key()) + "\\b");
    expr.replace(re, QString::number(it.value(), 'f', 10));
  }

  // Simple expression evaluator using basic operations
  // For production, use a proper math parser like muParser

  // Parse simple math: supports +, -, *, /, parentheses
  // This is a simplified version - a real implementation would use a proper
  // parser

  // Remove spaces
  expr = expr.simplified().replace(" ", "");

  // Try to evaluate using QJSEngine (Qt5+)
  // For now, use a simple approach for basic expressions

  // Handle simple cases
  bool ok;
  double result = expr.toDouble(&ok);
  if (ok)
    return result;

  // Handle basic operations (very simplified)
  // Real implementation should use proper expression parser
  if (expr.contains('*')) {
    QStringList parts = expr.split('*');
    if (parts.size() == 2) {
      double a = parts[0].toDouble(&ok);
      if (!ok)
        return 0;
      double b = parts[1].toDouble(&ok);
      if (!ok)
        return 0;
      return a * b;
    }
  }
  if (expr.contains('/')) {
    QStringList parts = expr.split('/');
    if (parts.size() == 2) {
      double a = parts[0].toDouble(&ok);
      if (!ok)
        return 0;
      double b = parts[1].toDouble(&ok);
      if (!ok || b == 0)
        return 0;
      return a / b;
    }
  }
  if (expr.contains('+')) {
    int pos = expr.lastIndexOf('+');
    double a = expr.left(pos).toDouble(&ok);
    if (!ok)
      return 0;
    double b = expr.mid(pos + 1).toDouble(&ok);
    if (!ok)
      return 0;
    return a + b;
  }
  if (expr.contains('-') && expr.indexOf('-') > 0) {
    int pos = expr.lastIndexOf('-');
    double a = expr.left(pos).toDouble(&ok);
    if (!ok)
      return 0;
    double b = expr.mid(pos + 1).toDouble(&ok);
    if (!ok)
      return 0;
    return a - b;
  }

  return 0;
}

bool ExpressionEvaluator::isValid(const QString &expression,
                                  const QMap<QString, double> &variables) {
  // Check all referenced variables exist
  QStringList refs = getReferencedVariables(expression);
  for (const QString &ref : refs) {
    if (!variables.contains(ref)) {
      return false;
    }
  }
  return true;
}

QStringList
ExpressionEvaluator::getReferencedVariables(const QString &expression) {
  QStringList result;
  QRegularExpression re("\\b([a-zA-Z_][a-zA-Z0-9_]*)\\b");
  QRegularExpressionMatchIterator it = re.globalMatch(expression);

  while (it.hasNext()) {
    QRegularExpressionMatch match = it.next();
    QString var = match.captured(1);
    // Exclude math functions
    if (var != "sin" && var != "cos" && var != "tan" && var != "sqrt" &&
        var != "abs" && var != "pow") {
      if (!result.contains(var)) {
        result.append(var);
      }
    }
  }
  return result;
}

// ============ ParameterManager ============

ParameterManager::ParameterManager(QObject *parent) : QObject(parent) {
  // Add built-in geometry parameters (read-only)
  addParameter("Volume", 0, ParameterType::Volume);
  addParameter("Area", 0, ParameterType::Area);
  addParameter("LengthX", 0, ParameterType::Length);
  addParameter("LengthY", 0, ParameterType::Length);
  addParameter("LengthZ", 0, ParameterType::Length);
  // Center of Mass parameters
  addParameter("CoM_X", 0, ParameterType::Length);
  addParameter("CoM_Y", 0, ParameterType::Length);
  addParameter("CoM_Z", 0, ParameterType::Length);
}

void ParameterManager::addParameter(const QString &name, double value,
                                    ParameterType type) {
  m_parameters[name] = std::make_shared<Parameter>(name, value, type);
}

void ParameterManager::removeParameter(const QString &name) {
  m_parameters.remove(name);
}

bool ParameterManager::hasParameter(const QString &name) const {
  return m_parameters.contains(name);
}

double ParameterManager::getValue(const QString &name) const {
  if (m_parameters.contains(name)) {
    return m_parameters[name]->value();
  }
  return 0;
}

void ParameterManager::setValue(const QString &name, double value) {
  if (m_parameters.contains(name)) {
    m_parameters[name]->setValue(value);
    emit parameterChanged(name, value);
    evaluateExpressions();
  }
}

void ParameterManager::setExpression(const QString &name,
                                     const QString &expression) {
  if (m_parameters.contains(name)) {
    m_parameters[name]->setExpression(expression);
    buildDependencyGraph();
    evaluateExpressions();
  }
}

void ParameterManager::clearExpression(const QString &name) {
  if (m_parameters.contains(name)) {
    m_parameters[name]->setExpression("");
    buildDependencyGraph();
  }
}

void ParameterManager::updateGeometryParameters(double volume, double area,
                                                double lengthX, double lengthY,
                                                double lengthZ, double comX,
                                                double comY, double comZ) {
  if (m_parameters.contains("Volume")) {
    m_parameters["Volume"]->setValue(volume);
    emit parameterChanged("Volume", volume);
  }
  if (m_parameters.contains("Area")) {
    m_parameters["Area"]->setValue(area);
    emit parameterChanged("Area", area);
  }
  if (m_parameters.contains("LengthX")) {
    m_parameters["LengthX"]->setValue(lengthX);
    emit parameterChanged("LengthX", lengthX);
  }
  if (m_parameters.contains("LengthY")) {
    m_parameters["LengthY"]->setValue(lengthY);
    emit parameterChanged("LengthY", lengthY);
  }
  if (m_parameters.contains("LengthZ")) {
    m_parameters["LengthZ"]->setValue(lengthZ);
    emit parameterChanged("LengthZ", lengthZ);
  }
  // Center of Mass
  if (m_parameters.contains("CoM_X")) {
    m_parameters["CoM_X"]->setValue(comX);
    emit parameterChanged("CoM_X", comX);
  }
  if (m_parameters.contains("CoM_Y")) {
    m_parameters["CoM_Y"]->setValue(comY);
    emit parameterChanged("CoM_Y", comY);
  }
  if (m_parameters.contains("CoM_Z")) {
    m_parameters["CoM_Z"]->setValue(comZ);
    emit parameterChanged("CoM_Z", comZ);
  }

  evaluateExpressions();
}

QList<Parameter *> ParameterManager::allParameters() const {
  QList<Parameter *> result;
  for (auto &p : m_parameters) {
    result.append(p.get());
  }
  return result;
}

QStringList ParameterManager::parameterNames() const {
  return m_parameters.keys();
}

void ParameterManager::evaluateExpressions() {
  // Build variable map
  QMap<QString, double> vars;
  for (auto &p : m_parameters) {
    vars[p->name()] = p->value();
  }

  // Evaluate expressions in dependency order
  for (auto &p : m_parameters) {
    if (p->hasExpression()) {
      if (ExpressionEvaluator::isValid(p->expression(), vars)) {
        double newValue = ExpressionEvaluator::evaluate(p->expression(), vars);
        p->setValue(newValue);
        vars[p->name()] = newValue;
        emit parameterChanged(p->name(), newValue);
      } else {
        emit expressionError(p->name(),
                             "Invalid expression: " + p->expression());
      }
    }
  }
}

void ParameterManager::buildDependencyGraph() {
  m_dependencies.clear();

  for (auto &p : m_parameters) {
    if (p->hasExpression()) {
      QStringList deps =
          ExpressionEvaluator::getReferencedVariables(p->expression());
      m_dependencies[p->name()] = deps;
    }
  }
}

} // namespace core
} // namespace opencad
