#include "GearFeature.h"

#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <GC_MakeSegment.hxx>
#include <Geom_Line.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Wire.hxx>
#include <cmath>
#include <gp_Circ.hxx>
#include <gp_Pln.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace opencad {
namespace part {

TopoDS_Shape GearFeature::execute(const GearParams &params) {
  if (params.module <= 0 || params.numTeeth < 3 || params.thickness <= 0) {
    m_error = "Invalid gear parameters";
    return TopoDS_Shape();
  }

  // Gear calculations
  double m = params.module;
  int Z = params.numTeeth;
  double alpha = params.pressureAngle * M_PI / 180.0; // Pressure angle

  double d = m * Z;           // Pitch diameter
  double r = d / 2.0;         // Pitch radius
  double da = d + 2 * m;      // Tip diameter
  double ra = da / 2.0;       // Tip radius
  double db = d * cos(alpha); // Base diameter
  double rb = db / 2.0;       // Base radius
  double df = d - 2.5 * m;    // Root diameter
  double rf = df / 2.0;       // Root radius

  if (rf < 0)
    rf = 0.1;
  if (rb >= ra) {
    m_error = "Base radius is larger than Tip radius.";
    return TopoDS_Shape();
  }

  try {
    // --- Involute Calculation ---
    // Involute function inv(x) = tan(x) - x
    // Intersection of involute with pitch circle:
    double inv_alpha = tan(alpha) - alpha;

    // Half angular thickness of tooth at pitch circle
    double halfToothAngle = M_PI / (2.0 * Z);

    // Rotation required to place the involute curve
    // The Standard Involute (starting at X axis, CCW) has angle `inv_alpha` at
    // radius `r`. We want the Bottom Flank (lower side of tooth) to be at angle
    // `-halfToothAngle` at radius `r`. So we assume the Bottom Flank
    // corresponds to the standard Involute rotated by `theta_bottom`. inv_alpha
    // + theta_bottom = -halfToothAngle theta_bottom = -(halfToothAngle +
    // inv_alpha)

    double rotationOverlap = halfToothAngle + inv_alpha;

    // Generate Base Involute Points (Standard CCW from X-axis)
    int numPoints = 15;
    std::vector<gp_Pnt> baseInvolutePoints;
    double max_t = sqrt((ra * ra - rb * rb) / (rb * rb)); // Parameter t at Tip

    for (int i = 0; i <= numPoints; ++i) {
      double t = (double)i / numPoints * max_t;
      double x = rb * (cos(t) + t * sin(t));
      double y = rb * (sin(t) - t * cos(t));
      baseInvolutePoints.emplace_back(x, y, 0);
    }

    // Create Bottom Flank points (Rotate Standard Involute Clockwise)
    std::vector<gp_Pnt> bottomFlankPoints;
    gp_Trsf rotBottom;
    rotBottom.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)),
                          -rotationOverlap);
    for (const auto &p : baseInvolutePoints) {
      bottomFlankPoints.push_back(p.Transformed(rotBottom));
    }

    // Create Top Flank points (Mirror Standard Involute to be CW, then Rotate
    // Counter-Clockwise) Mirror of Standard Involute (around X axis) goes CW (y
    // becomes negative). At pitch r, its angle is -inv_alpha. We want Top Flank
    // at pitch r to be at angle +halfToothAngle. -inv_alpha + theta_top =
    // halfToothAngle theta_top = halfToothAngle + inv_alpha = rotationOverlap.

    std::vector<gp_Pnt> topFlankPoints;
    gp_Trsf mirrorX;
    mirrorX.SetMirror(
        gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 1, 0))); // XZ plane mirror

    gp_Trsf rotTop;
    rotTop.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)),
                       rotationOverlap);

    gp_Trsf topTransform = rotTop * mirrorX;

    for (const auto &p : baseInvolutePoints) {
      topFlankPoints.push_back(p.Transformed(topTransform));
    }

    // Build the Tooth Wire
    BRepBuilderAPI_MakeWire wireBuilder;
    double pitchAngle = 2.0 * M_PI / Z;

    for (int tooth = 0; tooth < Z; ++tooth) {
      double theta = tooth * pitchAngle;
      gp_Trsf toothRot;
      toothRot.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), theta);

      // 1. Bottom Flank (Base -> Tip, Moving Out)
      // Transform the pre-built bottom flank
      std::vector<gp_Pnt> bPts;
      for (const auto &p : bottomFlankPoints)
        bPts.push_back(p.Transformed(toothRot));

      for (size_t i = 0; i < bPts.size() - 1; ++i) {
        wireBuilder.Add(BRepBuilderAPI_MakeEdge(bPts[i], bPts[i + 1]));
      }

      // 2. Tip Arc (Bottom Flank Tip -> Top Flank Tip)
      gp_Pnt tipStart = bPts.back();
      gp_Pnt tipEnd = topFlankPoints.back().Transformed(toothRot);

      if (tipStart.Distance(tipEnd) > 1e-4) {
        Handle(Geom_TrimmedCurve) arc = GC_MakeArcOfCircle(
            gp_Circ(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), ra), tipStart,
            tipEnd, true); // CCW
        wireBuilder.Add(BRepBuilderAPI_MakeEdge(arc));
      }

      // 3. Top Flank (Tip -> Base, Moving In)
      // Reverse order of topFlankPoints
      std::vector<gp_Pnt> tPts;
      for (const auto &p : topFlankPoints)
        tPts.push_back(p.Transformed(toothRot));

      for (int i = (int)tPts.size() - 1; i > 0; --i) {
        wireBuilder.Add(BRepBuilderAPI_MakeEdge(tPts[i], tPts[i - 1]));
      }

      // 4. Root (Top Base of Current -> Bottom Base of Next)
      gp_Pnt rootStart = tPts[0]; // Current tooth Top Base (at rb)

      // Next tooth Bottom Base (at rb)
      double thetaNext = (tooth + 1) * pitchAngle;
      gp_Trsf nextRot;
      nextRot.SetRotation(gp_Ax1(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), thetaNext);
      gp_Pnt rootEnd = bottomFlankPoints[0].Transformed(nextRot);

      // Connection logic (handle rf < rb)
      if (rf < rb - 1e-4) {
        // Line Down from rb to rf (Radial-ish?)
        // Just project to rf along radial line
        double ang1 = atan2(rootStart.Y(), rootStart.X());
        gp_Pnt p1_rf(rf * cos(ang1), rf * sin(ang1), 0);
        wireBuilder.Add(BRepBuilderAPI_MakeEdge(rootStart, p1_rf));

        double ang2 = atan2(rootEnd.Y(), rootEnd.X());
        gp_Pnt p2_rf(rf * cos(ang2), rf * sin(ang2), 0);

        // Arc along rf (CCW)
        Handle(Geom_TrimmedCurve) rootArc = GC_MakeArcOfCircle(
            gp_Circ(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), rf), p1_rf, p2_rf,
            true);
        wireBuilder.Add(BRepBuilderAPI_MakeEdge(rootArc));

        // Line Up from rf to rb
        wireBuilder.Add(BRepBuilderAPI_MakeEdge(p2_rf, rootEnd));
      } else {
        // Arc along rb directly
        Handle(Geom_TrimmedCurve) rootArc = GC_MakeArcOfCircle(
            gp_Circ(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), rb), rootStart,
            rootEnd, true);
        wireBuilder.Add(BRepBuilderAPI_MakeEdge(rootArc));
      }
    }

    if (!wireBuilder.IsDone()) {
      m_error = "Failed to build gear profile wire";
      return TopoDS_Shape();
    }

    TopoDS_Wire profileWire = wireBuilder.Wire();

    // Create Face
    BRepBuilderAPI_MakeFace faceBuilder(profileWire, true); // planar
    if (!faceBuilder.IsDone()) {
      m_error = "Failed to create gear face";
      return TopoDS_Shape();
    }

    TopoDS_Face gearFace = faceBuilder.Face();

    // Extrude
    gp_Vec extrudeVec(0, 0, params.thickness);
    BRepPrimAPI_MakePrism prism(gearFace, extrudeVec);

    if (!prism.IsDone()) {
      m_error = "Failed to extrude gear";
      return TopoDS_Shape();
    }

    return prism.Shape();

  } catch (const std::exception &e) {
    m_error = std::string("Exception: ") + e.what();
    return TopoDS_Shape();
  } catch (...) {
    return TopoDS_Shape();
  }
}

} // namespace part
} // namespace opencad
