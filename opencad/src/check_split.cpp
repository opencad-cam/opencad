#include <BRepAlgoAPI_Splitter.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepTools.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <gp_Circ.hxx>
#include <gp_Dir.hxx>
#include <gp_Vec.hxx>
#include <iostream>

int main() {
  // 1. Two concentric circles
  gp_Circ cOuter(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), 100);
  gp_Circ cInner(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), 50);

  TopoDS_Edge eOuter = BRepBuilderAPI_MakeEdge(cOuter);
  TopoDS_Edge eInner = BRepBuilderAPI_MakeEdge(cInner);

  // 2. Substrate
  TopoDS_Wire subW = BRepBuilderAPI_MakePolygon(
                         gp_Pnt(-200, -200, 0), gp_Pnt(200, -200, 0),
                         gp_Pnt(200, 200, 0), gp_Pnt(-200, 200, 0), true)
                         .Wire();
  TopoDS_Face subFace = BRepBuilderAPI_MakeFace(subW).Face();

  // 3. Split
  BRepAlgoAPI_Splitter splitter;
  TopTools_ListOfShape args;
  args.Append(subFace);
  TopTools_ListOfShape tools;
  tools.Append(eOuter);
  tools.Append(eInner);
  splitter.SetArguments(args);
  splitter.SetTools(tools);
  splitter.Build();

  TopExp_Explorer exp(splitter.Shape(), TopAbs_FACE);
  int i = 0;
  while (exp.More()) {
    TopoDS_Face f = TopoDS::Face(exp.Current());
    std::cout << "Face " << ++i << " has wires: ";
    TopExp_Explorer wExp(f, TopAbs_WIRE);
    int wc = 0;
    while (wExp.More()) {
      wc++;
      wExp.Next();
    }
    std::cout << wc << "\n";
    exp.Next();
  }
  return 0;
}
