/**
 * @file FeatureRecord.h
 * @brief Lightweight parameter storage for SolidWorks-style Edit Feature
 *
 * Stores the operation type, parameters, and base shape for each feature
 * so it can be re-executed with modified parameters later.
 */

#pragma once

#include <QString>
#include <QVariantMap>
#include <TopoDS_Shape.hxx>
#include <memory>
#include <vector>

namespace opencad {

namespace sketch {
class Sketch;
}

namespace core {

/**
 * @struct FeatureRecord
 * @brief Stores parameters of an applied feature for later editing/replay
 */
struct FeatureRecord {
  QString type;             // "Extrude", "Cut", "Revolve", "Sweep", etc.
  QVariantMap parameters;   // Operation-specific params (depth, angle, etc.)
  int sketchIndex = -1;     // Index into document's sketch list (-1 = no sketch)
  int profileIndex = 0;     // Which profile was selected
  TopoDS_Shape baseShape;   // Shape state BEFORE this feature was applied

  // Helper accessors
  double depth() const { return parameters.value("depth", 20.0).toDouble(); }
  bool symmetric() const { return parameters.value("symmetric", false).toBool(); }
  double draftAngle() const { return parameters.value("draftAngle", 0.0).toDouble(); }
  double revolveAngle() const { return parameters.value("revolveAngle", 360.0).toDouble(); }
  QString axisType() const { return parameters.value("axisType", "X").toString(); }
  
  // Display string for feature list
  QString displayString() const {
    if (type == "Extrude") {
      QString s = QString::fromUtf8("\u2705 Extrude (%1)").arg(depth());
      if (symmetric()) s += " symmetric";
      if (std::abs(draftAngle()) > 0.001) s += QString(" draft=%1").arg(draftAngle());
      return s;
    } else if (type == "Cut") {
      return QString::fromUtf8("\u2702\uFE0F Cut (%1)").arg(depth());
    } else if (type == "Revolve") {
      return QString::fromUtf8("\U0001F504 Revolve (%1\u00B0)").arg(revolveAngle());
    }
    return QString::fromUtf8("\u2699\uFE0F ") + type;
  }
};

} // namespace core
} // namespace opencad
