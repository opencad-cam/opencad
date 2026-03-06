#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Splitter.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepGProp.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepTools.hxx>
#include <BRep_Builder.hxx>
#include <Bnd_Box.hxx>
#include <GProp_GProps.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <gp_Ax2.hxx>
#include <gp_Circ.hxx>
#include <gp_Dir.hxx>
#include <gp_Pnt.hxx>
#include <iostream>


int main() {
  // 1. Create circles for substrate and split edges
  gp_Dir normal(0, 0, 1);
  gp_Pnt center(0, 0, 0);

  // Outer circle R=50
  TopoDS_Edge eOuter =
      BRepBuilderAPI_MakeEdge(gp_Circ(gp_Ax2(center, normal), 50.0));
  // Inner circle R=20
  TopoDS_Edge eInner =
      BRepBuilderAPI_MakeEdge(gp_Circ(gp_Ax2(center, normal), 20.0));

  // Substrate
  TopoDS_Edge s1 =
      BRepBuilderAPI_MakeEdge(gp_Pnt(-100, -100, 0), gp_Pnt(100, -100, 0));
  TopoDS_Edge s2 =
      BRepBuilderAPI_MakeEdge(gp_Pnt(100, -100, 0), gp_Pnt(100, 100, 0));
  TopoDS_Edge s3 =
      BRepBuilderAPI_MakeEdge(gp_Pnt(100, 100, 0), gp_Pnt(-100, 100, 0));
  TopoDS_Edge s4 =
      BRepBuilderAPI_MakeEdge(gp_Pnt(-100, 100, 0), gp_Pnt(-100, -100, 0));
  TopoDS_Wire subWire = BRepBuilderAPI_MakeWire(s1, s2, s3, s4);
  TopoDS_Face subFace = BRepBuilderAPI_MakeFace(subWire, true);

  // Split
  BRepAlgoAPI_Splitter splitter;
  TopTools_ListOfShape args, tools;
  args.Append(subFace);
  tools.Append(eOuter);
  tools.Append(eInner);
  splitter.SetArguments(args);
  splitter.SetTools(tools);
  splitter.Build();

  std::cout << "Splitter Done: " << splitter.IsDone() << std::endl;

  TopoDS_Shape res = splitter.Shape();
  TopExp_Explorer exp(res, TopAbs_FACE);
  std::vector<TopoDS_Face> faces;
  while (exp.More()) {
    TopoDS_Face f = TopoDS::Face(exp.Current());
    Bnd_Box box;
    BRepBndLib::Add(f, box);
    double xmin, ymin, zmin, xmax, ymax, zmax;
    box.Get(xmin, ymin, zmin, xmax, ymax, zmax);

    // Output bounds
    std::cout << "Face bounds: " << xmin << " " << ymin << " " << xmax << " "
              << ymax << std::endl;

    // Check if it's not the substrate boundary
    if (xmax < 90) {
      faces.push_back(f);

      // Analyze wires
      int wCount = 0;
      TopExp_Explorer wExp(f, TopAbs_WIRE);
      while (wExp.More()) {
        wCount++;
        TopoDS_Wire w = TopoDS::Wire(wExp.Current());
        std::cout << "  Wire orient: "
                  << (w.Orientation() == TopAbs_FORWARD ? "FORWARD"
                                                        : "REVERSED")
                  << std::endl;
        wExp.Next();
      }
      std::cout << "  Valid Face has wires: " << wCount << std::endl;
    }
    exp.Next();
  }

  // Test extrude the ring
  for (auto f : faces) {
    GProp_GProps props;
    BRepGProp::SurfaceProperties(f, props);
    std::cout << "\nTesting extrude on face area " << props.Mass() << std::endl;

    BRepPrimAPI_MakePrism prism(f, gp_Vec(0, 0, 50));
    std::cout << "Prism done: " << prism.IsDone() << std::endl;

    if (prism.IsDone()) {
      TopoDS_Shape shape = prism.Shape();
      int solidCount = 0;
      TopExp_Explorer sExp(shape, TopAbs_SOLID);
      while (sExp.More()) {
        solidCount++;
        sExp.Next();
      }

      GProp_GProps sprops;
      BRepGProp::VolumeProperties(shape, sprops);
      std::cout << "  Extruded solid volume: " << sprops.Mass() << std::endl;
      std::cout << "  Extruded solids count: " << solidCount << std::endl;
    }
  }

  return 0;
}
