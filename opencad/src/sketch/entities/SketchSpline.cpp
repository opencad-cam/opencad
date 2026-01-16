/**
 * @file SketchSpline.cpp
 * @brief Implementation of SketchSpline with SolidWorks-style tangent handles
 */

#include "SketchSpline.h"
#include <GCPnts_AbscissaPoint.hxx>
#include <Geom2dAPI_Interpolate.hxx>
#include <Geom2dAdaptor_Curve.hxx>
#include <Geom2dLProp_CLProps2d.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColgp_Array1OfPnt2d.hxx>


namespace opencad {
namespace sketch {

SketchSpline::SketchSpline()
    : SketchEntity(), m_splineType(SplineType::FitPoints), m_degree(3),
      m_isClosed(false), m_startTangentMagnitude(1.0),
      m_endTangentMagnitude(1.0), m_useStartTangent(false),
      m_useEndTangent(false), m_curveValid(false) {}

SketchSpline::SketchSpline(const std::vector<gp_Pnt2d> &points)
    : SketchEntity(), m_controlPoints(points),
      m_splineType(SplineType::FitPoints), m_degree(3), m_isClosed(false),
      m_startTangentMagnitude(1.0), m_endTangentMagnitude(1.0),
      m_useStartTangent(false), m_useEndTangent(false), m_curveValid(false) {}

void SketchSpline::setControlPoints(const std::vector<gp_Pnt2d> &points) {
  m_controlPoints = points;
  m_curveValid = false;
}

void SketchSpline::addControlPoint(const gp_Pnt2d &point) {
  m_controlPoints.push_back(point);
  m_curveValid = false;
}

void SketchSpline::insertControlPoint(size_t index, const gp_Pnt2d &point) {
  if (index <= m_controlPoints.size()) {
    m_controlPoints.insert(m_controlPoints.begin() + index, point);
    m_curveValid = false;
  }
}

void SketchSpline::removeControlPoint(size_t index) {
  if (index < m_controlPoints.size()) {
    m_controlPoints.erase(m_controlPoints.begin() + index);
    m_curveValid = false;
  }
}

void SketchSpline::setControlPoint(size_t index, const gp_Pnt2d &point) {
  if (index < m_controlPoints.size()) {
    m_controlPoints[index] = point;
    m_curveValid = false;
  }
}

void SketchSpline::setDegree(int degree) {
  m_degree = std::max(1, std::min(degree, 7));
  m_curveValid = false;
}

// ========== TANGENT HANDLE IMPLEMENTATION ==========

void SketchSpline::setStartTangent(const gp_Vec2d &tangent, double magnitude) {
  m_startTangent = tangent;
  if (m_startTangent.Magnitude() > 1e-10) {
    m_startTangent.Normalize();
  }
  m_startTangentMagnitude = std::max(0.1, magnitude);
  m_useStartTangent = true;
  m_curveValid = false;
}

void SketchSpline::clearStartTangent() {
  m_useStartTangent = false;
  m_curveValid = false;
}

void SketchSpline::setEndTangent(const gp_Vec2d &tangent, double magnitude) {
  m_endTangent = tangent;
  if (m_endTangent.Magnitude() > 1e-10) {
    m_endTangent.Normalize();
  }
  m_endTangentMagnitude = std::max(0.1, magnitude);
  m_useEndTangent = true;
  m_curveValid = false;
}

void SketchSpline::clearEndTangent() {
  m_useEndTangent = false;
  m_curveValid = false;
}

gp_Pnt2d SketchSpline::startTangentHandle() const {
  if (m_controlPoints.empty())
    return gp_Pnt2d(0, 0);

  gp_Pnt2d start = m_controlPoints.front();
  gp_Vec2d dir = m_useStartTangent ? m_startTangent : gp_Vec2d(1, 0);
  double mag = m_startTangentMagnitude * 20.0; // Visual scale

  return gp_Pnt2d(start.X() + dir.X() * mag, start.Y() + dir.Y() * mag);
}

gp_Pnt2d SketchSpline::endTangentHandle() const {
  if (m_controlPoints.empty())
    return gp_Pnt2d(0, 0);

  gp_Pnt2d end = m_controlPoints.back();
  gp_Vec2d dir = m_useEndTangent ? m_endTangent : gp_Vec2d(1, 0);
  double mag = m_endTangentMagnitude * 20.0;

  return gp_Pnt2d(end.X() + dir.X() * mag, end.Y() + dir.Y() * mag);
}

void SketchSpline::setStartTangentFromHandle(const gp_Pnt2d &handlePos) {
  if (m_controlPoints.empty())
    return;

  gp_Pnt2d start = m_controlPoints.front();
  gp_Vec2d dir(handlePos.X() - start.X(), handlePos.Y() - start.Y());
  double mag = dir.Magnitude() / 20.0;

  if (mag > 0.01) {
    setStartTangent(dir, mag);
  }
}

void SketchSpline::setEndTangentFromHandle(const gp_Pnt2d &handlePos) {
  if (m_controlPoints.empty())
    return;

  gp_Pnt2d end = m_controlPoints.back();
  gp_Vec2d dir(handlePos.X() - end.X(), handlePos.Y() - end.Y());
  double mag = dir.Magnitude() / 20.0;

  if (mag > 0.01) {
    setEndTangent(dir, mag);
  }
}

// ========== CURVE BUILDING ==========

void SketchSpline::rebuildCurve() const {
  if (m_controlPoints.size() < 2) {
    m_curve = Handle(Geom2d_BSplineCurve)();
    m_curveValid = false;
    return;
  }

  try {
    // Create point array
    TColgp_Array1OfPnt2d points(1, static_cast<int>(m_controlPoints.size()));
    for (size_t i = 0; i < m_controlPoints.size(); ++i) {
      points.SetValue(static_cast<int>(i + 1), m_controlPoints[i]);
    }

    // Use interpolation to create a smooth curve through points
    Handle(TColgp_HArray1OfPnt2d) hPoints = new TColgp_HArray1OfPnt2d(points);
    Geom2dAPI_Interpolate interpolator(hPoints, m_isClosed, 1e-6);

    // Apply tangent constraints if set
    if (m_useStartTangent || m_useEndTangent) {
      gp_Vec2d startTan =
          m_useStartTangent
              ? gp_Vec2d(m_startTangent.X() * m_startTangentMagnitude,
                         m_startTangent.Y() * m_startTangentMagnitude)
              : gp_Vec2d(0, 0);

      gp_Vec2d endTan = m_useEndTangent
                            ? gp_Vec2d(m_endTangent.X() * m_endTangentMagnitude,
                                       m_endTangent.Y() * m_endTangentMagnitude)
                            : gp_Vec2d(0, 0);

      if (m_useStartTangent && m_useEndTangent) {
        interpolator.Load(startTan, endTan);
      } else if (m_useStartTangent) {
        // Only start tangent - use automatic end
        interpolator.Load(startTan, endTan, false);
      } else if (m_useEndTangent) {
        // Only end tangent - use automatic start
        interpolator.Load(startTan, endTan, false);
      }
    }

    interpolator.Perform();

    if (interpolator.IsDone()) {
      m_curve = interpolator.Curve();
      m_curveValid = true;
    } else {
      m_curve = Handle(Geom2d_BSplineCurve)();
      m_curveValid = false;
    }
  } catch (...) {
    m_curve = Handle(Geom2d_BSplineCurve)();
    m_curveValid = false;
  }
}

Handle(Geom2d_Curve) SketchSpline::curve() const {
  if (!m_curveValid) {
    rebuildCurve();
  }
  return m_curve;
}

gp_Pnt2d SketchSpline::startPoint() const {
  if (m_controlPoints.empty()) {
    return gp_Pnt2d(0, 0);
  }
  return m_controlPoints.front();
}

gp_Pnt2d SketchSpline::endPoint() const {
  if (m_controlPoints.empty()) {
    return gp_Pnt2d(0, 0);
  }
  return m_isClosed ? m_controlPoints.front() : m_controlPoints.back();
}

gp_Pnt2d SketchSpline::midPoint() const { return pointAtParameter(0.5); }

gp_Pnt2d SketchSpline::pointAtParameter(double u) const {
  if (!m_curveValid) {
    rebuildCurve();
  }

  if (m_curve.IsNull()) {
    return gp_Pnt2d(0, 0);
  }

  double first = m_curve->FirstParameter();
  double last = m_curve->LastParameter();
  double param = first + u * (last - first);

  return m_curve->Value(param);
}

gp_Vec2d SketchSpline::tangentAtParameter(double u) const {
  if (!m_curveValid) {
    rebuildCurve();
  }

  if (m_curve.IsNull()) {
    return gp_Vec2d(1, 0);
  }

  double first = m_curve->FirstParameter();
  double last = m_curve->LastParameter();
  double param = first + u * (last - first);

  gp_Pnt2d point;
  gp_Vec2d tangent;
  m_curve->D1(param, point, tangent);

  if (tangent.Magnitude() > 1e-10) {
    tangent.Normalize();
  }
  return tangent;
}

double SketchSpline::curvatureAtParameter(double u) const {
  if (!m_curveValid) {
    rebuildCurve();
  }

  if (m_curve.IsNull()) {
    return 0.0;
  }

  double first = m_curve->FirstParameter();
  double last = m_curve->LastParameter();
  double param = first + u * (last - first);

  Geom2dLProp_CLProps2d props(m_curve, 2, 1e-10);
  props.SetParameter(param);

  if (props.IsTangentDefined()) {
    return props.Curvature();
  }
  return 0.0;
}

double SketchSpline::length() const {
  if (!m_curveValid) {
    rebuildCurve();
  }

  if (m_curve.IsNull()) {
    return 0.0;
  }

  Geom2dAdaptor_Curve adaptor(m_curve);
  return GCPnts_AbscissaPoint::Length(adaptor);
}

int SketchSpline::baseDOF() const {
  int dof = static_cast<int>(m_controlPoints.size() * 2);
  if (m_useStartTangent)
    dof += 2; // direction + magnitude
  if (m_useEndTangent)
    dof += 2;
  return dof;
}

double SketchSpline::getParameter(int index) const {
  int pointIndex = index / 2;
  int coord = index % 2;

  if (pointIndex >= 0 &&
      pointIndex < static_cast<int>(m_controlPoints.size())) {
    return coord == 0 ? m_controlPoints[pointIndex].X()
                      : m_controlPoints[pointIndex].Y();
  }
  return 0.0;
}

void SketchSpline::setParameter(int index, double value) {
  int pointIndex = index / 2;
  int coord = index % 2;

  if (pointIndex >= 0 &&
      pointIndex < static_cast<int>(m_controlPoints.size())) {
    if (coord == 0) {
      m_controlPoints[pointIndex].SetX(value);
    } else {
      m_controlPoints[pointIndex].SetY(value);
    }
    m_curveValid = false;
  }
}

SketchEntity::Ptr SketchSpline::clone() const {
  auto cloned = std::make_shared<SketchSpline>(m_controlPoints);
  cloned->setDegree(m_degree);
  cloned->setClosed(m_isClosed);
  cloned->setConstruction(isConstruction());
  cloned->setSplineType(m_splineType);
  if (m_useStartTangent) {
    cloned->setStartTangent(m_startTangent, m_startTangentMagnitude);
  }
  if (m_useEndTangent) {
    cloned->setEndTangent(m_endTangent, m_endTangentMagnitude);
  }
  return cloned;
}

bool SketchSpline::isValid() const { return m_controlPoints.size() >= 2; }

} // namespace sketch
} // namespace opencad
