#include "ConstraintSolver.h"
#include "GeometryHelper.h"
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRep_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Shape.hxx>
#include <cmath>
#include <gp_Pnt.hxx>
#include <gp_Quaternion.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <gp_XYZ.hxx>

namespace opencad {
namespace assembly {

bool ConstraintSolver::solve(Assembly &assembly, int maxIterations,
                             double tolerance) {
  m_iterations = 0;
  m_errorMessage.clear();
  m_residual = computeTotalResidual(assembly);

  if (assembly.getConstraints().empty()) {
    return true; // No constraints to solve
  }

  // Check if all components are fixed
  auto components = assembly.getComponents();
  bool hasMovable = false;
  for (const auto &comp : components) {
    if (!comp->isFixed()) {
      hasMovable = true;
      break;
    }
  }

  if (!hasMovable) {
    m_errorMessage = "All components are fixed, cannot solve";
    return false;
  }

  // Iterative solver loop
  for (int iter = 0; iter < maxIterations; ++iter) {
    m_iterations = iter + 1;

    // Process each constraint
    for (const auto &constraint : assembly.getConstraints()) {
      auto c1 = constraint->getComponent1();
      auto c2 = constraint->getComponent2();

      if (!c1 || !c2)
        continue;

      SolverDelta delta;

      switch (constraint->getType()) {
      case ConstraintType::Coincident:
        delta = computeCoincidentCorrection(constraint);
        break;

      case ConstraintType::Distance:
        delta = computeDistanceCorrection(constraint);
        break;

      case ConstraintType::Parallel:
        delta = computeParallelCorrection(constraint);
        break;
      case ConstraintType::Perpendicular:
        delta = computePerpendicularCorrection(constraint);
        break;
      case ConstraintType::Concentric:
        delta = computeConcentricCorrection(constraint);
        break;
      case ConstraintType::Tangent:
        delta = computeTangentCorrection(constraint);
        break;
      case ConstraintType::Angle:
        delta = computeAngleCorrection(constraint);
        break;
      case ConstraintType::Lock:
        delta = computeLockCorrection(constraint);
        break;
      case ConstraintType::Gear:
        delta = computeGearCorrection(constraint);
        break;
      case ConstraintType::Screw:
        delta = computeScrewCorrection(constraint);
        break;
      }

      // Apply damped movement to non-fixed component
      if (!c1->isFixed() && c2->isFixed()) {
        // Move c1 towards c2
        SolverDelta d = delta;
        d.translation = d.translation * m_dampingFactor;
        d.rotationAngle = d.rotationAngle * m_dampingFactor;
        applyDelta(c1, d);
      } else if (c1->isFixed() && !c2->isFixed()) {
        // Move c2 towards c1 (opposite direction)
        SolverDelta d = delta;
        d.translation = d.translation * (-m_dampingFactor);
        d.rotationAngle = d.rotationAngle * (-m_dampingFactor);
        // Note: Negative rotation angle typically reverses the rotation
        // direction around same axis

        applyDelta(c2, d);
      } else if (!c1->isFixed() && !c2->isFixed()) {
        // Split the movement between both
        SolverDelta d1 = delta;
        d1.translation = d1.translation * (m_dampingFactor * 0.5);
        d1.rotationAngle = d1.rotationAngle * (m_dampingFactor * 0.5);

        SolverDelta d2 = delta;
        d2.translation = d2.translation * (-m_dampingFactor * 0.5);
        d2.rotationAngle = d2.rotationAngle * (-m_dampingFactor * 0.5);

        applyDelta(c1, d1);
        applyDelta(c2, d2);
      }
    }

    // Check convergence
    m_residual = computeTotalResidual(assembly);
    if (m_residual < tolerance) {
      return true;
    }
  }

  m_errorMessage = "Did not converge within max iterations";
  return false;
}

ConstraintSolver::SolverDelta ConstraintSolver::computeCoincidentCorrection(
    std::shared_ptr<AssemblyConstraint> constraint) {
  SolverDelta delta;
  auto c1 = constraint->getComponent1();
  auto c2 = constraint->getComponent2();

  TopoDS_Shape shape1, shape2;

  // Handle SubShape 1
  if (!constraint->getSubShape1().IsNull()) {
    BRepBuilderAPI_Transform xform(constraint->getSubShape1(),
                                   c1->getPlacement());
    shape1 = xform.Shape();
  } else {
    shape1 = c1->getTransformedShape();
  }

  // Handle SubShape 2
  if (!constraint->getSubShape2().IsNull()) {
    BRepBuilderAPI_Transform xform(constraint->getSubShape2(),
                                   c2->getPlacement());
    shape2 = xform.Shape();
  } else {
    shape2 = c2->getTransformedShape();
  }

  if (shape1.IsNull() || shape2.IsNull()) {
    return delta;
  }

  // Optimization: Analytical Plane-Plane Coincidence
  gp_Pln p1, p2;
  if (GeometryHelper::getPlane(shape1, p1) &&
      GeometryHelper::getPlane(shape2, p2)) {
    // 1. Rotation (Align Normals)
    gp_Vec n1 = p1.Axis().Direction();
    gp_Vec n2 = p2.Axis().Direction();

    if (shape1.Orientation() == TopAbs_REVERSED)
      n1.Reverse();
    if (shape2.Orientation() == TopAbs_REVERSED)
      n2.Reverse();

    // Target: n1 should be anti-parallel to n2 (opposing faces)
    // Default: n1 aligns with -n2. Flipped: n1 aligns with n2.
    gp_Vec targetN1 = constraint->isFlipped() ? n2 : -n2;

    if (n1.Magnitude() > 1e-6 && targetN1.Magnitude() > 1e-6) {
      double angle = n1.Angle(targetN1);
      if (std::abs(angle) > 1e-3) {
        delta.rotationAxis = n1.Crossed(targetN1);
        if (delta.rotationAxis.Magnitude() < 1e-6) {
          // 180 deg flip
          gp_Vec arbitrary(1, 0, 0);
          if (n1.IsParallel(arbitrary, 1e-2))
            arbitrary = gp_Vec(0, 1, 0);
          delta.rotationAxis = n1.Crossed(arbitrary);
        }
        delta.rotationAngle = angle;
      }
    }

    // 2. Translation (Plane to Plane distance)
    // We want p1 to be on p2.
    // Distance from p1 location to p2.
    // Calculate signed distance from origin of p1 to plane p2
    double A, B, C, D;
    p2.Coefficients(A, B, C, D);
    double val = A * p1.Location().X() + B * p1.Location().Y() +
                 C * p1.Location().Z() + D;

    // We want to move p1 by vector V such that new pos adheres to plane eq.
    // Simplest approach: Move along plane normal (gradient descent direction)
    // p2 normal is (A, B, C).
    // Translation = -val * Normal
    gp_Vec normal(A, B, C);
    delta.translation = normal * (-val);

    return delta;
  }

  // Fallback to General BRepExtrema
  try {
    BRepExtrema_DistShapeShape distCalc(shape1, shape2);
    if (distCalc.IsDone() && distCalc.NbSolution() > 0) {
      gp_Pnt pt1 = distCalc.PointOnShape1(1);
      gp_Pnt pt2 = distCalc.PointOnShape2(1);

      // Vector from p1 to p2
      delta.translation = gp_Vec(pt1, pt2);

      // 2. Rotation (Align Normals for Faces)
      if (shape1.ShapeType() == TopAbs_FACE &&
          shape2.ShapeType() == TopAbs_FACE) {

        TopoDS_Face f1 = TopoDS::Face(shape1);
        TopoDS_Face f2 = TopoDS::Face(shape2);

        BRepAdaptor_Surface surf1(f1);
        BRepAdaptor_Surface surf2(f2);

        // Find U,V parameters for p1 on surf1
        double u1 = (surf1.FirstUParameter() + surf1.LastUParameter()) * 0.5;
        double v1 = (surf1.FirstVParameter() + surf1.LastVParameter()) * 0.5;
        double u2 = (surf2.FirstUParameter() + surf2.LastUParameter()) * 0.5;
        double v2 = (surf2.FirstVParameter() + surf2.LastVParameter()) * 0.5;

        gp_Pnt tmp;
        gp_Vec d1u, d1v;
        surf1.D1(u1, v1, tmp, d1u, d1v);
        gp_Vec normal1 = d1u.Crossed(d1v);

        gp_Vec d2u, d2v;
        surf2.D1(u2, v2, tmp, d2u, d2v);
        gp_Vec normal2 = d2u.Crossed(d2v);

        if (f1.Orientation() == TopAbs_REVERSED)
          normal1.Reverse();
        if (f2.Orientation() == TopAbs_REVERSED)
          normal2.Reverse();

        normal1.Normalize();
        normal2.Normalize();

        gp_Vec targetN1 = constraint->isFlipped() ? normal2 : -normal2;

        if (normal1.Magnitude() > 1e-6 && targetN1.Magnitude() > 1e-6) {
          double angle = normal1.Angle(targetN1);
          if (std::abs(angle) > 1e-3) {
            delta.rotationAxis = normal1.Crossed(targetN1);
            if (delta.rotationAxis.Magnitude() < 1e-6) {
              if (angle > 1.0) { // Near PI
                gp_Vec arbitrary(1, 0, 0);
                if (normal1.IsParallel(arbitrary, 1e-2))
                  arbitrary = gp_Vec(0, 1, 0);
                delta.rotationAxis = normal1.Crossed(arbitrary);
              }
            }
            delta.rotationAngle = angle;
          }
        }
      }
    }
  } catch (...) {
  }

  return delta;
}

ConstraintSolver::SolverDelta ConstraintSolver::computeDistanceCorrection(
    std::shared_ptr<AssemblyConstraint> constraint) {
  SolverDelta delta;
  auto c1 = constraint->getComponent1();
  auto c2 = constraint->getComponent2();
  double targetDist = constraint->getValue();

  TopoDS_Shape shape1, shape2;

  // Handle SubShape 1
  if (!constraint->getSubShape1().IsNull()) {
    BRepBuilderAPI_Transform xform(constraint->getSubShape1(),
                                   c1->getPlacement());
    shape1 = xform.Shape();
  } else {
    shape1 = c1->getTransformedShape();
  }

  // Handle SubShape 2
  if (!constraint->getSubShape2().IsNull()) {
    BRepBuilderAPI_Transform xform(constraint->getSubShape2(),
                                   c2->getPlacement());
    shape2 = xform.Shape();
  } else {
    shape2 = c2->getTransformedShape();
  }

  if (shape1.IsNull() || shape2.IsNull()) {
    return delta;
  }

  // Optimization: Analytical Plane-Plane Distance
  gp_Pln p1, p2;
  if (GeometryHelper::getPlane(shape1, p1) &&
      GeometryHelper::getPlane(shape2, p2)) {

    // We assume no rotation needed for Distance constraint for now in this MVP,
    // or we might want to align them parallel first?
    // Usually Distance constraint implies parallel planes.
    // Let's enforce parallel alignment first if needed, similar to Coincident.

    gp_Vec n1 = p1.Axis().Direction();
    gp_Vec n2 = p2.Axis().Direction();

    if (shape1.Orientation() == TopAbs_REVERSED)
      n1.Reverse();
    if (shape2.Orientation() == TopAbs_REVERSED)
      n2.Reverse();

    // Default: n1 aligns with -n2 (opposing faces, standard distance).
    gp_Vec targetN1 = -n2; // Distance usually between opposing faces

    if (n1.Magnitude() > 1e-6 && targetN1.Magnitude() > 1e-6) {
      double angle = n1.Angle(targetN1);
      if (std::abs(angle) > 1e-3) {
        delta.rotationAxis = n1.Crossed(targetN1);
        if (delta.rotationAxis.Magnitude() < 1e-6) {
          gp_Vec arbitrary(1, 0, 0);
          if (n1.IsParallel(arbitrary, 1e-2))
            arbitrary = gp_Vec(0, 1, 0);
          delta.rotationAxis = n1.Crossed(arbitrary);
        }
        delta.rotationAngle = angle;
      }
    }

    // Translation
    // Calculate current signed distance
    double A, B, C, D;
    p2.Coefficients(A, B, C, D);
    double val = A * p1.Location().X() + B * p1.Location().Y() +
                 C * p1.Location().Z() + D;

    // val is positive if p1 is on the normal side of p2.
    // If faces are opposing (n1 = -n2), and we want positive distance:
    // This depends on how distance is defined.
    // Currently, let's look at absolute difference for magnitude, and normal
    // for direction.

    // Current 'gap' is roughly |val|.
    // We want |gap| to be targetDist.
    // We should move p1 along p2's normal.
    // If we move p1 by -val * n2, we get to 0.
    // We want to be at targetDist.
    // Move amount = (-val) + (targetDist * Sign(val?))
    // Actually simpler:
    // Goal: Directed distance from p2 to p1 should be targetDist?
    // Or just distance regardless of side.

    // Simplification: Move to 0 then move back by targetDist.
    // Move to 0: delta = -val * n2.
    // Then move out: delta += targetDist * n2.
    // Net: (targetDist - val) * n2.

    gp_Vec normal(A, B, C); // This is n2 usually
    delta.translation =
        normal * (targetDist - val); // Use signed distance logic

    return delta;
  }

  try {
    BRepExtrema_DistShapeShape distCalc(shape1, shape2);
    if (distCalc.IsDone() && distCalc.NbSolution() > 0) {
      gp_Pnt p1 = distCalc.PointOnShape1(1);
      gp_Pnt p2 = distCalc.PointOnShape2(1);
      double currentDist = distCalc.Value();

      if (currentDist < 1e-9) {
        delta.translation = gp_Vec(0, 0, targetDist);
      } else {
        gp_Vec dir(p1, p2); // Vector from 1 to 2
        dir.Normalize();

        // We want to move p1.
        // dir is p2 - p1.
        // If we move p1 by dir * dist, p1 goes to p2.
        // We want final dist to be targetDist.
        // We want to move p1 by (currentDist - targetDist) towards p2.

        double moveAmount = currentDist - targetDist;
        delta.translation = dir * moveAmount;
      }
    }
  } catch (...) {
  }

  return delta;
}

double ConstraintSolver::computeDistance(
    std::shared_ptr<AssemblyConstraint> constraint) {
  auto c1 = constraint->getComponent1();
  auto c2 = constraint->getComponent2();

  TopoDS_Shape shape1, shape2;

  // Handle SubShape 1
  if (!constraint->getSubShape1().IsNull()) {
    BRepBuilderAPI_Transform xform(constraint->getSubShape1(),
                                   c1->getPlacement());
    shape1 = xform.Shape();
  } else {
    shape1 = c1->getTransformedShape();
  }

  // Handle SubShape 2
  if (!constraint->getSubShape2().IsNull()) {
    BRepBuilderAPI_Transform xform(constraint->getSubShape2(),
                                   c2->getPlacement());
    shape2 = xform.Shape();
  } else {
    shape2 = c2->getTransformedShape();
  }

  if (shape1.IsNull() || shape2.IsNull()) {
    return 0.0;
  }

  try {
    BRepExtrema_DistShapeShape distCalc(shape1, shape2);
    if (distCalc.IsDone()) {
      return distCalc.Value();
    }
  } catch (...) {
  }

  return 0.0;
}

void ConstraintSolver::applyDelta(std::shared_ptr<Component> component,
                                  const SolverDelta &delta) {
  if (!component)
    return;

  gp_Trsf currentTrsf = component->getPlacement();
  gp_Trsf updateTrsf;

  // 1. Rotation
  if (delta.hasRotation()) {
    gp_Quaternion quat;
    quat.SetVectorAndAngle(delta.rotationAxis, delta.rotationAngle);
    updateTrsf.SetRotation(quat);

    // Rotate around the component's center (or origin of local frame)
    // For now, we rotate around the origin of the delta application point?
    // Actually, simple rotation around centroid is safer, but complex.
    // Let's rotate around the component's current location origin for stability
    // or just multiply the transform.

    // Strategy: Apply rotation to the current orientation
  }

  // 2. Translation
  updateTrsf.SetTranslationPart(delta.translation.XYZ());

  // Combine: New = Update * Old
  // Or New = Old * Update?
  // Constraints are usually world-space deltas.
  // If delta is in World Space:
  // We want to move component by 'translation' V.
  // We want to rotate component by 'rotation' R.

  // Translation part is easy:
  gp_XYZ newTrans =
      currentTrsf.TranslationPart().Added(delta.translation.XYZ());
  currentTrsf.SetTranslationPart(newTrans);

  // Rotation part:
  if (delta.hasRotation()) {
    // Rotation axis is in World Space.
    gp_Ax1 axis(currentTrsf.TranslationPart(), // Rotate around current position
                gp_Dir(delta.rotationAxis));   // Axis direction

    gp_Trsf rotation;
    rotation.SetRotation(axis, delta.rotationAngle);

    // Apply rotation
    currentTrsf = rotation * currentTrsf;
  }

  component->setPlacement(currentTrsf);
}

double ConstraintSolver::computeTotalResidual(const Assembly &assembly) {
  double total = 0.0;

  for (const auto &constraint : assembly.getConstraints()) {
    auto c1 = constraint->getComponent1();
    auto c2 = constraint->getComponent2();

    if (!c1 || !c2)
      continue;

    double dist = computeDistance(constraint);

    switch (constraint->getType()) {
    case ConstraintType::Coincident:
      // Residual is the distance (should be 0) + Angular error
      total += dist * dist;

      // Calculate angular error for Face-Face
      {
        // Quick check for Face-Face implies alignment
        // We can reuse computeCoincidentCorrection logic or simplify
        // Let's call computeCoincidentCorrection to get the rotational delta
        // The magnitude of rotationAngle is a good error metric
        SolverDelta delta = computeCoincidentCorrection(constraint);
        if (delta.hasRotation()) {
          // Scale angular error to match unit of distance roughly?
          // Angle is in radians (0..PI). Distance in mm.
          // 1 radian error is HUGE compared to 1mm error usually.
          // Let's weight it.
          double angErr = delta.rotationAngle; // radians
          total += angErr * angErr * 100.0;    // weighting factor
        }
      }
      break;

    case ConstraintType::Distance: {
      double diff = dist - constraint->getValue();
      total += diff * diff;
    } break;

    default:
      // Other constraints not yet implemented
      break;
    }
  }

  return std::sqrt(total);
}

} // namespace assembly

using namespace opencad::assembly;

ConstraintSolver::SolverDelta ConstraintSolver::computeParallelCorrection(
    std::shared_ptr<AssemblyConstraint> constraint) {
  SolverDelta delta;
  auto c1 = constraint->getComponent1();
  auto c2 = constraint->getComponent2();

  TopoDS_Shape shape1, shape2;

  // Handle SubShape 1
  if (!constraint->getSubShape1().IsNull()) {
    BRepBuilderAPI_Transform xform(constraint->getSubShape1(),
                                   c1->getPlacement());
    shape1 = xform.Shape();
  } else {
    shape1 = c1->getTransformedShape();
  }

  // Handle SubShape 2
  if (!constraint->getSubShape2().IsNull()) {
    BRepBuilderAPI_Transform xform(constraint->getSubShape2(),
                                   c2->getPlacement());
    shape2 = xform.Shape();
  } else {
    shape2 = c2->getTransformedShape();
  }

  if (shape1.IsNull() || shape2.IsNull()) {
    return delta;
  }

  // Only rotation for parallel
  if (shape1.ShapeType() == TopAbs_FACE && shape2.ShapeType() == TopAbs_FACE) {
    TopoDS_Face f1 = TopoDS::Face(shape1);
    TopoDS_Face f2 = TopoDS::Face(shape2);

    BRepAdaptor_Surface surf1(f1);
    BRepAdaptor_Surface surf2(f2);

    // Get normals
    gp_Vec normal1, normal2;
    gp_Pnt tmp;

    double u1 = (surf1.FirstUParameter() + surf1.LastUParameter()) * 0.5;
    double v1 = (surf1.FirstVParameter() + surf1.LastVParameter()) * 0.5;
    gp_Vec d1u, d1v;
    surf1.D1(u1, v1, tmp, d1u, d1v);
    normal1 = d1u.Crossed(d1v);
    if (f1.Orientation() == TopAbs_REVERSED)
      normal1.Reverse();

    double u2 = (surf2.FirstUParameter() + surf2.LastUParameter()) * 0.5;
    double v2 = (surf2.FirstVParameter() + surf2.LastVParameter()) * 0.5;
    gp_Vec d2u, d2v;
    surf2.D1(u2, v2, tmp, d2u, d2v);
    normal2 = d2u.Crossed(d2v);
    if (f2.Orientation() == TopAbs_REVERSED)
      normal2.Reverse();

    normal1.Normalize();
    normal2.Normalize();

    // Target: n1 parallel to n2 (0) or anti-parallel (180)
    // Default: Parallel (0). Flipped: Anti-Parallel (180).
    gp_Vec targetN1 = constraint->isFlipped() ? -normal2 : normal2;

    if (normal1.Magnitude() > 1e-6 && targetN1.Magnitude() > 1e-6) {
      double rotAngle = normal1.Angle(targetN1);
      if (std::abs(rotAngle) > 1e-3) {
        delta.rotationAxis = normal1.Crossed(targetN1);
        if (delta.rotationAxis.Magnitude() < 1e-6) {
          // 180 degree flip needed
          gp_Vec arbitrary(1, 0, 0);
          if (normal1.IsParallel(arbitrary, 1e-2))
            arbitrary = gp_Vec(0, 1, 0);
          delta.rotationAxis = normal1.Crossed(arbitrary);
        }
        delta.rotationAngle = rotAngle;
      }
    }
  }

  return delta;
}

ConstraintSolver::SolverDelta ConstraintSolver::computePerpendicularCorrection(
    std::shared_ptr<AssemblyConstraint> constraint) {
  SolverDelta delta;
  auto c1 = constraint->getComponent1();
  auto c2 = constraint->getComponent2();

  TopoDS_Shape shape1, shape2;

  // Handle SubShape 1
  if (!constraint->getSubShape1().IsNull()) {
    BRepBuilderAPI_Transform xform(constraint->getSubShape1(),
                                   c1->getPlacement());
    shape1 = xform.Shape();
  } else {
    shape1 = c1->getTransformedShape();
  }

  // Handle SubShape 2
  if (!constraint->getSubShape2().IsNull()) {
    BRepBuilderAPI_Transform xform(constraint->getSubShape2(),
                                   c2->getPlacement());
    shape2 = xform.Shape();
  } else {
    shape2 = c2->getTransformedShape();
  }

  if (shape1.IsNull() || shape2.IsNull()) {
    return delta;
  }

  if (shape1.ShapeType() == TopAbs_FACE && shape2.ShapeType() == TopAbs_FACE) {
    TopoDS_Face f1 = TopoDS::Face(shape1);
    TopoDS_Face f2 = TopoDS::Face(shape2);

    BRepAdaptor_Surface surf1(f1);
    BRepAdaptor_Surface surf2(f2);

    // Get normals
    gp_Vec normal1, normal2;
    gp_Pnt tmp;

    double u1 = (surf1.FirstUParameter() + surf1.LastUParameter()) * 0.5;
    double v1 = (surf1.FirstVParameter() + surf1.LastVParameter()) * 0.5;
    gp_Vec d1u, d1v;
    surf1.D1(u1, v1, tmp, d1u, d1v);
    normal1 = d1u.Crossed(d1v);
    if (f1.Orientation() == TopAbs_REVERSED)
      normal1.Reverse();

    double u2 = (surf2.FirstUParameter() + surf2.LastUParameter()) * 0.5;
    double v2 = (surf2.FirstVParameter() + surf2.LastVParameter()) * 0.5;
    gp_Vec d2u, d2v;
    surf2.D1(u2, v2, tmp, d2u, d2v);
    normal2 = d2u.Crossed(d2v);
    if (f2.Orientation() == TopAbs_REVERSED)
      normal2.Reverse();

    normal1.Normalize();
    normal2.Normalize();

    // Target: n1 perpendicular to n2
    // We want n1 to rotate to be 90 degrees from n2.
    // axis = n1 x n2.
    // targetN1 = axis x n2 (vector in plane perpendicular to n2) ?
    // Actually, we just need to rotate n1 such that dot(n1, n2) = 0.
    // Shortest rotation: rotate n1 around axis (n1 x n2) until angle is 90.

    double currentAngle = normal1.Angle(normal2);
    double targetAngle = M_PI / 2.0;
    double diff = currentAngle - targetAngle; // How much we need to rotate

    // Check direction
    // If angle is 45, we need +45.
    // If angle is 135, we need -45 (to get to 90).

    if (std::abs(diff) > 1e-3) {
      delta.rotationAxis = normal1.Crossed(normal2);
      if (delta.rotationAxis.Magnitude() < 1e-6) {
        // Parallel? Then any rotation by 90 deg works.
        gp_Vec arbitrary(1, 0, 0);
        if (normal1.IsParallel(arbitrary, 1e-2))
          arbitrary = gp_Vec(0, 1, 0);
        delta.rotationAxis = normal1.Crossed(arbitrary);
        delta.rotationAngle = M_PI / 2.0;
      } else {
        // We need to rotate n1 by 'diff' around axis?
        // Not exactly. n1, n2, and axis form a frame.
        // We want n1 to move towards perpendicularity.
        // The axis n1 x n2 is orthogonal to both. Rotating around it changes
        // angle between n1 and n2 directly. Yes, rotating by (currentAngle -
        // 90deg) should work. However we need to be careful with sign. angle
        // returns [0, PI]. If we rotate n1 around (n1 x n2) by +alpha? Right
        // hand rule:
        delta.rotationAngle = -(currentAngle - M_PI / 2.0);
        // Wait, let's verify.
        // If angle is 0 (parallel). n1 x n2 is 0 (handled above).
        // If angle is SMALL (e.g. 10 deg). We want 90. Delta = 80.
        // If angle is LARGE (170 deg). We want 90. Delta = -80.
      }
    }
  }

  return delta;
}

ConstraintSolver::SolverDelta ConstraintSolver::computeConcentricCorrection(
    std::shared_ptr<AssemblyConstraint> constraint) {
  SolverDelta delta;
  auto c1 = constraint->getComponent1();
  auto c2 = constraint->getComponent2();

  TopoDS_Shape shape1, shape2;

  // Handle SubShape 1
  if (!constraint->getSubShape1().IsNull()) {
    BRepBuilderAPI_Transform xform(constraint->getSubShape1(),
                                   c1->getPlacement());
    shape1 = xform.Shape();
  } else {
    shape1 = c1->getTransformedShape();
  }

  // Handle SubShape 2
  if (!constraint->getSubShape2().IsNull()) {
    BRepBuilderAPI_Transform xform(constraint->getSubShape2(),
                                   c2->getPlacement());
    shape2 = xform.Shape();
  } else {
    shape2 = c2->getTransformedShape();
  }

  if (shape1.IsNull() || shape2.IsNull()) {
    return delta;
  }

  // Helper to extract axis
  auto getAxis = [](const TopoDS_Shape &s, gp_Ax1 &axis) -> bool {
    if (s.ShapeType() != TopAbs_FACE && s.ShapeType() != TopAbs_EDGE)
      return false;

    if (s.ShapeType() == TopAbs_FACE) {
      BRepAdaptor_Surface surf(TopoDS::Face(s));
      if (surf.GetType() == GeomAbs_Cylinder) {
        axis = surf.Cylinder().Axis();
        return true;
      } else if (surf.GetType() == GeomAbs_Cone) {
        axis = surf.Cone().Axis();
        return true;
      } else if (surf.GetType() == GeomAbs_Torus) {
        axis = surf.Torus().Axis();
        return true;
      }
    } else if (s.ShapeType() == TopAbs_EDGE) {
      BRepAdaptor_Curve curve(TopoDS::Edge(s));
      if (curve.GetType() == GeomAbs_Circle) {
        axis = curve.Circle().Axis();
        return true;
      } else if (curve.GetType() == GeomAbs_Ellipse) {
        axis = curve.Ellipse().Axis();
        return true;
      }
    }
    return false;
  };

  gp_Ax1 axis1, axis2;
  bool hasAxis1 = getAxis(shape1, axis1);
  bool hasAxis2 = getAxis(shape2, axis2);

  if (hasAxis1 && hasAxis2) {
    // 1. Align axes (Rotation)
    gp_Vec dir1 = axis1.Direction();
    gp_Vec dir2 = axis2.Direction();

    // For concentric, alignment can be parallel or anti-parallel
    // Default: Parallel. Flipped: Anti-parallel.
    gp_Vec targetDir1 = constraint->isFlipped() ? -dir2 : dir2;

    if (dir1.Magnitude() > 1e-6 && targetDir1.Magnitude() > 1e-6) {
      double rotAngle = dir1.Angle(targetDir1);
      if (std::abs(rotAngle) > 1e-3) {
        delta.rotationAxis = dir1.Crossed(targetDir1);
        if (delta.rotationAxis.Magnitude() < 1e-6) {
          gp_Vec arbitrary(1, 0, 0);
          if (dir1.IsParallel(arbitrary, 1e-2))
            arbitrary = gp_Vec(0, 1, 0);
          delta.rotationAxis = dir1.Crossed(arbitrary);
        }
        delta.rotationAngle = rotAngle;
      }
    }

    // 2. Coincident Axes (Translation)
    // Project origin of axis1 onto axis2.
    gp_Vec v2 = axis2.Direction();
    gp_Vec p2p1(axis2.Location(), axis1.Location());

    double t = p2p1.Dot(v2);
    gp_Pnt pClosest = axis2.Location().Translated(v2 * t);

    delta.translation = gp_Vec(axis1.Location(), pClosest);
  }

  return delta;
}

ConstraintSolver::SolverDelta ConstraintSolver::computeTangentCorrection(
    std::shared_ptr<AssemblyConstraint> constraint) {
  // Reuse coincident correction logic which handles distance minimization and
  // face alignment
  return computeCoincidentCorrection(constraint);
}

ConstraintSolver::SolverDelta ConstraintSolver::computeAngleCorrection(
    std::shared_ptr<AssemblyConstraint> constraint) {
  SolverDelta delta;
  auto c1 = constraint->getComponent1();
  auto c2 = constraint->getComponent2();
  double targetAngle =
      constraint->getValue() * (M_PI / 180.0); // Convert deg to rad

  TopoDS_Shape shape1, shape2;
  // Handle SubShape 1
  if (!constraint->getSubShape1().IsNull()) {
    BRepBuilderAPI_Transform xform(constraint->getSubShape1(),
                                   c1->getPlacement());
    shape1 = xform.Shape();
  } else {
    shape1 = c1->getTransformedShape();
  }
  // Handle SubShape 2
  if (!constraint->getSubShape2().IsNull()) {
    BRepBuilderAPI_Transform xform(constraint->getSubShape2(),
                                   c2->getPlacement());
    shape2 = xform.Shape();
  } else {
    shape2 = c2->getTransformedShape();
  }

  if (shape1.IsNull() || shape2.IsNull())
    return delta;

  if (shape1.ShapeType() == TopAbs_FACE && shape2.ShapeType() == TopAbs_FACE) {
    TopoDS_Face f1 = TopoDS::Face(shape1);
    TopoDS_Face f2 = TopoDS::Face(shape2);
    BRepAdaptor_Surface surf1(f1);
    BRepAdaptor_Surface surf2(f2);

    // Simplification: use center normals
    gp_Pnt tmp;
    double u1 = (surf1.FirstUParameter() + surf1.LastUParameter()) * 0.5;
    double v1 = (surf1.FirstVParameter() + surf1.LastVParameter()) * 0.5;
    gp_Vec d1u, d1v, n1;
    surf1.D1(u1, v1, tmp, d1u, d1v);
    n1 = d1u.Crossed(d1v);
    if (f1.Orientation() == TopAbs_REVERSED)
      n1.Reverse();

    double u2 = (surf2.FirstUParameter() + surf2.LastUParameter()) * 0.5;
    double v2 = (surf2.FirstVParameter() + surf2.LastVParameter()) * 0.5;
    gp_Vec d2u, d2v, n2;
    surf2.D1(u2, v2, tmp, d2u, d2v);
    n2 = d2u.Crossed(d2v);
    if (f2.Orientation() == TopAbs_REVERSED)
      n2.Reverse();

    double currentAngle = n1.Angle(n2);
    double diff = currentAngle - targetAngle;

    if (std::abs(diff) > 1e-3) {
      delta.rotationAxis = n1.Crossed(n2);
      if (delta.rotationAxis.Magnitude() < 1e-6) {
        // Vectors are parallel (angle 0 or PI)
        gp_Vec arbitrary(1, 0, 0);
        if (n1.IsParallel(arbitrary, 1e-2))
          arbitrary = gp_Vec(0, 1, 0);
        delta.rotationAxis = n1.Crossed(arbitrary);
      }
      // Determine direction - heuristic
      delta.rotationAngle = -diff;
    }
  }
  return delta;
}

ConstraintSolver::SolverDelta ConstraintSolver::computeLockCorrection(
    std::shared_ptr<AssemblyConstraint> constraint) {
  // Lock means fully rigid.
  // Treat as Coincident (Distance=0) + Alignment of all axes
  SolverDelta delta = computeCoincidentCorrection(constraint);

  // If no rotation from Coincident, check alignment
  if (!delta.hasRotation()) {
    auto c1 = constraint->getComponent1();
    auto c2 = constraint->getComponent2();
    gp_Trsf t1 = c1->getPlacement();
    gp_Trsf t2 = c2->getPlacement();

    gp_Vec xAxis(1, 0, 0);
    gp_Vec x1 = xAxis.Transformed(t1);
    gp_Vec x2 = xAxis.Transformed(t2);

    if (!x1.IsParallel(x2, 1e-2)) {
      delta.rotationAxis = x1.Crossed(x2);
      if (delta.rotationAxis.Magnitude() < 1e-6) {
        // 180 flip
        gp_Vec arbitrary(0, 1, 0);
        if (x1.IsParallel(arbitrary, 1e-2))
          arbitrary = gp_Vec(0, 0, 1);
        delta.rotationAxis = x1.Crossed(arbitrary);
      }
      delta.rotationAngle = x1.Angle(x2);
    }
  }
  return delta;
}

ConstraintSolver::SolverDelta ConstraintSolver::computeGearCorrection(
    std::shared_ptr<AssemblyConstraint> constraint) {
  // Simplification: Treat like Distance/Tangent for now to just bring them
  // close
  return computeDistanceCorrection(constraint);
}

ConstraintSolver::SolverDelta ConstraintSolver::computeScrewCorrection(
    std::shared_ptr<AssemblyConstraint> constraint) {
  // Simplification: Treat like Concentric for now
  return computeConcentricCorrection(constraint);
}

} // namespace opencad
