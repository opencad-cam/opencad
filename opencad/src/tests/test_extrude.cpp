#include <BRepAlgoAPI_Cut.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepTools.hxx>
#include <Bnd_Box.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <gp_Circ.hxx>
#include <iostream>


int main() {
  gp_Dir normal(0, 0, 1);
  gp_Pnt center(0, 0, 0);

  TopoDS_Edge eOuter =
      BRepBuilderAPI_MakeEdge(gp_Circ(gp_Ax2(center, normal), 50.0));
  TopoDS_Wire wOuter = BRepBuilderAPI_MakeWire(eOuter);
  TopoDS_Face fOuter = BRepBuilderAPI_MakeFace(wOuter);

  TopoDS_Edge eInner =
      BRepBuilderAPI_MakeEdge(gp_Circ(gp_Ax2(center, normal), 20.0));
  TopoDS_Wire wInner = BRepBuilderAPI_MakeWire(eInner);
  TopoDS_Face fInner = BRepBuilderAPI_MakeFace(wInner);

  BRepAlgoAPI_Cut cutOp(fOuter, fInner);
  cutOp.Build();
  TopoDS_Shape cutShape = cutOp.Shape();

  TopExp_Explorer faceExp(cutShape, TopAbs_FACE);
  if (faceExp.More()) {
    TopoDS_Face holeyFace = TopoDS::Face(faceExp.Current());

    int wCount = 0;
    TopExp_Explorer wExp(holeyFace, TopAbs_WIRE);
    while (wExp.More()) {
      wCount++;
      std::cout << "Wire orient: " << wExp.Current().Orientation() << std::endl;
      wExp.Next();
    }
    std::cout << "Wires in cut face: " << wCount << std::endl;

    BRepPrimAPI_MakePrism prism(holeyFace, gp_Vec(0, 0, 50));
    std::cout << "Prism done: " << prism.IsDone() << std::endl;

    if (prism.IsDone()) {
      TopoDS_Shape res = prism.Shape();
      int fCount = 0;
      TopExp_Explorer e(res, TopAbs_FACE);
      while (e.More()) {
        fCount++;
        e.Next();
      }
      std::cout << "Extruded faces: " << fCount << std::endl;
    }

    // Now test my manual rebuilding logic
    std::cout << "\n--- Testing Manual Rebuilding Logic ---" << std::endl;
    std::vector<TopoDS_Wire> wires;
    wExp.Init(holeyFace, TopAbs_WIRE);
    while (wExp.More()) {
      wires.push_back(TopoDS::Wire(wExp.Current()));
      wExp.Next();
    }

    std::sort(
        wires.begin(), wires.end(),
        [](const TopoDS_Wire &a, const TopoDS_Wire &b) {
          Bnd_Box bA, bB;
          BRepBndLib::Add(a, bA);
          BRepBndLib::Add(b, bB);
          double minXa, minYa, minZa, maxXa, maxYa, maxZa;
          double minXb, minYb, minZb, maxXb, maxYb, maxZb;
          bA.Get(minXa, minYa, minZa, maxXa, maxYa, maxZa);
          bB.Get(minXb, minYb, minZb, maxXb, maxYb, maxZb);
          double diagA =
              gp_Pnt(minXa, minYa, minZa).Distance(gp_Pnt(maxXa, maxYa, maxZa));
          double diagB =
              gp_Pnt(minXb, minYb, minZb).Distance(gp_Pnt(maxXb, maxYb, maxZb));
          return diagA > diagB;
        });

    TopoDS_Wire outerWire = TopoDS::Wire(wires[0].Oriented(TopAbs_FORWARD));
    BRepBuilderAPI_MakeFace mkFace(outerWire, true);
    if (mkFace.IsDone()) {
      for (size_t i = 1; i < wires.size(); ++i) {
        TopoDS_Wire innerWire =
            TopoDS::Wire(wires[i].Oriented(TopAbs_REVERSED));
        mkFace.Add(innerWire);
      }
      if (mkFace.IsDone()) {
        TopoDS_Face rebuiltFace = mkFace.Face();
        std::cout << "Rebuilt face successfully." << std::endl;

        BRepPrimAPI_MakePrism prism2(rebuiltFace, gp_Vec(0, 0, 50));
        std::cout << "Rebuilt Prism done: " << prism2.IsDone() << std::endl;
        if (prism2.IsDone()) {
          TopoDS_Shape res2 = prism2.Shape();
          int fCount2 = 0;
          TopExp_Explorer e2(res2, TopAbs_FACE);
          while (e2.More()) {
            fCount2++;
            e2.Next();
          }
          std::cout << "Rebuilt Extruded faces: " << fCount2 << std::endl;
        }
      } else {
        std::cout << "MakeFace failed adding holes." << std::endl;
      }
    }
  }
  return 0;
}
