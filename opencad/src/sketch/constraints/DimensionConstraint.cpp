/**
 * @file DimensionConstraint.cpp
 * @brief Implementation of DimensionConstraint
 */

#include "DimensionConstraint.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace opencad {
namespace sketch {

DimensionConstraint::DimensionConstraint()
    : Constraint()
    , m_dimType(DimensionType::PointToPoint)
    , m_dimension(0.0)
{
}

DimensionConstraint::Ptr DimensionConstraint::createPointToPoint(
    SketchPoint::Ptr p1, SketchPoint::Ptr p2, double distance) {
    auto c = std::make_shared<DimensionConstraint>();
    c->m_dimType = DimensionType::PointToPoint;
    c->m_point1 = p1;
    c->m_point2 = p2;
    c->m_dimension = distance;
    return c;
}

DimensionConstraint::Ptr DimensionConstraint::createLineLength(
    SketchLine::Ptr line, double length) {
    auto c = std::make_shared<DimensionConstraint>();
    c->m_dimType = DimensionType::LineLength;
    c->m_line1 = line;
    c->m_dimension = length;
    return c;
}

DimensionConstraint::Ptr DimensionConstraint::createRadius(
    SketchCircle::Ptr circle, double radius) {
    auto c = std::make_shared<DimensionConstraint>();
    c->m_dimType = DimensionType::Radius;
    c->m_circle = circle;
    c->m_dimension = radius;
    return c;
}

DimensionConstraint::Ptr DimensionConstraint::createRadius(
    SketchArc::Ptr arc, double radius) {
    auto c = std::make_shared<DimensionConstraint>();
    c->m_dimType = DimensionType::Radius;
    c->m_arc = arc;
    c->m_dimension = radius;
    return c;
}

DimensionConstraint::Ptr DimensionConstraint::createAngle(
    SketchLine::Ptr line1, SketchLine::Ptr line2, double angleDegrees) {
    auto c = std::make_shared<DimensionConstraint>();
    c->m_dimType = DimensionType::Angle;
    c->m_line1 = line1;
    c->m_line2 = line2;
    c->m_dimension = angleDegrees * M_PI / 180.0; // Store in radians
    return c;
}

DimensionConstraint::Ptr DimensionConstraint::createLineToLine(
    SketchLine::Ptr line1, SketchLine::Ptr line2, double distance) {
    auto c = std::make_shared<DimensionConstraint>();
    c->m_dimType = DimensionType::LineToLine;
    c->m_line1 = line1;
    c->m_line2 = line2;
    c->m_dimension = distance;
    return c;
}

std::string DimensionConstraint::typeName() const {
    switch (m_dimType) {
        case DimensionType::PointToPoint: return "Distance";
        case DimensionType::PointToLine: return "Distance";
        case DimensionType::LineLength: return "Length";
        case DimensionType::LineToLine: return "Distance";
        case DimensionType::Radius: return "Radius";
        case DimensionType::Diameter: return "Diameter";
        case DimensionType::Angle: return "Angle";
        default: return "Dimension";
    }
}

std::vector<SketchEntity::Ptr> DimensionConstraint::entities() const {
    std::vector<SketchEntity::Ptr> result;
    if (m_point1) result.push_back(m_point1);
    if (m_point2) result.push_back(m_point2);
    if (m_line1) result.push_back(m_line1);
    if (m_line2) result.push_back(m_line2);
    if (m_circle) result.push_back(m_circle);
    if (m_arc) result.push_back(m_arc);
    return result;
}

int DimensionConstraint::entityCount() const {
    int count = 0;
    if (m_point1) count++;
    if (m_point2) count++;
    if (m_line1) count++;
    if (m_line2) count++;
    if (m_circle) count++;
    if (m_arc) count++;
    return count;
}

double DimensionConstraint::measuredValue() const {
    switch (m_dimType) {
        case DimensionType::PointToPoint:
            if (m_point1 && m_point2) {
                return m_point1->position().Distance(m_point2->position());
            }
            break;
            
        case DimensionType::LineLength:
            if (m_line1) {
                return m_line1->length();
            }
            break;
            
        case DimensionType::Radius:
            if (m_circle) return m_circle->radius();
            if (m_arc) return m_arc->radius();
            break;
            
        case DimensionType::Angle:
            if (m_line1 && m_line2) {
                double a1 = m_line1->angle();
                double a2 = m_line2->angle();
                return std::abs(a2 - a1);
            }
            break;

        case DimensionType::LineToLine:
            if (m_line1 && m_line2) {
                // Midpoint of line1
                double mx = (m_line1->startPoint().X() + m_line1->endPoint().X()) / 2.0;
                double my = (m_line1->startPoint().Y() + m_line1->endPoint().Y()) / 2.0;
                // Direction vector of line2
                double ux = m_line2->endPoint().X() - m_line2->startPoint().X();
                double uy = m_line2->endPoint().Y() - m_line2->startPoint().Y();
                double len = std::sqrt(ux*ux + uy*uy);
                if (len < 1e-10) return 0.0;
                // Perpendicular distance from midpoint of line1 to infinite line2
                double px = m_line2->startPoint().X();
                double py = m_line2->startPoint().Y();
                double cross = (mx - px)*uy - (my - py)*ux;
                return std::abs(cross / len);
            }
            break;
            
        default:
            break;
    }
    return 0.0;
}

double DimensionConstraint::error() const {
    return measuredValue() - m_dimension;
}

std::vector<double> DimensionConstraint::jacobian() const {
    switch (m_dimType) {
        case DimensionType::PointToPoint:
            if (m_point1 && m_point2) {
                double dx = m_point2->x() - m_point1->x();
                double dy = m_point2->y() - m_point1->y();
                double d = std::sqrt(dx*dx + dy*dy);
                if (d < 1e-10) d = 1e-10;
                return {-dx/d, -dy/d, dx/d, dy/d};
            }
            break;
            
        case DimensionType::LineLength:
            if (m_line1) {
                double dx = m_line1->endPoint().X() - m_line1->startPoint().X();
                double dy = m_line1->endPoint().Y() - m_line1->startPoint().Y();
                double d = m_line1->length();
                if (d < 1e-10) d = 1e-10;
                return {-dx/d, -dy/d, dx/d, dy/d};
            }
            break;
            
        case DimensionType::Radius:
            return {0.0, 0.0, 1.0};

        case DimensionType::LineToLine:
            if (m_line1 && m_line2) {
                // Variables: [l1.sx, l1.sy, l1.ex, l1.ey,  l2.sx, l2.sy, l2.ex, l2.ey]
                // Error = |cross| / L   where:
                //   mx = (l1.sx + l1.ex)/2,  my = (l1.sy + l1.ey)/2
                //   ux = l2.ex - l2.sx,  uy = l2.ey - l2.sy
                //   L  = sqrt(ux^2 + uy^2)
                //   cross = (mx - l2.sx)*uy - (my - l2.sy)*ux
                double ax = m_line1->startPoint().X(), ay = m_line1->startPoint().Y();
                double bx = m_line1->endPoint().X(),  by = m_line1->endPoint().Y();
                double px = m_line2->startPoint().X(), py = m_line2->startPoint().Y();
                double rx = m_line2->endPoint().X(),   ry = m_line2->endPoint().Y();

                double mx = (ax + bx) / 2.0;
                double my = (ay + by) / 2.0;
                double ux = rx - px;
                double uy = ry - py;
                double L  = std::sqrt(ux*ux + uy*uy);
                if (L < 1e-10) return {};

                double cross = (mx - px)*uy - (my - py)*ux;
                double s     = (cross >= 0) ? 1.0 : -1.0;  // sign(cross)
                double L2    = L * L;
                double L3    = L2 * L;

                // Partial derivatives of |cross|/L with respect to each param:
                // For line1 endpoints (cross depends linearly on mx,my = average of line1 endpoints):
                double dJ_ax = s * uy / (2.0 * L);
                double dJ_ay = -s * ux / (2.0 * L);
                double dJ_bx = s * uy / (2.0 * L);
                double dJ_by = -s * ux / (2.0 * L);

                // For line2 start point (px, py):
                // ∂cross/∂px = -uy + (my-py)  -- wait: cross=(mx-px)*uy-(my-py)*ux
                //              = -uy
                // (my-py) term: ∂(-( my-py)*ux)/∂px = -(my-py)*(-1) = (my-py)  <-- ux=rx-px, ∂ux/∂px=-1
                // So ∂cross/∂px = (-1)*uy - (my-py)*(-1) = -uy + (my-py)
                // ∂L/∂px = -ux/L
                double dCross_dpx = -uy + (my - py);
                double dL_dpx     = -ux / L;
                double dJ_px = s * (dCross_dpx * L - cross * dL_dpx * L / L) / L2;
                // simplify: s*(dCross_dpx/L + cross*ux/L3)
                dJ_px = s * (dCross_dpx / L + cross * ux / L3);

                // ∂cross/∂py = (mx-px)*(-1) + ux  [uy=ry-py, ∂uy/∂py=-1; ux=rx-px, ∂ux/∂py=0]
                //             But wait: ∂(-( my-py)*ux)/∂py = -(-1)*ux = ux
                //             ∂((mx-px)*uy)/∂py = (mx-px)*(-1)
                //             So ∂cross/∂py = -(mx-px) + ux
                double dCross_dpy = -(mx - px) + ux;
                double dL_dpy     = -uy / L;
                dJ_px = s * (dCross_dpx / L + cross * ux / L3);
                double dJ_py = s * (dCross_dpy / L + cross * uy / L3);

                // For line2 end point (rx, ry):
                // ∂cross/∂rx: ux=rx-px, ∂ux/∂rx=1; uy=ry-py, ∂uy/∂rx=0
                //   ∂cross/∂rx = -(my-py)*1 = -(my-py)
                // ∂L/∂rx = ux/L
                double dCross_drx = -(my - py);
                double dL_drx     = ux / L;
                double dJ_rx = s * (dCross_drx / L - cross * ux / L3);

                // ∂cross/∂ry: uy=ry-py, ∂uy/∂ry=1; ux unchanged
                //   ∂cross/∂ry = (mx-px)*1 = (mx-px)
                // ∂L/∂ry = uy/L
                double dCross_dry = (mx - px);
                double dL_dry     = uy / L;
                double dJ_ry = s * (dCross_dry / L - cross * uy / L3);

                // Return order: [l1.sx, l1.sy, l1.ex, l1.ey, l2.sx, l2.sy, l2.ex, l2.ey]
                return {dJ_ax, dJ_ay, dJ_bx, dJ_by, dJ_px, dJ_py, dJ_rx, dJ_ry};
            }
            break;
            
        default:
            break;
    }
    return {};
}

Constraint::Ptr DimensionConstraint::clone() const {
    auto cloned = std::make_shared<DimensionConstraint>();
    cloned->m_dimType = m_dimType;
    cloned->m_dimension = m_dimension;
    cloned->m_point1 = m_point1;
    cloned->m_point2 = m_point2;
    cloned->m_line1 = m_line1;
    cloned->m_line2 = m_line2;
    cloned->m_circle = m_circle;
    cloned->m_arc = m_arc;
    cloned->setDriving(isDriving());
    cloned->setEnabled(isEnabled());
    return cloned;
}

} // namespace sketch
} // namespace opencad
