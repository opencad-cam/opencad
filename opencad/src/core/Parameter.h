/**
 * @file Parameter.h
 * @brief Parametric system - NX-style associative parameters
 */

#pragma once

#include <QMap>
#include <QObject>
#include <QString>
#include <functional>
#include <memory>

namespace opencad {
namespace core {

/**
 * @brief Parameter types
 */
enum class ParameterType {
  Length, // mm
  Area,   // mm²
  Volume, // mm³
  Angle,  // degrees
  Mass,   // kg
  Scalar  // dimensionless
};

/**
 * @brief Single parameter with value and optional expression
 */
class Parameter {
public:
  Parameter(const QString &name, double value,
            ParameterType type = ParameterType::Scalar);

  // Getters
  QString name() const { return m_name; }
  double value() const { return m_value; }
  ParameterType type() const { return m_type; }
  QString expression() const { return m_expression; }
  bool hasExpression() const { return !m_expression.isEmpty(); }

  // Setters
  void setValue(double value);
  void setExpression(const QString &expr);

  // Unit string
  QString unitString() const;
  QString displayString() const;

private:
  QString m_name;
  double m_value;
  ParameterType m_type;
  QString m_expression; // Optional: "width * height"
};

/**
 * @brief Expression evaluator for parametric relationships
 */
class ExpressionEvaluator {
public:
  // Evaluate expression with variable values
  static double evaluate(const QString &expression,
                         const QMap<QString, double> &variables);

  // Check if expression is valid
  static bool isValid(const QString &expression,
                      const QMap<QString, double> &variables);

  // Get referenced variable names
  static QStringList getReferencedVariables(const QString &expression);
};

/**
 * @brief Parameter manager with dependency tracking
 */
class ParameterManager : public QObject {
  Q_OBJECT

public:
  explicit ParameterManager(QObject *parent = nullptr);

  // Add/remove parameters
  void addParameter(const QString &name, double value, ParameterType type);
  void removeParameter(const QString &name);
  bool hasParameter(const QString &name) const;

  // Get/set values
  double getValue(const QString &name) const;
  void setValue(const QString &name, double value);

  // Expression binding
  void setExpression(const QString &name, const QString &expression);
  void clearExpression(const QString &name);

  // Built-in geometry parameters (read-only, auto-calculated)
  void updateGeometryParameters(double volume, double area, double lengthX,
                                double lengthY, double lengthZ, double comX = 0,
                                double comY = 0, double comZ = 0);

  // Get all parameters
  QList<Parameter *> allParameters() const;
  QStringList parameterNames() const;

signals:
  void parameterChanged(const QString &name, double value);
  void expressionError(const QString &name, const QString &error);

private:
  void evaluateExpressions();
  void buildDependencyGraph();

  QMap<QString, std::shared_ptr<Parameter>> m_parameters;
  QMap<QString, QStringList> m_dependencies; // param -> depends on
};

} // namespace core
} // namespace opencad
